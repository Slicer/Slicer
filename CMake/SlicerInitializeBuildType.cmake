
# Default build type to use if none was specified
if(NOT DEFINED Slicer_DEFAULT_BUILD_TYPE)
  set(Slicer_DEFAULT_BUILD_TYPE "Debug")
endif()

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

  message(STATUS "Setting build type to '${Slicer_DEFAULT_BUILD_TYPE}' as none was specified.")

  set(CMAKE_BUILD_TYPE ${Slicer_DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build." FORCE)
  mark_as_advanced(CMAKE_BUILD_TYPE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug"
    "Release"
    "MinSizeRel"
    "RelWithDebInfo"
    )
endif()

# Pass variables to dependent projects
if(COMMAND mark_as_superbuild)
  if(NOT CMAKE_CONFIGURATION_TYPES)
    mark_as_superbuild(VARS CMAKE_BUILD_TYPE ALL_PROJECTS)
  else()
    mark_as_superbuild(VARS CMAKE_CONFIGURATION_TYPES ALL_PROJECTS)
  endif()
endif()
