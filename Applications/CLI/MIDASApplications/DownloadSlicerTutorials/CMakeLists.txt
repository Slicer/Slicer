project (DownloadSlicerTutorials)

#
# Disable MSVC 8 warnings
if(WIN32)
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS 
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif(DISABLE_MSVC8_DEPRECATED_WARNINGS)
endif(WIN32)

#
# Find VTK
find_package(VTK REQUIRED)
include_directories(${VTK_SOURCE_DIR})

#
# Find SLICERLIBCURL (slicercurl)
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

#
# Find GenerateCLP
find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

#
# Set and generate source
set(DownloadSlicerTutorials_SOURCE DownloadSlicerTutorials.cxx)

generateclp(DownloadSlicerTutorials_SOURCE DownloadSlicerTutorials.xml)

#
# Generate executable
add_executable(DownloadSlicerTutorials ${DownloadSlicerTutorials_SOURCE})
target_link_libraries(DownloadSlicerTutorials 
  ${SLICERLIBCURL_LIBRARIES} ITKIO MRMLIDIO itkzlib)

#
# Generate shared loadable slicer module
add_library(DownloadSlicerTutorialsModule SHARED
  ${DownloadSlicerTutorials_SOURCE})
set_target_properties (DownloadSlicerTutorialsModule PROPERTIES
  COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(DownloadSlicerTutorialsModule 
  ${SLICERLIBCURL_LIBRARIES} ITKIO MRMLIDIO itkzlib)

#
# Move exec and module to where Slicer can find them
if (Slicer3_SOURCE_DIR)

  set(TARGET_MODULES DownloadSlicerTutorials DownloadSlicerTutorialsModule)

  foreach(target ${TARGET_MODULES})
    install(TARGETS ${target}
      RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
      )
  endforeach(target)

endif (Slicer3_SOURCE_DIR)
