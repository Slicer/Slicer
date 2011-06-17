
SET_PROPERTY(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
INCLUDE(CTest)

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


