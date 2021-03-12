#include <string>

#include <libcore/testing/testing_frameworks.hpp>
#include <libcore/utility/build_info.hpp>

namespace sjsu
{
namespace build
{
TEST_CASE("Testing Build Info")
{
  SECTION("IsPlatform(Platform)")
  {
    static_assert(IsPlatform("host"));
    static_assert(!IsPlatform("lpc40xx"));
    static_assert(!IsPlatform("lpc17xx"));
    static_assert(!IsPlatform("lpc"));
    static_assert(!IsPlatform("stm32f10x"));
    static_assert(!IsPlatform("stm"));
    static_assert(!IsPlatform("linux"));
  }
}
}  // namespace build
}  // namespace sjsu
