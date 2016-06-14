
#
# This module will set the variables Slicer_VERSION and Slicer_VERSION_FULL.
#
# It will also set all variables describing the SCM associated
# with Slicer_SOURCE_DIR.
#
# It has been designed to be included in the build system of Slicer.
#
# The following variables are expected to be defined in the including scope:
#  GIT_EXECUTABLE
#  Slicer_CMAKE_DIR
#  Slicer_MAIN_PROJECT_APPLICATION_NAME
#  Slicer_SOURCE_DIR
#  Slicer_VERSION_MAJOR
#  Slicer_VERSION_MINOR
#  Slicer_VERSION_PATCH
#  Subversion_SVN_EXECUTABLE
#
# Optionally, these variable can also be set:
#  Slicer_VERSION_TWEAK
#  Slicer_VERSION_RC
#

# --------------------------------------------------------------------------
# Sanity checks
# --------------------------------------------------------------------------
set(expected_defined_vars
  GIT_EXECUTABLE
  Slicer_CMAKE_DIR
  Slicer_MAIN_PROJECT_APPLICATION_NAME
  Slicer_SOURCE_DIR
  Slicer_VERSION_MAJOR
  Slicer_VERSION_MINOR
  Slicer_VERSION_PATCH
  Subversion_SVN_EXECUTABLE
  )
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is mandatory")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Update CMake module path
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH
  ${Slicer_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  )

include(SlicerMacroExtractRepositoryInfo)

#-----------------------------------------------------------------------------
# Slicer version number
#-----------------------------------------------------------------------------

SlicerMacroExtractRepositoryInfo(
  VAR_PREFIX Slicer
  SOURCE_DIR ${Slicer_SOURCE_DIR}
  )
string(REGEX REPLACE ".*([0-9][0-9][0-9][0-9]\\-[0-9][0-9]\\-[0-9][0-9]).*" "\\1"
  Slicer_BUILDDATE "${Slicer_WC_LAST_CHANGED_DATE}")

if(NOT Slicer_FORCED_WC_REVISION STREQUAL "")
  set(Slicer_WC_REVISION "${Slicer_FORCED_WC_REVISION}")
endif()

if("${Slicer_VERSION_TWEAK}" STREQUAL "")
  set(_version_qualifier "-${Slicer_BUILDDATE}")
elseif("${Slicer_VERSION_TWEAK}" GREATER 0)
  set(_version_qualifier "-${Slicer_VERSION_TWEAK}")
endif()

# XXX This variable should not be set explicitly
set(Slicer_VERSION      "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_VERSION_FULL "${Slicer_VERSION}.${Slicer_VERSION_PATCH}")
if(Slicer_VERSION_RC)
  set(Slicer_VERSION_FULL "${Slicer_VERSION_FULL}-rc${Slicer_VERSION_RC}")
endif()
set(Slicer_VERSION_FULL "${Slicer_VERSION_FULL}${_version_qualifier}")

message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} version [${Slicer_VERSION_FULL}]")
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} revision [${Slicer_WC_REVISION}]")
