
# If no CTestConfig.cmake file is found in source directory,
# one will be generated.
IF(NOT EXISTS "${CMAKE_SOURCE_DIR}/CTestConfig.cmake")
  MESSAGE(STATUS "CTestConfig.cmake has been written to: ${CMAKE_SOURCE_DIR}")
  FILE(WRITE ${CMAKE_SOURCE_DIR}/CTestConfig.cmake
"set(CTEST_PROJECT_NAME \"Slicer\")
set(CTEST_NIGHTLY_START_TIME \"3:00:00 UTC\")

set(CTEST_DROP_METHOD \"http\")
set(CTEST_DROP_SITE \"www.cdash.org\")
set(CTEST_DROP_LOCATION \"/slicer4/submit.php?project=Slicer4\")
set(CTEST_DROP_SITE_CDASH TRUE)")
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
INCLUDE(CTest)
INCLUDE(SlicerMacroPythonTesting)
INCLUDE(SlicerMacroConfigureGenericCxxModuleTests)
INCLUDE(SlicerMacroConfigureGenericPythonModuleTests)

INCLUDE(SlicerBlockUploadExtensionPrerequisites)
SET(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(EXTENSION_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE(SlicerBlockUploadExtension)

ADD_CUSTOM_TARGET(Experimental
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )

ADD_CUSTOM_TARGET(ExperimentalUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )


