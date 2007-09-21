PROJECT (EMSegmentBatchMakeModule)

# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

# Find BatchMake
FIND_PACKAGE( BatchMake REQUIRED )
INCLUDE( ${BatchMake_USE_FILE} )

# Find program to configure the module
IF(BatchMake_DIR)
  FIND_PROGRAM(bmGridSend
            NAMES bmGridSend.exe bmGridSend
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
                  ${BatchMake_DIR}/bin)

  FIND_PROGRAM(bmGridStore
            NAMES bmGridStore.exe bmGridStore
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
                  ${BatchMake_DIR}/bin)

  FIND_PROGRAM(bmSliceExtractor
            NAMES bmSliceExtractor.exe bmSliceExtractor
            PATHS ${BatchMake_DIR}/bin/relwithdebinfo
                  ${BatchMake_DIR}/bin/release
                  ${BatchMake_DIR}/bin/debug
                  ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
                  ${BatchMake_DIR}/bin)

  FIND_PATH(Slicer3_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  IF(Slicer3_BINARY_DIR)
  
    FIND_PROGRAM(EMSegmentCommandLine_GUIVersion
              NAMES EMSegmentCommandLine_GUIVersion.exe EMSegmentCommandLine_GUIVersion
              PATHS ${Slicer3_BINARY_DIR}/bin/relwithdebinfo
                    ${Slicer3_BINARY_DIR}/bin/release
                    ${Slicer3_BINARY_DIR}/bin/debug
                    ${Slicer3_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
                    ${Slicer3_BINARY_DIR}/bin
                    ${Slicer3_LIBRARIES_DIR}/Plugins/relwithdebinfo
                    ${Slicer3_LIBRARIES_DIR}/Plugins/release
                    ${Slicer3_LIBRARIES_DIR}/Plugins/debug
                    ${Slicer3_LIBRARIES_DIR}/Plugins/${CMAKE_BUILD_TYPE}
                    ${Slicer3_LIBRARIES_DIR}/Plugins)
  
    IF (Slicer3_SOURCE_DIR)
      SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
      SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
    ENDIF (Slicer3_SOURCE_DIR)

    # Use custom location for .bmm files for slicer batchmake modules
    SET(BatchMake_APPLICATIONS_DIRS ${EXECUTABLE_OUTPUT_PATH})

    # Copy the wrapped application for BatchMake.
    CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/EMSegmentCommandLine_GUIVersion.bmm.in"
                   "${EXECUTABLE_OUTPUT_PATH}/EMSegmentCommandLine_GUIVersion.bmm")
  
    CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmSliceExtractor.bmm.in"
                   "${EXECUTABLE_OUTPUT_PATH}/bmSliceExtractor.bmm")
    CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmGridSend.bmm.in"
                   "${EXECUTABLE_OUTPUT_PATH}/bmGridSend.bmm")
    CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/bmGridStore.bmm.in"
                   "${EXECUTABLE_OUTPUT_PATH}/bmGridStore.bmm")
  
    # Configure a header
    CONFIGURE_FILE("${EMSegmentBatchMakeModule_SOURCE_DIR}/EMSegmentBatchMakeModuleConfig.h.in"
                   "${EMSegmentBatchMakeModule_BINARY_DIR}/EMSegmentBatchMakeModuleConfig.h")
 
  ENDIF(Slicer3_BINARY_DIR)

  # Find GenerateCLP
  FIND_PACKAGE(GenerateCLP REQUIRED)
  INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

  SET ( EMSegmentBatchMakeModule_SOURCE EMSegmentBatchMakeModule.cxx )
  
  GENERATECLP(EMSegmentBatchMakeModule_SOURCE 
              EMSegmentBatchMakeModule.xml 
              ${EMSegmentBatchMakeModule_SOURCE_DIR}/logo.h)
  
  ADD_EXECUTABLE (EMSegmentBatchMakeModule ${EMSegmentBatchMakeModule_SOURCE})
  TARGET_LINK_LIBRARIES(EMSegmentBatchMakeModule BatchMakeLib)
  
ENDIF(BatchMake_DIR)
