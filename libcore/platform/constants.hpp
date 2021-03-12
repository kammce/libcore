#pragma once

namespace sjsu
{
/// ResourceID is a base class that represents an ID associated with a
/// resource managed by the SystemController. That ID association may be used
/// for methods to determine which peripheral needs to be powered up/down or
/// to obtain the clock rate of a resource.
class ResourceID  // NOLINT
{
 public:
  /// This helper function is used to make it easier to create additional
  /// ResourceID's.
  ///
  ///  Usage:
  ///
  ///    static constexpr auto kUart0 = ResourceID::Define<5>();
  ///    static constexpr auto kUart1 = ResourceID::Define<6>();
  ///    ...
  ///
  /// Typically the ID number used has some mapping to a register offset or
  /// bit offset within a register. For example, if we have a register for
  /// powering on peripherals and the 5th bit is for Uart0 then the ID for
  /// kUart0 should be 5. In many systems the mapping between clocks and other
  /// required things for a particular peripheral have the same numeric
  /// mapping where kUart's system clock register may be 5 x 4 bytes from the
  /// first register handling clock speeds. This may not always be true, and
  /// if not, a look table may be required to map IDs to the appropriate
  /// registers.
  ///
  /// @tparam id - compile time constant device ID for the peripheral
  template <int id>
  static constexpr ResourceID Define()
  {
    return { .device_id = id };
  }
  /// ID associated with the resource defined for this object.
  int device_id = -1;

  /// @param compare - the other resource to compare to this one to.
  /// @return true if their device_id's are equal.
  bool operator==(const ResourceID & compare) const
  {
    return device_id == compare.device_id;
  }
};
}  // namespace sjsu
