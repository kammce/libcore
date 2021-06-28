#pragma once

namespace sjsu
{
#ifndef HOST_TEST
extern void InitializePlatform();
#else
inline void InitializePlatform() {}
#endif
}  // namespace sjsu