
#
# This module will add a target named 'packageupload'.
#
# It has been designed to be included in the build system of Slicer.
#
# The new target will
#  (1) build the standard 'package' target,
#  (2) extract the list of generated packages from its standard output,
#  (3) append the list of generated package filepaths to a file named PACKAGES.txt,
#  (4) upload the first package on midas.
#
# The following variables are expected to be defined in the including scope:
#  Slicer_SOURCE_DIR
#  Slicer_BINARY_DIR
#  Slicer_OS
#  Slicer_ARCHITECTURE
#  MIDAS_PACKAGE_URL
#  MIDAS_PACKAGE_EMAIL
#  MIDAS_PACKAGE_API_KEY
#
# Optionally, these variable can also be set:
#  CTEST_MODEL (default: Experimental)
#
# Then, using  the 'SlicerMacroExtractRepositoryInfo' CMake module, the script
# will also set the following variables:
#  Slicer_REVISION
#  Slicer_WC_LAST_CHANGED_DATE
#
# Finally, each time the 'packageupload' target is built, this same module will be
# executed with all variables previously defined as arguments.
#

# Macro allowing to set a variable to its default value if not already defined.
# The default value is set with:
#  (1) if set, the value environment variable <var>.
#  (2) if set, the value of local variable variable <var>.
#  (3) if none of the above, the value passed as a parameter.
# Setting the optional parameter 'OBFUSCATE' will display 'OBFUSCATED' instead of the real value.
macro(_sput_set_if_not_defined var defaultvalue)
  set(_obfuscate FALSE)
  foreach(arg ${ARGN})
    if(arg STREQUAL "OBFUSCATE")
      set(_obfuscate TRUE)
    endif()
  endforeach()
  if(DEFINED ENV{${var}} AND NOT DEFINED ${var})
    set(_value "$ENV{${var}}")
    if(_obfuscate)
      set(_value "OBFUSCATED")
    endif()
    message(STATUS "Setting '${var}' variable with environment variable value '${_value}'")
    set(${var} $ENV{${var}})
  endif()
  if(NOT DEFINED ${var})
    set(_value "${defaultvalue}")
    if(_obfuscate)
      set(_value "OBFUSCATED")
    endif()
    message(STATUS "Setting '${var}' variable with default value '${_value}'")
    set(${var} "${defaultvalue}")
  endif()
endmacro()

if(NOT DEFINED PACKAGEUPLOAD)
  set(PACKAGEUPLOAD 0)
endif()

if(NOT PACKAGEUPLOAD)

  _sput_set_if_not_defined(CTEST_MODEL "Experimental")
  _sput_set_if_not_defined(MIDAS_PACKAGE_URL "http://slicer.kitware.com/midas3")
  _sput_set_if_not_defined(MIDAS_PACKAGE_EMAIL "MIDAS_PACKAGE_EMAIL-NOTDEFINED" OBFUSCATE)
  _sput_set_if_not_defined(MIDAS_PACKAGE_API_KEY "MIDAS_PACKAGE_API_KEY-NOTDEFINED" OBFUSCATE)

  set(script_vars
    Slicer_CMAKE_DIR
    Slicer_BINARY_DIR
    Slicer_OS
    Slicer_ARCHITECTURE
    CTEST_MODEL
    MIDAS_PACKAGE_URL
    MIDAS_PACKAGE_EMAIL
    MIDAS_PACKAGE_API_KEY
    )

  # Sanity checks
  set(expected_defined_vars
    Slicer_SOURCE_DIR
    ${script_vars}
    )
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} is not defined !")
    endif()
  endforeach()

  # Get working copy information
  include(SlicerMacroExtractRepositoryInfo)
  SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer SOURCE_DIR ${Slicer_SOURCE_DIR})

  # Given a date formatted like "2019-01-15 22:08:54 -0500 (Tue, 15 Jan 2019)", only
  # keep "2019-01-15 22:08:54".
  string(REGEX REPLACE "^([0-9][0-9][0-9][0-9]\\-[0-9][0-9]\\-[0-9][0-9] [0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]).*"
    "\\1" Slicer_WC_LAST_CHANGED_DATE "${Slicer_WC_LAST_CHANGED_DATE}")

  set(script_arg_list)
  foreach(varname
    ${script_vars}
    # Optional variables
    CTEST_MODEL
    # Variables set by SlicerMacroExtractRepositoryInfo
    Slicer_REVISION
    Slicer_WC_LAST_CHANGED_DATE
    )
    if(NOT DEFINED ${varname})
      message(FATAL_ERROR "Variable ${varname} is expected to be defined.")
    endif()
    set(script_arg_list "${script_arg_list}
set(${varname} \"${${varname}}\")")
  endforeach()

  set(script_args_file ${CMAKE_CURRENT_BINARY_DIR}/midas_api_upload_package-command-args.cmake)
  file(WRITE ${script_args_file} ${script_arg_list})

  set(_cpack_output_file ${Slicer_BINARY_DIR}/packageupload_cpack_output.txt)

  add_custom_target(packageupload
    COMMAND ${CMAKE_COMMAND} -E echo "CPack log: ${_cpack_output_file}"
    COMMAND ${CMAKE_COMMAND}
      -DPACKAGEUPLOAD:BOOL=1
      -DCONFIG:STRING=${CMAKE_CFG_INTDIR}
      -DCPACK_OUTPUT_FILE:FILEPATH=${_cpack_output_file}
      -DSCRIPT_ARGS_FILE:FILEPATH=${script_args_file}
      -P ${CMAKE_CURRENT_LIST_FILE}
    COMMENT "Package and upload"
    )
  return()
endif()

#-----------------------------------------------------------------------------
# Package and upload
#-----------------------------------------------------------------------------

# Build package target and extract list of generated packages

if(NOT EXISTS "${SCRIPT_ARGS_FILE}")
  message(FATAL_ERROR "Argument 'SCRIPT_ARGS_FILE' is either missing or pointing to an nonexistent file !")
endif()
include(${SCRIPT_ARGS_FILE})

# Sanity checks
set(expected_defined_vars
  CONFIG
  CPACK_OUTPUT_FILE
  )
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# The following code will build the 'package' target, extract the list
# of generated packages from its standard output and create a file PACKAGES.txt
# containing the list of package paths.

# Setting the environment variable SLICER_PACKAGE_UPLOAD_SKIP_PACKAGING_TARGET to
# any non empty value can be used when testing this module. It avoids to wait for a rebuild
# of the project.
set(_build_target 1)
if(NOT "$ENV{SLICER_PACKAGE_UPLOAD_SKIP_PACKAGING_TARGET}" STREQUAL "")
  set(_build_target 0)
endif()

if(_build_target)
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${Slicer_BINARY_DIR} --target package --config ${CONFIG}
    WORKING_DIRECTORY ${Slicer_BINARY_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_FILE ${CPACK_OUTPUT_FILE}
    RESULT_VARIABLE rv
    )
else()
  set(rv 0)
endif()

# Display CPack output
file(READ ${CPACK_OUTPUT_FILE} cpack_output)
message(${cpack_output})

if(NOT rv EQUAL 0)
  message(FATAL_ERROR "Failed to package project: ${Slicer_BINARY_DIR}")
endif()

# File containing the list of filepath corresponding to the generated packages
# or installers
set(package_list_file ${Slicer_BINARY_DIR}/PACKAGES.txt)

# Clear file
file(WRITE ${package_list_file} "")

# Extract list of generated packages
set(regexp ".*CPack: - package: (.*) generated\\.")
set(raw_package_list)
file(STRINGS ${CPACK_OUTPUT_FILE} raw_package_list REGEX ${regexp})

foreach(package ${raw_package_list})
  string(REGEX REPLACE ${regexp} "\\1" package_path "${package}" )
  file(APPEND ${package_list_file} "${package_path}\n")
endforeach()


#-----------------------------------------------------------------------------
# The following code will read the list of created packages from PACKAGES.txt
# file and upload the first one to midas.

# Current assumption: Exactly one Slicer package is expected. If this
# ever changes. The following code would have to be updated.

file(STRINGS ${Slicer_BINARY_DIR}/PACKAGES.txt package_list)

list(LENGTH package_list package_count)
if(package_count EQUAL 0)
  message(FATAL_ERROR "Slicer package failed to be generated.")
endif()

set(CMAKE_MODULE_PATH
  ${Slicer_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  )

include(MIDASAPIUploadPackage)

# Upload generated extension packages to midas
set(package_uploaded 0)
foreach(p ${package_list})
  if(package_uploaded)
    message(WARNING "Skipping additional package: ${p}")
  else()
    get_filename_component(package_name "${p}" NAME)
    message("Uploading [${package_name}] on [${MIDAS_PACKAGE_URL}]")
    midas_api_upload_package(
      SERVER_URL ${MIDAS_PACKAGE_URL}
      SERVER_EMAIL ${MIDAS_PACKAGE_EMAIL}
      SERVER_APIKEY ${MIDAS_PACKAGE_API_KEY}
      SUBMISSION_TYPE ${CTEST_MODEL}
      SOURCE_REVISION ${Slicer_REVISION}
      SOURCE_CHECKOUTDATE ${Slicer_WC_LAST_CHANGED_DATE}
      OPERATING_SYSTEM ${Slicer_OS}
      ARCHITECTURE ${Slicer_ARCHITECTURE}
      PACKAGE_FILEPATH ${p}
      PACKAGE_TYPE "installer"
      RESULT_VARNAME slicer_midas_upload_status
      )
    set(package_uploaded 1)
    if(NOT slicer_midas_upload_status STREQUAL "ok")
      file(WRITE ${Slicer_BINARY_DIR}/PACKAGES.txt "")
      message(FATAL_ERROR
"Upload of [${package_name}] failed !
Check that:
(1) you have been granted permission to upload
(2) your email and api key are correct")
    endif()
  endif()
endforeach()
