
# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
endif()

include(SlicerMacroGetCompilerName)
SlicerMacroGetCompilerName(EXTENSION_COMPILER)

set(EXTENSION_BITNESS 32)
set(EXTENSION_ARCHITECTURE "i386")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(EXTENSION_ARCHITECTURE "amd64")
  set(EXTENSION_BITNESS 64)
endif()

if(WIN32)
  set(EXTENSION_OPERATING_SYSTEM "win")
elseif(APPLE)
  set(EXTENSION_OPERATING_SYSTEM "macosx")
elseif(UNIX)
  set(EXTENSION_OPERATING_SYSTEM "linux")
endif()
