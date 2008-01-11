PROJECT ( RegisterImagesModule )

# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS
         "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

SET ( RegisterImages_SOURCE 
      RegisterImages.cxx
    )

GENERATECLP(RegisterImages_SOURCE RegisterImages.xml)

ADD_EXECUTABLE(RegisterImages ${RegisterImages_SOURCE})
TARGET_LINK_LIBRARIES(RegisterImages ITKIO ITKCommon ITKStatistics ITKNumerics)

ADD_LIBRARY(RegisterImagesModule SHARED ${RegisterImages_SOURCE})
SET_TARGET_PROPERTIES (RegisterImagesModule PROPERTIES
                       COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES(RegisterImagesModule ITKIO ITKCommon ITKStatistics
ITKNumerics)

IF (Slicer3_SOURCE_DIR)

  SET(TARGET_MODULES RegisterImagesModule RegisterImages)

  FOREACH(target ${TARGET_MODULES})
    INSTALL(TARGETS ${target}
            RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
            LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
