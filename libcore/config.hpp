/// @mainpage SJSU-Dev2 Reference Page
///
/// @section welcome_sec Welcome
/// This is the reference API documentation for the SJSU-Dev2 Project. The
/// reference material should be used as such. This page is not a good place to
/// learn how to use SJSU-Dev2, but instead should be used to get details about
/// specific interfaces/classes/implementations/data structures/functions/
/// macros used in SJSU-Dev2.
///
/// <br />
///
/// @section class_api_sec Finding Class APIs
/// The easiest way to get started with the reference material is to go to the
/// "classes list" page to see all of the classes and data structures used in
/// SJSU-Dev2.
///
/// <br />
///
/// @section namespace_sec Finding everything within the SJSU Namespace
/// See the the "namespace" page and open the sjsu drop down to see everything
/// contained within the sjsu namespace. This will include data structures,
/// templated functions, class interfaces (not their descendants), and
/// everything contained within the subnamespaces of sjsu.
///
/// <br />
///
/// @section support_sec Contact & Support
/// If you want to contribute to SJSU-Dev2, check out the documentation
/// <a
///  href="https://sjsu-dev2.readthedocs.io/en/latest/contributing/philosophy/">
///  here</a>.
///
/// If you spotted a bug, need a feature or have an awesome idea, post an issue
/// on the SJSU-Dev2 github
/// <a href="https://github.com/SJSU-Dev2/SJSU-Dev2/issues">issues</a> page.
///
/// <br />
///
/// @section license_sec License
///
/// <a href="https://github.com/SJSU-Dev2/SJSU-Dev2/blob/master/LICENSE">
/// Apache License Version 2.0
/// </a>
///

#pragma once

#include <cstddef>
#include <cstdint>

// Include using <> instead of "" for  to make sure we only grab the project
// version of project_config.hpp
#include <project_config.hpp>

#include "log_levels.hpp"

namespace config
{
/// @defgroup config_group Configuration
/// @brief Lists all of the configuration options for a SJSU-Dev2 project.
///
/// How to properly use global configuration option:
/// ================================================
/// If you are using the preprocessor "if", then use the macro name directly.
/// Otherwise always use the kConstant type. Since this section is namespaced,
/// don't forget to include the config:: scope for the kConstant. Example:
/// config::kConstant.
///
/// How to add a new global configuration option:
/// ==============================================
/// 1) Check if the macro is already defined. The macros should only be changed
///    in the <project_config.hpp> file
///
/// 2) If the macro is not defined, give it a default value here.
///
/// 3) Generate a typed constexpr version of the macro using
///    SJ2_DECLARE_CONSTANT This will check that the desired typed variable,
///    and the macro equal each other. They wouldn't in cases where the type is
///    bool and the user uses the value 2 for the macro.
///
/// 4) It is recommend, that if there exists an acceptable range for a constant,
///    then use a static_assert to check that the constant generated from
///    SJ2_DECLARE_CONSTANT, is within range. For example if kSystemClockRate
///    can only be between 1Hz and 100Mhz, then kSystemClockRate should be
///    checked if it is within range.
///

/// Creates a typed constexpr version of the macro defintion which should be
/// used rather than using the macro directly.
#define SJ2_DECLARE_CONSTANT(macro_name, type, constant_name) \
  constexpr type constant_name = SJ2_##macro_name;            \
  static_assert(constant_name == SJ2_##macro_name,            \
                "SJ2_" #macro_name " must be of type '" #type "'")

/// Used to enable and disable the sending of ANSI color escape codes via the
/// ansi_terminal_codes.hpp
#if !defined(SJ2_ENABLE_ANSI_CODES)
#define SJ2_ENABLE_ANSI_CODES true
#endif  // !defined(SJ2_ENABLE_ANSI_CODES)
/// Delcare Constant ENABLE_ANSI_CODES
SJ2_DECLARE_CONSTANT(ENABLE_ANSI_CODES, bool, kEnableAnsiCodes);
}  // namespace config
