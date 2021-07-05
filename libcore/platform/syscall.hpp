#pragma once

#include <cstdio>
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
  using write_function = std::function<int(FILE *, const char *, int)>;
  using read_function  = std::function<int(FILE *, char *, int)>;

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
        [&serial_port](FILE *, const char * buffer, int length) -> int
        {
          serial_port.Write(std::span<const std::byte>(
              reinterpret_cast<const std::byte *>(buffer), length));
          return length;
        });

    // Create a routine for reading to the UART port
    AddReader(
        [&serial_port](FILE *, char * buffer, int length) -> int
        {
          if (serial_port.HasData())
          {
            int bytes_read = serial_port.Read(std::span<std::byte>(
                reinterpret_cast<std::byte *>(buffer), length));
            return bytes_read;
          }
          return 0;
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
  static int Write(FILE * file, const char * source_buffer, size_t length)
  {
    for (auto writer : Get().GetWriter())
    {
      writer(file, source_buffer, length);
    }
    return length;
  }

  static int Read(FILE * file, char * destination_buffer, size_t length)
  {
    int bytes_read = 0;
    for (auto reader : Get().GetReader())
    {
      bytes_read = reader(file, destination_buffer, length);
      if (bytes_read > 0)
      {
        break;
      }
    }
    return bytes_read;
  }

  static int Flush([[maybe_unused]] FILE * file)
  {
    Write(file, buffer.data(), buffer.size());
    buffer.clear();
    return 0;
  }

  static int PutChar(char c, [[maybe_unused]] FILE * file)
  {
    if (buffer.size() == memory_resource.Capacity())
    {
      Flush(file);
    }

    buffer.push_back(c);

    if (c == '\n')
    {
      Flush(file);
    }

    return 0;
  }

  static int GetChar([[maybe_unused]] FILE * file)
  {
    char byte = 0;
    Read(file, &byte, sizeof(byte));
    return byte;
  }

  inline static StaticSysCall<2> default_newlib;
  inline static SysCall * platform_newlib = &default_newlib;
  inline static sjsu::StaticMemoryResource<BUFSIZ> memory_resource;
  inline static std::pmr::vector<char> buffer;

 public:
  inline static FILE stdio =
      FDEV_SETUP_STREAM(PutChar, GetChar, Flush, _FDEV_SETUP_RW);
};
}  // namespace sjsu

extern "C"
{
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

  inline FILE * const __iob[3] = {
    &sjsu::SysCallManager::stdio,
    &sjsu::SysCallManager::stdio,
    &sjsu::SysCallManager::stdio,
  };
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
  static const void * system_call_symols[] [[gnu::used]] = {
      reinterpret_cast<const void *>(_exit),
      reinterpret_cast<const void *>(__cxa_pure_virtual),
      reinterpret_cast<const void *>(__cxa_atexit),
      reinterpret_cast<const void *>(kill),
      reinterpret_cast<const void *>(getpid),
      reinterpret_cast<const void *>(__iob),
  };
}
}  // namespace sjsu

namespace __cxxabiv1
{
/* Override default terminate handler to just abort. */
std::terminate_handler __terminate_handler = std::abort;
} /* namespace __cxxabiv1 */
