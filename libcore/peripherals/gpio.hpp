#pragma once

#include <cstdint>
#include <functional>
#include <libcore/module.hpp>
#include <libcore/peripherals/inactive.hpp>
#include <libcore/peripherals/interrupt.hpp>
#include <libcore/utility/error_handling.hpp>

namespace sjsu
{
/// Generic settings for a standard chip's Pin peripheral
struct PinSettings_t : public MemoryEqualOperator_t<PinSettings_t>
{
  /// Defines the set of internal resistance connections to a pin. Read specific
  /// enumeration constant comments/documentation to understand more about what
  /// each one does.
  enum class Resistor : uint8_t
  {
    /// Disable resistor pull. If the pin is setup as high-z (input mode) and
    /// not connected to anything then the pin will be floating. Its value will
    /// not undefined.
    kNone = 0,

    /// Connect pin to ground using weak (high resistance) resistor.
    kPullDown,

    /// Connect pin to controller digital voltage (VCC) using a weak (high
    /// resistance) resistor.
    kPullUp,
  };

  /// Set the pin's function using a function code.
  /// The function code is very specific to the controller being used.
  ///
  /// But an example could be the LPC4078 chip's P0.0. It has the following
  /// functions:
  ///
  ///    0. GPIO (General purpose input or output) pin
  ///    1. CANBUS port 1 Read
  ///    2. UART port 3 transmitter
  ///    3. I2C port 1 serial data
  ///    4. and UART port 0 transmitter
  ///
  /// Each function listed above is listed with its function code. If you pass
  /// the value 4 into the ConfigureFunction function, for P0.0, it will set
  /// that pin to the UART port 0 transmitter function. After that, if you
  /// have properly enabled the UART hardware, when you attempt to send data
  /// through the uart0 port, it will show up on the pin.
  ///
  /// Please consult the user manual for the chip you are using to figure out
  /// which function codes correspond to specific functions.
  ///
  /// Generally, this method is only used laterally in the L1 layer. This is
  /// due to the fact that other L1s need to use this library to setup their
  /// external pins, but also due to the fact that a Hardware abstraction or
  /// above should not have to concern itself with function codes, thus it is
  /// the job of the L1 peripheral that uses this pin to manage its own
  /// function code usage.
  ///
  uint8_t function = 0;

  /// Set pin's resistor pull, setting ot either no resistor pull, pull down,
  /// pull up and repeater.
  Resistor resistor = Resistor::kPullUp;

  /// Make pin open drain
  bool open_drain = false;

  /// Put pin into analog mode
  bool as_analog = false;

  /// Set the pin to use the internal pull up resistor
  constexpr PinSettings_t PullUp()
  {
    resistor = Resistor::kPullUp;
    return *this;
  }

  /// Set the pin to use the internal pull up resistor
  constexpr PinSettings_t PullDown()
  {
    resistor = Resistor::kPullDown;
    return *this;
  }

  /// Set the pin to not use a pull resistor
  constexpr PinSettings_t Floating()
  {
    resistor = Resistor::kNone;
    return *this;
  }
};

/// An abstract interface for General Purpose I/O
/// @ingroup l1_peripheral
class Gpio : public Module<PinSettings_t>
{
 public:
  // ===========================================================================
  // Interface Defintions
  // ===========================================================================

  /// Defines the set of directions a GPIO can be.
  enum Direction : uint8_t
  {
    kInput  = 0,
    kOutput = 1
  };

  /// Defines what states a GPIO pin can be in.
  enum State : uint8_t
  {
    kLow  = 0,
    kHigh = 1
  };

  /// Defines the set of events that can trigger a GPIO interrupt.
  enum class Edge : uint8_t
  {
    kRising  = 0,
    kFalling = 1,
    kBoth    = 2
  };

  /// Set internal port and pin values.
  constexpr Gpio(uint8_t port, uint8_t pin) : port_(port), pin_(pin) {}

  /// Set pin as an output or an input
  ///
  /// NOTE: this method acts is the GPIO initialization, and must be called
  ///       first before calling any other method
  ///
  /// @param direction - which direction to set the pin to.
  virtual void SetDirection(Direction direction) = 0;

  /// Set the pin state as HIGH voltage or LOW voltage
  virtual void Set(State output) = 0;

  /// Toggles pin state. If the pin is HIGH, after this call it will be LOW
  /// and vise versa.
  virtual void Toggle() = 0;

  /// @return the state of the pin, note that this method does not consider
  ///         whether or not the active level is high or low. Simply returns the
  ///         state as depicted in memory
  virtual bool Read() = 0;

  /// Attach an interrupt call to a pin
  ///
  /// @param callback - the callback supplied here will be executed when the
  ///        interrupt condition occurs
  /// @param edge - the pin condition that will trigger the interrupt
  virtual void AttachInterrupt(InterruptCallback callback, Edge edge) = 0;

  /// Remove interrupt call from pin and deactivate interrupts for this pin
  virtual void DetachInterrupt() = 0;

  // ===========================================================================
  // Helper Functions
  // ===========================================================================

  /// Set pin to HIGH voltage
  void SetHigh()
  {
    Set(State::kHigh);
  }

  /// Set pin to LOW voltage
  void SetLow()
  {
    Set(State::kLow);
  }

  /// Set pin direction as input
  void SetAsInput()
  {
    SetDirection(Direction::kInput);
  }

  /// Set pin direction as output
  void SetAsOutput()
  {
    SetDirection(Direction::kOutput);
  }

  /// Set pin to run callback when the pin sees a rising edge
  ///
  /// @param callback - the function to be called when a rising edge event
  ///                   occurs on this pin.
  void OnRisingEdge(InterruptCallback callback)
  {
    return AttachInterrupt(callback, Edge::kRising);
  }

  /// Set pin to run callback when the pin sees a falling edge
  ///
  /// @param callback - the function to be called when a falling edge event
  ///                   occurs on this pin.
  void OnFallingEdge(InterruptCallback callback)
  {
    return AttachInterrupt(callback, Edge::kFalling);
  }

  /// Set pin to run callback when the pin sees a change on the pin's state.
  ///
  /// @param callback - the function to be called when a rising edge or falling
  ///                   edge event occurs on this pin.
  void OnChange(InterruptCallback callback)
  {
    return AttachInterrupt(callback, Edge::kBoth);
  }

  /// Getter method for the pin's port.
  ///
  /// @returns The pin's port.
  uint8_t GetPort() const
  {
    return port_;
  }

  /// Getter method for the pin's pin.
  ///
  /// @returns The pin's pin.
  uint8_t GetPin() const
  {
    return pin_;
  }

 private:
  const uint8_t port_;
  const uint8_t pin_;
};

/// Template specialization that generates an inactive sjsu::Gpio.
template <>
inline sjsu::Gpio & GetInactive<sjsu::Gpio>()
{
  class InactiveGpio : public sjsu::Gpio
  {
   public:
    InactiveGpio(uint8_t port, uint8_t pin) : sjsu::Gpio(port, pin) {}
    void ModuleInitialize() override {}
    void SetDirection(Direction) override {}
    void Set(State) override {}
    void Toggle() override {}
    bool Read() override
    {
      return false;
    }
    void AttachInterrupt(InterruptCallback, Edge) override {}
    void DetachInterrupt() override {}
  };

  static InactiveGpio inactive(0, 0);
  return inactive;
}
}  // namespace sjsu
