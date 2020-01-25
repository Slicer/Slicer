
#
# Check that TestExtC generated description file is valid.
#

cmake_minimum_required(VERSION 3.13.4)

# Sanity checks
set(expected_existing_vars
  Slicer_EXTENSIONS_CMAKE_DIR
  TestExtC_BUILD_DIR
  )
foreach(var ${expected_existing_vars})
  if(NOT EXISTS ${${var}})
    message(FATAL_ERROR "error: ${var} points to an inexistent directory: ${${var}}")
  endif()
endforeach()

set(PROJECT_NAME "TestExtC")

include(${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerFunctionExtractExtensionDescription.cmake)

set(description_file ${TestExtC_BUILD_DIR}/${PROJECT_NAME}.s4ext)
slicerFunctionExtractExtensionDescription(
  EXTENSION_FILE ${description_file}
  VAR_PREFIX TEST
)
set(errmsg "Problem with generated description file ${description_file}")
function(check_extension_metadata metadata_name expected)
  if(NOT "${TEST_EXT_${metadata_name}}" STREQUAL "${expected}")
    message(FATAL_ERROR "${errmsg}
Incorrect value for TEST_EXT_${metadata_name} metadata
  Actual [${TEST_EXT_${metadata_name}}]
  Expected [${expected}]")
  endif()
endfunction()

check_extension_metadata(
  "HOMEPAGE"
  "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/TestExtC"
  )
check_extension_metadata(
  "CATEGORY"
  "Examples"
  )
check_extension_metadata(
  "CONTRIBUTORS"
  "John Doe (AnyWare Corp.)"
  )
check_extension_metadata(
  "DESCRIPTION"
  "This is an example of a simple extension"
  )
check_extension_metadata(
  "ICONURL"
  "http://www.example.com/Slicer/Extensions/TestExtC.png"
  )
check_extension_metadata(
  "SCREENSHOTURLS"
  "http://www.example.com/Slicer/Extensions/TestExtC/Screenshots/1.png"
  )
check_extension_metadata(
  "DEPENDS"
  "TestExtA;TestExtB"
  )
check_extension_metadata(
  "STATUS"
  ""
  )
check_extension_metadata(
  "BUILD_SUBDIRECTORY"
  "."
  )
check_extension_metadata(
  "ENABLED"
  "1"
  )
foreach(metadata_name IN ITEMS SCM SCMREVISION SCMURL)
  if(${TEST_EXT_${metadata_name}} STREQUAL "")
    message(FATAL_ERROR "${errmsg}
Incorrect value for TEST_EXT_${metadata_name} metadata
Value should NOT be an empty string.")
  endif()
endforeach()
