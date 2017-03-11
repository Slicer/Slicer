


#! Usage:
#! \code
#! simple_test(<testname> [argument1 ...])
#! \endcode
#!
#! This macro add a test using the complete add_test signature specifying target using
#! $<TARGET_FILE:...> generator expression. Optionnal test argument(s) can be passed
#! after specifying the <testname>.
#!
#! Variable named KIT (or CLP or EXTENSION_NAME) is expected to be defined in the current scope.
#! KIT (or CLP or EXTENSION_NAME) variable usually matches the value of PROJECT_NAME.
#!
#! The macro also associates a label to the test based on the current value of KIT (or CLP or EXTENSION_NAME).
#!
#! \sa http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:add_test
#! \sa http://www.cmake.org/cmake/help/cmake-2-8-docs.html#variable:PROJECT_NAME
#!
#! \ingroup CMakeUtilities
macro(simple_test testname)

  if("${KIT}" STREQUAL "")
    set(KIT ${CLP})
  endif()

  if("${KIT}" STREQUAL "")
    set(KIT ${EXTENSION_NAME})
  endif()

  if("${KIT}" STREQUAL "")
    message(FATAL_ERROR "error: KIT or CLP or EXTENSION_NAME variable is not set !")
  endif()

  if("${Slicer_LAUNCH_COMMAND}" STREQUAL "")
    message(FATAL_ERROR "error: Slicer_LAUNCH_COMMAND variable is not set !")
  endif()

  if(NOT TARGET ${KIT}CxxTests)
    message(FATAL_ERROR "error: Target '${KIT}CxxTests' is not defined !")
  endif()

  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET} NAME ${testname}
           COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${testname} ${ARGN})
  set_property(TEST ${testname} PROPERTY LABELS ${KIT})
endmacro()

