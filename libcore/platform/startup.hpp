#pragma once

namespace sjsu
{
#ifndef HOST_TEST
extern void InitializePlatform();
#else
inline void InitializePlatform() {}
#endif
}  // namespace sjsu

#define REDUCE_NEWLIB_MEMORY_USAGE()                       \
  namespace __cxxabiv1                                     \
  {                                                        \
  /* Override default terminate handler to just abort. */  \
  std::terminate_handler __terminate_handler = std::abort; \
  } /* namespace __cxxabiv1 */
