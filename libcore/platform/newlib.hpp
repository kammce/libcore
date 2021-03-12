#pragma once

#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <functional>
#include <libcore/utility/ansi_terminal_codes.hpp>
#include <libcore/utility/error_handling.hpp>
#include <libcore/utility/memory_resource.hpp>
#include <span>
#include <vector>

#undef errno
extern int errno;

namespace sjsu
{
class Newlib
{
 public:
  using sbrk_function  = std::function<void *(int increment)>;
  using write_function = std::function<int(int, const char *, int)>;
  using read_function  = std::function<int(int, char *, int)>;

  /// Allocate more Heap (space break value)
  virtual const std::span<sbrk_function> GetHeapAllocators() = 0;

  /// Write to a resource
  virtual const std::span<write_function> GetWriter() = 0;

  /// Read from a resource
  virtual const std::span<read_function> GetReader() = 0;

  /// Allocate more Heap (space break value)
  ///
  /// @throws std::bad_alloc if the function could not be added
  virtual void AddHeapAllocators(sbrk_function additional_memory_resource) = 0;

  /// Write to a resource
  ///
  /// @throws std::bad_alloc if the function could not be added
  virtual void AddWriter(write_function additional_output_stream) = 0;

  /// Read from a resource
  ///
  /// @throws std::bad_alloc if the function could not be added
  virtual void AddReader(read_function additional_input_stream) = 0;
};

template <size_t callback_count>
class StaticNewlib : public Newlib
{
 public:
  static constexpr size_t kBytesPerCallback = sizeof(sbrk_function);
  static constexpr size_t kReserveBytes = kBytesPerCallback * callback_count;

  StaticNewlib()
      : memory_resource{},
        sbrk(&memory_resource),
        write(&memory_resource),
        read(&memory_resource)
  {
  }

  /// Allocate more Heap (space break value)
  const std::span<sbrk_function> GetHeapAllocators() override
  {
    return sbrk;
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

  /// Allocate more Heap (space break value)
  void AddHeapAllocators(sbrk_function additional_memory_resource) override
  {
    sbrk.push_back(additional_memory_resource);
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
  std::pmr::vector<sbrk_function> sbrk;
  std::pmr::vector<write_function> write;
  std::pmr::vector<read_function> read;
};

class NewlibManager
{
 public:
  class DefaultNewlib : public Newlib
  {
    /// Allocate more Heap (space break value)
    const std::span<sbrk_function> GetHeapAllocators() override
    {
      static sbrk_function dummy_sbrk_function = [](int) -> void * {
        return nullptr;
      };
      static std::array<sbrk_function, 1> array = { dummy_sbrk_function };
      return array;
    }

    /// Write to a resource
    const std::span<write_function> GetWriter() override
    {
      static write_function dummy_write_function =
          [](int, const char *, int) -> int { return 0; };
      static std::array<write_function, 1> array = { dummy_write_function };
      return array;
    }

    /// Read from a resource
    const std::span<read_function> GetReader() override
    {
      static read_function dummy_read_function = [](int, char *, int) -> int {
        return 0;
      };
      static std::array<read_function, 1> array = { dummy_read_function };
      return array;
    }

    void AddHeapAllocators(sbrk_function) override
    {
      throw std::bad_alloc();
    }

    void AddWriter(write_function) override
    {
      throw std::bad_alloc();
    }

    void AddReader(read_function) override
    {
      throw std::bad_alloc();
    }
  };

  inline static DefaultNewlib default_newlib;

  /// Set the controller for the platform. This is set by the system's
  /// platforms startup code and does not need to be executed by the
  /// user. This can be run by the user if they want to inject their own
  /// system controller into the system to be used by the whole system.
  ///
  /// @param newlib - a pointer to the current platform's
  ///        system controller.
  static void Set(Newlib * newlib)
  {
    platform_newlib = newlib;
  }

  /// Get the system controller set for this platform.
  /// After main() is called by the startup code, this function will return a
  /// valid system controller. It is required that each platform startup routine
  /// set a system controller using the `void Set(Newlib&)` static
  /// method.
  static Newlib & Get()
  {
    return *platform_newlib;
  }

 protected:
  /// Global platform system controller scoped within this class. Most
  /// systems only need a single platform system controller, and thus this
  /// can hold a general/default platform system controller that can be
  /// retrieved via Set and Get.
  static inline Newlib * platform_newlib = &default_newlib;
};

inline void HandleExceptionPointer(std::exception_ptr exception_pointer)
{
  printf(SJ2_BACKGROUND_RED "Uncaught exception: ");
  try
  {
    if (exception_pointer)
    {
      std::rethrow_exception(exception_pointer);
    }
  }
  catch (const std::exception & e)
  {
    printf("std::exception(%s)\n", e.what());
  }
  catch (sjsu::Exception & e)
  {
    e.Print();
  }
  catch (...)
  {
    printf("Unknown...\n");
  }

  puts(SJ2_COLOR_RESET);
}
}  // namespace sjsu

extern "C"
{
  // Dummy implementation of getpid
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _getpid()
  {
    return 1;
  }

  // Dummy implementation of kill
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _kill(int, int)
  {
    return -1;
  }

  // Dummy implementation of fstat, makes the assumption that the "device"
  // representing, in this case STDIN, STDOUT, and STDERR as character devices.
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _fstat([[maybe_unused]] int file, struct stat * status)
  {
    status->st_mode = S_IFCHR;
    return 0;
  }

  // Dummy implementation of _lseek
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _lseek_r([[maybe_unused]] int file,
                      [[maybe_unused]] int ptr,
                      [[maybe_unused]] int dir)
  {
    return 0;
  }

  // Dummy implementation of close
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _close_r([[maybe_unused]] int file)
  {
    return -1;
  }

  // Dummy implementation of isatty
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _isatty_r([[maybe_unused]] int file)
  {
    return 1;
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  inline void * _sbrk(int increment)
  {
    void * new_memory_location = 0;
    for (auto allocator : sjsu::NewlibManager::Get().GetHeapAllocators())
    {
      new_memory_location = allocator(increment);
      if (new_memory_location != nullptr)
      {
        break;
      }
    }

    if (new_memory_location == nullptr)
    {
      // Check that by allocating this space, we do not exceed the heap area.
      // If so, set new_memory_location to nullptr
      if ((heap_position + increment) <= &heap_end)
      {
        new_memory_location = static_cast<void *>(heap_position);
        heap_position += increment;
      }
    }

    return new_memory_location;
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _write(int file, const char * source_buffer, int length)
  {
    for (auto writer : sjsu::NewlibManager::Get().GetWriter())
    {
      writer(file, source_buffer, length);
    }
    return length;
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  inline int _read(int file, char * destination_buffer, int length)
  {
    int bytes_read = 0;
    for (auto reader : sjsu::NewlibManager::Get().GetReader())
    {
      bytes_read = reader(file, destination_buffer, length);
      if (bytes_read > 0)
      {
        break;
      }
    }
    return bytes_read;
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  inline void _exit(int return_code)
  {
    // Print based on return code value
    if (return_code >= 0)
    {
      printf("\n" SJ2_BOLD_WHITE SJ2_BACKGROUND_GREEN
             "Program Returned Exit Code: %d\n" SJ2_COLOR_RESET,
             return_code);
    }
    else
    {
      printf("\n" SJ2_BOLD_WHITE SJ2_BACKGROUND_RED
             "Program Returned Exit Code: %d\n" SJ2_COLOR_RESET,
             return_code);
    }

    sjsu::HandleExceptionPointer(std::current_exception());

    while (1)
    {
      continue;
    }
  }

  // Overload default libnano putchar() with a more optimal version that does
  // not use dynamic memory
  inline int putchar(int character)  // NOLINT
  {
    char character_value = static_cast<char>(character);
    return _write(0, &character_value, 1);
  }

  // Overload default libnano puts() with a more optimal version that does
  // not use dynamic memory
  inline int puts(const char * str)  // NOLINT
  {
    int string_length = static_cast<int>(strlen(str));
    int result        = 0;

    result += _write(0, str, string_length);
    result += _write(0, "\n", 1);

    return result;
  }

  // Overload default libnano puts() with a more optimal version that does
  // not use dynamic memory
  inline int fputs(const char * str, FILE * file)  // NOLINT
  {
    int string_length    = static_cast<int>(strlen(str));
    int result           = 0;
    intptr_t file_intptr = reinterpret_cast<intptr_t>(file);
    int file_int         = static_cast<int>(file_intptr);

    result += _write(static_cast<int>(file_int), str, string_length);
    return result;
  }
}

namespace sjsu
{
/// Call this anywhere in the code to cause the compiler to pull in all of the
/// newlib implementation functions. This function has no cost when called. With
/// little optimization, the cost will be a single function call with a return,
/// which should only be done once or with optimization, this call should be
/// eliminated entierly, with the symbols still in the final binary.
inline void AddNewlibSymbols()
{
  static void * newlib_function_symols[] [[gnu::used]] = {
    reinterpret_cast<void *>(_getpid),  reinterpret_cast<void *>(_kill),
    reinterpret_cast<void *>(_fstat),   reinterpret_cast<void *>(_lseek_r),
    reinterpret_cast<void *>(_close_r), reinterpret_cast<void *>(_isatty_r),
    reinterpret_cast<void *>(_sbrk),    reinterpret_cast<void *>(_write),
    reinterpret_cast<void *>(_read),    reinterpret_cast<void *>(_exit),
  };
}
}  // namespace sjsu