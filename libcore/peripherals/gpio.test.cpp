#include <libcore/testing/testing_frameworks.hpp>
#include <libcore/peripherals/gpio.hpp>

namespace sjsu
{
TEST_CASE("Testing L1 gpio")
{
  Mock<sjsu::Gpio> mock_gpio;
  Fake(Method(mock_gpio, SetDirection));
  Fake(Method(mock_gpio, Set));

  sjsu::Gpio & gpio = mock_gpio.get();

  SECTION("SetAsInput()")
  {
    // Exercise
    gpio.SetAsInput();

    // Verify
    Verify(
        Method(mock_gpio, SetDirection).Using(sjsu::Gpio::Direction::kInput));
  }

  SECTION("SetAsOutput()")
  {
    // Exercise
    gpio.SetAsOutput();

    // Verify
    Verify(
        Method(mock_gpio, SetDirection).Using(sjsu::Gpio::Direction::kOutput));
  }

  SECTION("SetHigh()")
  {
    // Exercise
    gpio.SetHigh();

    // Verify
    Verify(Method(mock_gpio, Set).Using(sjsu::Gpio::State::kHigh));
  }

  SECTION("SetLow()")
  {
    // Exercise
    gpio.SetLow();

    // Verify
    Verify(Method(mock_gpio, Set).Using(sjsu::Gpio::State::kLow));
  }

  SECTION("PullUp()")
  {
    // Exercise
    gpio.settings.PullUp();

    // Verify
    CHECK(gpio.settings.resistor == PinSettings_t::Resistor::kPullUp);
  }

  SECTION("PullDown()")
  {
    // Exercise
    gpio.settings.PullDown();

    // Verify
    CHECK(gpio.settings.resistor == PinSettings_t::Resistor::kPullDown);
  }

  SECTION("Floating()")
  {
    // Exercise
    gpio.settings.Floating();

    // Verify
    CHECK(gpio.settings.resistor == PinSettings_t::Resistor::kNone);
  }
}
}  // namespace sjsu
