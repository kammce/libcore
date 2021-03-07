#define DOCTEST_CONFIG_IMPLEMENT

#include <unistd.h>

#include <span>

#include "platforms/utility/ram.hpp"
#include "newlib/newlib.hpp"
#include "testing_frameworks.hpp"

// =============================================================================
// Define Empty InitializePlatform()
// =============================================================================

namespace sjsu
{
void InitializePlatform() {}
}  // namespace sjsu

// =============================================================================
// Ram definitions for testing
// =============================================================================

namespace
{
struct DataSection_t
{
  int32_t a;
  uint8_t b;
  double d;
  uint16_t s;
};

DataSection_t rom = {
  .a = 15,
  .b = 'C',
  .d = 5.0,
  .s = 12'346U,
};

DataSection_t ram;

std::array<uint32_t, 128> bss_section;
}  // namespace

DataSectionTable_t data_section_table[] = {
  DataSectionTable_t{
      .rom_location = reinterpret_cast<uint32_t *>(&rom),
      .ram_location = reinterpret_cast<uint32_t *>(&ram),
      .length       = sizeof(rom),
  },
};

// NOTE: Not used, but must be defined
DataSectionTable_t data_section_table_end;

BssSectionTable_t bss_section_table[] = {
  BssSectionTable_t{
      .ram_location = bss_section.data(),
      .length       = bss_section.size(),
  },
};

// NOTE: Not used, but must be defined
BssSectionTable_t bss_section_table_end;

// =============================================================================
// Setup write() and read()
// =============================================================================

int HostTestWrite(std::span<const char> str)
{
  return static_cast<int>(write(1, str.data(), str.size()));
}

int HostTestRead(std::span<char> str)
{
  return static_cast<int>(read(1, str.data(), str.size()));
}

int main(int argc, char * argv[])
{
  doctest::Context context;

  sjsu::newlib::SetStdout(HostTestWrite);
  sjsu::newlib::SetStdin(HostTestRead);

  context.applyCommandLine(argc, argv);

  int res = context.run();  // run

  // important - query flags (and --exit) rely on the user doing this propagate
  // the result of the tests
  if (context.shouldExit())
  {
    return res;
  }

  int client_stuff_return_code = 0;
  // your program - if the testing framework is integrated in your production
  // code

  return res + client_stuff_return_code;  // the result from doctest is
                                          // propagated here as well
}
