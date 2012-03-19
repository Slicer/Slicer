
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
include(CTest)
include(SlicerMacroSimpleTest)
include(SlicerMacroPythonTesting)
include(SlicerMacroConfigureGenericCxxModuleTests)
include(SlicerMacroConfigureGenericPythonModuleTests)

include(SlicerBlockUploadExtensionPrerequisites)
set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
  set(EXTENSION_SUPERBUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
if(NOT DEFINED EXTENSION_BUILD_SUBDIRECTORY)
  set(EXTENSION_BUILD_SUBDIRECTORY ".")
endif()
if(NOT DEFINED EXTENSION_ENABLED)
  set(EXTENSION_ENABLED 1)
endif()
message(STATUS "Setting EXTENSION_SOURCE_DIR ..........: ${EXTENSION_SOURCE_DIR}")
message(STATUS "Setting EXTENSION_SUPERBUILD_BINARY_DIR: ${EXTENSION_SUPERBUILD_BINARY_DIR}")
message(STATUS "Setting EXTENSION_BUILD_SUBDIRECTORY ..: ${EXTENSION_BUILD_SUBDIRECTORY}")
message(STATUS "Setting EXTENSION_BINARY_DIR ..........: ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}")

set(CTEST_MODEL "Experimental")
include(SlicerBlockUploadExtension)
add_custom_target(Experimental
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )
add_custom_target(ExperimentalUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )

set(CTEST_MODEL "Nightly")
include(SlicerBlockUploadExtension)
add_custom_target(Nightly
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )
add_custom_target(NightlyUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )
