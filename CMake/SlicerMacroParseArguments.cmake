
#
# See http://www.cmake.org/Wiki/CMakeMacroParseArguments
#
macro(slicer_parse_arguments prefix arg_names option_names)
  message(FATAL_ERROR "Macro 'slicer_parse_arguments' has been removed. Consider using the built-in"
                      " macro 'cmake_parse_arguments' instead."
                      "See http://www.cmake.org/cmake/help/v2.8.10/cmake.html#module:CMakeParseArguments")
endmacro()
