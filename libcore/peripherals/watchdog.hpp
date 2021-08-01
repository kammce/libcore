#pragma once

#include <cstdint>
#include <libcore/module.hpp>
#include <libcore/utility/math/units.hpp>

namespace sjsu
{
/// Settings for the watchdog peripheral
struct WatchdogSettings_t
{
  /// The interval to trigger the watchdog if the system does not feed it in
  /// time.
  std::chrono::nanoseconds trigger_interval = 1s;
};

/// Abstract interface for a hardware watchdog timer peripheral which can be
/// used to determine if the system has become locked up, and if so, restarts
/// the system.
///
/// @ingroup l1_peripheral
class Watchdog : public sjsu::Module<WatchdogSettings_t>
{
 public:
  /// Feeds the watchdog and restarts the sequence.
  virtual void FeedSequence() const = 0;
};
}  // namespace sjsu
