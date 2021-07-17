/// @ingroup SJSU-Dev2
/// @defgroup Log Utility Functions
/// @brief This module is meant for general purpose macros that can be used
/// across the SJSU-Dev2 environment.
/// @{
#pragma once

#include <chrono>
#include <experimental/source_location>
#include <libcore/utility/ansi_terminal_codes.hpp>
#include <libcore/utility/time/time.hpp>
#include <string_view>
#include <type_traits>

#define FMT_STATIC_THOUSANDS_SEPARATOR ','
#define FMT_USE_FLOAT 0
#define FMT_USE_DOUBLE 0
#define FMT_USE_LONG_DOUBLE 0
#define FMT_REDUCE_INT_INSTANTIATIONS 1
#define FMT_EXCEPTIONS 0
#define NDEBUG
#define FMT_HEADER_ONLY

#ifndef INFO_LOGS
#define INFO_LOGS 0
#endif

#ifndef DEBUG_LOGS
#define DEBUG_LOGS 0
#endif

#ifndef ENABLE_LOGS
#define ENABLE_LOGS 1
#endif

#include <libcore/external/fmt/include/fmt/core.h>

namespace sjsu::log
{
using DecoratorFunction =
    std::function<void(const std::experimental::source_location &,
                       std::chrono::nanoseconds)>;

struct Decorators
{
  static inline DecoratorFunction info_prefix =
      [](const std::experimental::source_location & location,
         std::chrono::nanoseconds nanos)
  {
    if constexpr ((DEBUG_LOGS || INFO_LOGS) && ENABLE_LOGS)
    {
      fmt::print(
          "{}:{}:{}:{}s> " SJ2_HI_BLACK,
          location.file_name(),
          location.line(),
          location.function_name(),
          std::chrono::duration_cast<std::chrono::seconds>(nanos).count());
    }
  };
  static inline DecoratorFunction debug_prefix =
      [](const std::experimental::source_location & location,
         std::chrono::nanoseconds nanos)
  {
    if constexpr (DEBUG_LOGS && ENABLE_LOGS)
    {
      fmt::print(
          "{}:{}:{}:{}s> " SJ2_HI_YELLOW,
          location.file_name(),
          location.line(),
          location.function_name(),
          std::chrono::duration_cast<std::chrono::seconds>(nanos).count());
    }
  };
  static inline DecoratorFunction print_prefix =
      [](const std::experimental::source_location & location,
         std::chrono::nanoseconds nanos)
  {
    if constexpr (ENABLE_LOGS)
    {
      fmt::print(
          "{}:{}:{}:{}s> " SJ2_HI_BOLD_WHITE,
          location.file_name(),
          location.line(),
          location.function_name(),
          std::chrono::duration_cast<std::chrono::seconds>(nanos).count());
    }
  };
  static inline DecoratorFunction critical_prefix =
      [](const std::experimental::source_location & location,
         std::chrono::nanoseconds nanos)
  {
    if constexpr (ENABLE_LOGS)
    {
      fmt::print(
          "{}:{}:{}:{}s> " SJ2_RED,
          location.file_name(),
          location.line(),
          location.function_name(),
          std::chrono::duration_cast<std::chrono::seconds>(nanos).count());
    }
  };

  static inline std::function<void(void)> info_suffix = []()
  {
    if constexpr ((DEBUG_LOGS || INFO_LOGS) && ENABLE_LOGS)
    {
      fmt::print(SJ2_COLOR_RESET);
    }
  };
  static inline std::function<void(void)> debug_suffix = []()
  {
    if constexpr (DEBUG_LOGS && ENABLE_LOGS)
    {
      fmt::print(SJ2_COLOR_RESET);
    }
  };
  static inline std::function<void(void)> print_suffix = []()
  {
    if constexpr (ENABLE_LOGS)
    {
      fmt::print(SJ2_COLOR_RESET);
    }
  };
  static inline std::function<void(void)> critical_suffix = []()
  {
    if constexpr (ENABLE_LOGS)
    {
      fmt::print(SJ2_COLOR_RESET);
    }
  };
};

/// Specialized log object that labels the log with a preceeding "DEBUG" label.
/// Will only log if the SJ2_LOG_LEVEL is level SJ2_LOG_LEVEL_DEBUG or greater.
///
/// @tparam Args - Variadic type array to describe the args variable pack.
template <size_t N, typename... Args>
struct Info  // NOLINT
{
  /// @param format - format string to be used for logging
  /// @param args - variadic list of parameters to be passed to the log object
  /// @param location - the location in the source code where this object was
  ///        constructed.
  Info(const char (&format)[N],
       Args... args,
       const std::experimental::source_location & location =
           std::experimental::source_location::current())
  {
    if constexpr ((DEBUG_LOGS || INFO_LOGS) && ENABLE_LOGS)
    {
      Decorators::info_prefix(location, sjsu::Uptime());
      fmt::print(fmt::runtime(format), args...);
      Decorators::info_suffix();
    }
  }
};

/// @tparam Args - Variadic type array to describe the args variable pack.
template <size_t N, typename... Args>
struct Debug  // NOLINT
{
  /// @param format - format string to be used for logging
  /// @param args - variadic list of parameters to be passed to the log object
  /// @param location - the location in the source code where this object was
  ///        constructed.
  Debug(const char (&format)[N],
        Args... args,
        const std::experimental::source_location & location =
            std::experimental::source_location::current())
  {
    if constexpr (DEBUG_LOGS && ENABLE_LOGS)
    {
      Decorators::debug_prefix(location, sjsu::Uptime());
      fmt::print(fmt::runtime(format), args...);
      Decorators::debug_suffix();
    }
  }
};

/// @tparam Args - Variadic type array to describe the args variable pack.
template <size_t N, typename... Args>
struct Print  // NOLINT
{
  /// @param format - format string to be used for logging
  /// @param args - variadic list of parameters to be passed to the log object
  /// @param location - the location in the source code where this object was
  ///        constructed.
  constexpr Print(const char (&format)[N],
                  Args... args,
                  const std::experimental::source_location & location =
                      std::experimental::source_location::current())
  {
    if constexpr (ENABLE_LOGS)
    {
      Decorators::print_prefix(location, sjsu::Uptime());
      fmt::print(fmt::runtime(format), args...);
      Decorators::print_suffix();
    }
  }
};

template <size_t N, typename... Args>
struct Critical  // NOLINT
{
  Critical(const char (&format)[N],
           Args... args,
           const std::experimental::source_location & location =
               std::experimental::source_location::current())
  {
    if constexpr (ENABLE_LOGS)
    {
      Decorators::critical_prefix(location, sjsu::Uptime());
      fmt::print(fmt::runtime(format), args...);
      Decorators::critical_suffix();
    }
  }
};

/// Deduction guide for Critical
template <size_t N, typename... Args>
Critical(const char (&format)[N], Args...) -> Critical<N, Args...>;

/// Deduction guide for Print
template <size_t N, typename... Args>
Print(const char (&format)[N], Args...) -> Print<N, Args...>;

/// Deduction guide for Info
template <size_t N, typename... Args>
Info(const char (&format)[N], Args...) -> Info<N, Args...>;

/// Deduction guide for Debug
template <size_t N, typename... Args>
Debug(const char (&format)[N], Args...) -> Debug<N, Args...>;

}  // namespace sjsu::log
