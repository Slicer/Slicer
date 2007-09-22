PROJECT (DownloadSlicerTutorials)

#
# Disable MSVC 8 warnings
IF(WIN32)
  OPTION(DISABLE_MSVC8_DEPRECATED_WARNINGS 
         "Disable Visual Studio 8 deprecated warnings" ON)
  MARK_AS_ADVANCED(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  IF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
  ENDIF(DISABLE_MSVC8_DEPRECATED_WARNINGS)
ENDIF(WIN32)

#
# Find VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE_DIRECTORIES(${VTK_SOURCE_DIR})

#
# Find SLICERLIBCURL (slicercurl)
FIND_PACKAGE(SLICERLIBCURL REQUIRED)
INCLUDE(${SLICERLIBCURL_USE_FILE})

#
# Find GenerateCLP
FIND_PACKAGE(GenerateCLP REQUIRED)
INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

                    
#
# Set and generate source
SET(DownloadSlicerTutorials_SOURCE DownloadSlicerTutorials.cxx)

GENERATECLP(DownloadSlicerTutorials_SOURCE DownloadSlicerTutorials.xml)

#
# Generate executable
ADD_EXECUTABLE(DownloadSlicerTutorials ${DownloadSlicerTutorials_SOURCE})
TARGET_LINK_LIBRARIES(DownloadSlicerTutorials 
                      ${SLICERLIBCURL_LIBRARIES} ITKIO MRMLIDIO itkzlib)

#
# Generate shared loadable slicer module
ADD_LIBRARY(DownloadSlicerTutorialsModule SHARED
            ${DownloadSlicerTutorials_SOURCE})
SET_TARGET_PROPERTIES (DownloadSlicerTutorialsModule PROPERTIES
                       COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES(DownloadSlicerTutorialsModule 
                      ${SLICERLIBCURL_LIBRARIES} ITKIO MRMLIDIO itkzlib)

#
# Move exec and module to where Slicer can find them
IF (Slicer3_SOURCE_DIR)

  SET(TARGET_MODULES DownloadSlicerTutorials DownloadSlicerTutorialsModule)

  FOREACH(target ${TARGET_MODULES})
    INSTALL(TARGETS ${target}
            RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
            LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
