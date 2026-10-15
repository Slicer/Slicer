# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(Slicer_EXTENSION_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
endif()

include(SlicerMacroGetCompilerName)
SlicerMacroGetCompilerName(EXTENSION_COMPILER)

include(SlicerMacroGetOperatingSystemArchitectureBitness)
SlicerMacroGetOperatingSystemArchitectureBitness(VAR_PREFIX EXTENSION)

set(EXTENSION_OPERATING_SYSTEM "${EXTENSION_OS}")
