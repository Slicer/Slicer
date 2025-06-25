
set( proj RapidJSON )

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(${proj}_DIR CACHE)
  find_package(${proj} REQUIRED)
endif()

# Sanity checks
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/slicer/rapidjson.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "4da93e4b581f88fecc460e2259f468faaa438554" # slicer-2024-12-22-24b5e7a8
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  if(CMAKE_CXX_STANDARD STREQUAL "17")
    set(_build_cxx17 ON)
    set(_build_cxx20 OFF)
  elseif(CMAKE_CXX_STANDARD STREQUAL "20")
    set(_build_cxx17 OFF)
    set(_build_cxx20 ON)
  else()
    message(FATAL_ERROR "CMAKE_CXX_STANDARD must be set to 17 or 20")
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      # Not used -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      # Not used -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      # Specifying these options conflicts with the use of the RAPIDJSON_BUILD_CXX
      # options.
      # -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      # -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      # -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      # Options
      -DRAPIDJSON_BUILD_CXX11:BOOL=OFF
      -DRAPIDJSON_BUILD_CXX17:BOOL=${_build_cxx17}
      -DRAPIDJSON_BUILD_CXX20:BOOL=${_build_cxx20}
      -DRAPIDJSON_BUILD_DOC:BOOL=OFF
      -DRAPIDJSON_BUILD_EXAMPLES:BOOL=OFF
      -DRAPIDJSON_BUILD_TESTS:BOOL=OFF
      -DRAPIDJSON_ENABLE_INSTRUMENTATION_OPT:BOOL=OFF
      # Install directories
      -DCMAKE_INSTALL_PREFIX:PATH=${EP_INSTALL_DIR}

      # Specify CMAKE_INSTALL_DIR to ensure CMake configuration files
      # (RapidJSONConfig.cmake, RapidJSONConfigVersion.cmake, RapidJSON-targets.cmake)
      # are generated in a consistent location across platforms (Linux, macOS, Windows).
      # This variable is specific to RapidJSON's build system.
      -DCMAKE_INSTALL_DIR:PATH=${EP_INSTALL_DIR}/lib/cmake/RapidJSON

      # Install the project to support importing the associated "header-only"
      # CMake target via find_package().
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(${proj}_DIR "${EP_INSTALL_DIR}/lib/cmake/RapidJSON/")

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    ${proj}_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
