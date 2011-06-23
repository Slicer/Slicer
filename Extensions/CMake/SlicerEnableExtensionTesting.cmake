
# If no CTestConfig.cmake file is found in source directory,
# one will be generated.
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/CTestConfig.cmake")
  message(STATUS "CTestConfig.cmake has been written to: ${CMAKE_SOURCE_DIR}")
  file(WRITE ${CMAKE_SOURCE_DIR}/CTestConfig.cmake
"set(CTEST_PROJECT_NAME \"Slicer\")
set(CTEST_NIGHTLY_START_TIME \"3:00:00 UTC\")

set(CTEST_DROP_METHOD \"http\")
set(CTEST_DROP_SITE \"www.cdash.org\")
set(CTEST_DROP_LOCATION \"/slicer4/submit.php?project=Slicer4\")
set(CTEST_DROP_SITE_CDASH TRUE)")
endif()

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
include(CTest)
include(SlicerMacroPythonTesting)
include(SlicerMacroConfigureGenericCxxModuleTests)
include(SlicerMacroConfigureGenericPythonModuleTests)

include(SlicerBlockUploadExtensionPrerequisites)
set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTENSION_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
include(SlicerBlockUploadExtension)

add_custom_target(Experimental
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )

add_custom_target(ExperimentalUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )


