PROJECT (ExtractSkeleton)

# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS
         "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

SET ( ExtractSkeleton_SOURCE 
      ExtractSkeleton.cxx
      SkelGraph.h
      SkelGraph.cxx
      tilg_iso_3D.h
      tilg_iso_3D.cxx
      coordTypes.h
      misc.h
      misc.cxx )

GENERATECLP(ExtractSkeleton_SOURCE ExtractSkeleton.xml)

ADD_EXECUTABLE(ExtractSkeleton ${ExtractSkeleton_SOURCE})
TARGET_LINK_LIBRARIES(ExtractSkeleton ITKIO)

ADD_LIBRARY(ExtractSkeletonModule SHARED ${ExtractSkeleton_SOURCE})
SET_TARGET_PROPERTIES (ExtractSkeletonModule PROPERTIES
                       COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES(ExtractSkeletonModule ITKIO)

IF (Slicer3_SOURCE_DIR)

  SET(TARGET_MODULES ExtractSkeletonModule ExtractSkeleton)

  FOREACH(target ${TARGET_MODULES})
    INSTALL(TARGETS ${target}
            RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
            LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
