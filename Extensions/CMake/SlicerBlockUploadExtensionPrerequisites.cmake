
# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
endif()

# Get EXTENSION_BITNESS, EXTENSION_COMPILER and Slicer_WC_REVISION
include(SlicerMacroGetCompilerName)
SlicerMacroGetCompilerName(EXTENSION_COMPILER)
set(EXTENSION_BITNESS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(EXTENSION_BITNESS 64)
endif()

