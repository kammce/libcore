#pragma once

#include <cstdint>
#include <cstring>

extern size_t _data_size;
extern uint32_t _data;
extern uint32_t _data_load;
extern size_t _bss_size;
extern uint32_t _bss;
extern uint8_t _heap;
extern uint8_t _heap_end;
inline uint8_t * heap_position = &_heap;

namespace sjsu
{
/// Copies the defined variabes within the .data section in ROM into RAM
inline void InitializeDataSection()
{
  memcpy(&_data, &_data_load, reinterpret_cast<size_t>(&_data_size));
}

/// Initializes the .bss section of RAM. The STD C libraries assume that BSS is
/// set to zero and will fault otherwise.
inline void InitializeBssSection()
{
  memset(&_bss, 0UL, reinterpret_cast<size_t>(&_bss_size));
}
}  // namespace sjsu
