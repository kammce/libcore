#pragma once

#include <string_view>

namespace sjsu
{
namespace build
{

#if !defined(PLATFORM)
#define PLATFORM_STRING "unknown"
#else
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define PLATFORM_STRING STR(PLATFORM)
#endif

constexpr const std::string_view kPlatform = PLATFORM_STRING;

constexpr bool IsPlatform(std::string_view platform)
{
  return kPlatform.starts_with(platform);
}
}  // namespace build
}  // namespace sjsu
