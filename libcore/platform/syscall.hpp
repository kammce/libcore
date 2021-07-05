#pragma once

#include <functional>
#include <libcore/peripherals/uart.hpp>
#include <libcore/utility/ansi_terminal_codes.hpp>
#include <libcore/utility/error_handling.hpp>
#include <libcore/utility/log.hpp>
#include <libcore/utility/memory_resource.hpp>
#include <span>
#include <vector>

namespace sjsu
{
class SysCall
{
 public:
  using write_function = std::function<int(int, const char *, int)>;
  using read_function  = std::function<int(int, char *, int)>;

  /// Write to a resource
  virtual const std::span<write_function> GetWriter() = 0;

  /// Read from a resource
  virtual const std::span<read_function> GetReader() = 0;

  /// Write to a resource
  ///
  /// @throws std::bad_alloc if the function could not be added
  virtual void AddWriter(write_function additional_output_stream) = 0;

  /// Read from a resource
  ///
  /// @throws std::bad_alloc if the function could not be added
  virtual void AddReader(read_function additional_input_stream) = 0;

  /// Add a UART serial port to both the writers and readers
  ///
  /// @param serial_port
  void AddSerial(sjsu::Uart & serial_port)
  {
    // Create a routine for writing to the UART port
    AddWriter(
        [&serial_port](int, const char * buffer, int length) -> int
        {
          serial_port.Write(std::span<const std::byte>(
              reinterpret_cast<const std::byte *>(buffer), length));
          return length;
        });

    // Create a routine for reading to the UART port
    AddReader(
        [&serial_port](int, char * buffer, int length) -> int
        {
          serial_port.Read(std::span<std::byte>(
              reinterpret_cast<std::byte *>(buffer), length));
          return length;
        });
  }
};

template <size_t callback_count>
class StaticSysCall : public SysCall
{
 public:
  static constexpr size_t kBytesPerCallback = sizeof(write_function);
  static constexpr size_t kReserveBytes = kBytesPerCallback * callback_count;

  StaticSysCall()
      : memory_resource{}, write(&memory_resource), read(&memory_resource)
  {
  }

  /// Write to a resource
  const std::span<write_function> GetWriter() override
  {
    return write;
  }

  /// Read from a resource
  const std::span<read_function> GetReader() override
  {
    return read;
  }

  /// Write to a resource
  void AddWriter(write_function additional_output_stream) override
  {
    write.push_back(additional_output_stream);
  }

  /// Read from a resource
  void AddReader(read_function additional_input_stream) override
  {
    read.push_back(additional_input_stream);
  }

 protected:
  StaticMemoryResource<kReserveBytes> memory_resource;
  std::pmr::vector<write_function> write;
  std::pmr::vector<read_function> read;
};

class SysCallManager
{
 public:
  inline static StaticSysCall<2> default_newlib;

  /// Set the controller for the platform. This is set by the system's
  /// platforms startup code and does not need to be executed by the
  /// user. This can be run by the user if they want to inject their own
  /// system controller into the system to be used by the whole system.
  ///
  /// @param newlib - a pointer to the current platform's
  ///        system controller.
  static void Set(SysCall * newlib)
  {
    platform_newlib = newlib;
  }

  /// Get the system controller set for this platform.
  /// After main() is called by the startup code, this function will return a
  /// valid system controller. It is required that each platform startup routine
  /// set a system controller using the `void Set(SysCall&)` static
  /// method.
  static SysCall & Get()
  {
    return *platform_newlib;
  }

  static int Write(int file, const void * source_buffer, size_t length)
  {
    for (auto writer : Get().GetWriter())
    {
      writer(file, reinterpret_cast<const char *>(source_buffer), length);
    }
    return length;
  }

  static int Read(int file, void * destination_buffer, size_t length)
  {
    int bytes_read = 0;
    for (auto reader : Get().GetReader())
    {
      bytes_read =
          reader(file, reinterpret_cast<char *>(destination_buffer), length);
      if (bytes_read > 0)
      {
        break;
      }
    }
    return bytes_read;
  }

  static void HandleExceptionPointer(std::exception_ptr exception_pointer)
  {
    sjsu::log::Critical("Uncaught exception: ");
    try
    {
      if (exception_pointer)
      {
        std::rethrow_exception(exception_pointer);
      }
    }
    catch (const std::exception & e)
    {
      sjsu::log::Critical("std::exception(%s)\n", e.what());
    }
    catch (sjsu::Exception & e)
    {
      e.Print();
    }
    catch (...)
    {
      sjsu::log::Critical("Unknown...\n");
    }
  }

 protected:
  /// Global platform system controller scoped within this class. Most
  /// systems only need a single platform system controller, and thus this
  /// can hold a general/default platform system controller that can be
  /// retrieved via Set and Get.
  static inline SysCall * platform_newlib = &default_newlib;
};

}  // namespace sjsu

extern "C"
{
  inline size_t fwrite(const void * buffer,
                       size_t size,
                       size_t count,
                       FILE * stream)
  {
    return sjsu::SysCallManager::Write(
        reinterpret_cast<int>(stream), buffer, count * size);
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  inline void _exit(int return_code)
  {
    // Print based on return code value
    if (return_code >= 0)
    {
      sjsu::log::Print("\n" SJ2_BOLD_WHITE SJ2_BACKGROUND_GREEN
                       "Program Returned Exit Code: {}\n" SJ2_COLOR_RESET,
                       return_code);
    }
    else
    {
      sjsu::log::Print("\n" SJ2_BOLD_WHITE SJ2_BACKGROUND_RED
                       "Program Returned Exit Code: {}\n" SJ2_COLOR_RESET,
                       return_code);
    }

    sjsu::SysCallManager::HandleExceptionPointer(std::current_exception());

    while (1)
    {
      continue;
    }
  }

  // Dummy implementation of getpid
  // NOLINTNEXTLINE(readability-identifier-naming)
  int getpid()
  {
    return 1;
  }

  // Dummy implementation of kill
  // NOLINTNEXTLINE(readability-identifier-naming)
  int kill(int, int)
  {
    return -1;
  }

  inline void __cxa_pure_virtual() {}
  inline void __cxa_atexit() {}

  inline struct _reent r = { 0, nullptr, reinterpret_cast<FILE *>(1), nullptr };
  inline struct _reent * _impure_ptr = &r;
}

namespace sjsu
{
/// Call this anywhere in the code to cause the compiler to pull in all of the
/// newlib implementation functions. This function has no cost when called. With
/// little optimization, the cost will be a single function call with a return,
/// which should only be done once or with optimization, this call should be
/// eliminated entierly, with the symbols still in the final binary.
inline void AddSysCallSymbols()
{
  static void * system_call_symols[] [[gnu::used]] = {
    reinterpret_cast<void *>(_exit),
    reinterpret_cast<void *>(__cxa_pure_virtual),
    reinterpret_cast<void *>(__cxa_atexit),
    reinterpret_cast<void *>(fwrite),
    reinterpret_cast<void *>(kill),
    reinterpret_cast<void *>(getpid),
  };
}
}  // namespace sjsu

namespace __cxxabiv1
{
/* Override default terminate handler to just abort. */
std::terminate_handler __terminate_handler = std::abort;
} /* namespace __cxxabiv1 */
