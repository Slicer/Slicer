PROJECT (RegisterImagesBatchMakeModule)

# Set the name of the Slicer3 execution module to be run using BatchMake
SET(Slicer3ExeModule RegisterImages)

# Set the name of the Slicer3 BatchMake module to be created
SET(Slicer3BatchModule RegisterImagesBatchMakeModule)

# Set the source code for the Slicer3 BatchMake module
SET(Slicer3BatchModule_SOURCE RegisterImagesBatchMakeModule.cxx)

# Set the logo for the module, or comment out the following line
SET(Slicer3BatchModule_LOGO logo.h)

# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS 
         "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

FIND_PACKAGE(BatchMake REQUIRED)
INCLUDE(${BatchMake_USE_FILE})

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

    FIND_PROGRAM(${Slicer3ExeModule}
              NAMES ${Slicer3ExeModule}.exe ${Slicer3ExeModule}
              PATHS ${Slicer3_BINARY_DIR}/bin/relwithdebinfo
                    ${Slicer3_BINARY_DIR}/bin/release
                    ${Slicer3_BINARY_DIR}/bin/debug
                    ${Slicer3_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
                    ${Slicer3_BINARY_DIR}/bin
                    ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/relwithdebinfo
                    ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/release
                    ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/debug
                    ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins/${CMAKE_BUILD_TYPE}
                    ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
                    ${SLICER_LIBRARIES_DIR}/Plugins/relwithdebinfo
                    ${SLICER_LIBRARIES_DIR}/Plugins/release
                    ${SLICER_LIBRARIES_DIR}/Plugins/debug
                    ${SLICER_LIBRARIES_DIR}/Plugins/${CMAKE_BUILD_TYPE}
                    ${SLICER_LIBRARIES_DIR}/Plugins)
    
    IF (Slicer3_SOURCE_DIR)
      SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
      SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
    ENDIF (Slicer3_SOURCE_DIR)

    SET(BatchMake_WRAPPED_APPLICATION_DIR 
      ${EXECUTABLE_OUTPUT_PATH}/BatchMakeApplications)

    # Copy the wrapped application for BatchMake.
    CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3ExeModule}.bmm.in"
                    "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3ExeModule}.bmm")

    CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/bmSliceExtractor.bmm.in"
                    "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm")
    CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridSend.bmm.in"
                    "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm")
    CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridStore.bmm.in"
                    "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm")

    # Configure a header
    CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule}Config.h.in"
                    "${CMAKE_CURRENT_BINARY_DIR}/${Slicer3BatchModule}Config.h")

  ENDIF(Slicer3_BINARY_DIR)
 
  # Find GenerateCLP
  FIND_PACKAGE(GenerateCLP REQUIRED)
  INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

  IF(${Slicer3BatchModule_LOGO})
    GENERATECLP(Slicer3BatchModule_SOURCE 
                ${Slicer3BatchModule}.xml 
                ${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule_LOGO})
  ELSE(${Slicer3BatchModule_LOGO})
    GENERATECLP(Slicer3BatchModule_SOURCE 
                ${Slicer3BatchModule}.xml)
  ENDIF(${Slicer3BatchModule_LOGO})

  ADD_EXECUTABLE(${Slicer3BatchModule} ${Slicer3BatchModule_SOURCE})
  TARGET_LINK_LIBRARIES(${Slicer3BatchModule} BatchMakeLib BatchMakeUtilities ITKIO ITKCommon
ITKStatistics ITKNumerics)

  IF (Slicer3_SOURCE_DIR)
    # install each target in the production area (where it would appear in an 
    # installation) and install each target in the developer area (for running 
    # from a build)
    SET(TARGETS ${Slicer3BatchModule})
    FOREACH(target ${TARGETS})
      INSTALL(TARGETS ${target}
        RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
        LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
    ENDFOREACH(target)

    SET(TARGETS ${Slicer3BatchModule})
    FOREACH(target ${TARGETS})
      INSTALL(TARGETS ${target}
        RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
        LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
    ENDFOREACH(target)

  ENDIF(Slicer3_SOURCE_DIR)

ENDIF(BatchMake_DIR)
 
