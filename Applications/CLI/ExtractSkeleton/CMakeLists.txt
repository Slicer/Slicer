project (ExtractSkeleton)

# Disable MSVC 8 warnings
if(WIN32)
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif(DISABLE_MSVC8_DEPRECATED_WARNINGS)
endif(WIN32)

set ( ExtractSkeleton_SOURCE 
  ExtractSkeleton.cxx
  SkelGraph.h
  SkelGraph.cxx
  tilg_iso_3D.h
  tilg_iso_3D.cxx
  coordTypes.h
  misc.h
  misc.cxx )

generateclp(ExtractSkeleton_SOURCE ExtractSkeleton.xml)

add_library(ExtractSkeletonModule SHARED ${ExtractSkeleton_SOURCE})
slicer3_set_plugins_output_path(ExtractSkeletonModule)
set_target_properties (ExtractSkeletonModule PROPERTIES
  COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries(ExtractSkeletonModule ITKIO)

set(CLP ExtractSkeleton)
add_executable(${CLP} ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
slicer3_set_plugins_output_path(${CLP})
target_link_libraries (${CLP} ${CLP}Module)

if (Slicer_SOURCE_DIR)

  set(TARGETS ExtractSkeletonModule ExtractSkeleton)
  slicer3_install_plugins(${TARGETS})

endif (Slicer_SOURCE_DIR)
