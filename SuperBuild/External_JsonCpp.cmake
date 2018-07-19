
set( proj JsonCpp )

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

# Sanity checks
if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/jsoncpp.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "73b8e172d6615251ef851d883ef02f163e7075b2" # slicer-v0.10.6-2016-04-22
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DBUILD_TESTING:BOOL=OFF
      -DJSONCPP_WITH_TESTS:BOOL=OFF
      -DJSONCPP_WITH_POST_BUILD_UNITTEST:BOOL=OFF
      -DJSONCPP_WITH_WARNING_AS_ERROR:BOOL=OFF
      -DJSONCPP_WITH_PKGCONFIG_SUPPORT:BOOL=OFF
      -DJSONCPP_WITH_CMAKE_PACKAGE:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_STATIC_LIBS:BOOL=OFF
      -DLIBRARY_INSTALL_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DRUNTIME_INSTALL_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DARCHIVE_INSTALL_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DINCLUDE_INSTALL_DIR:PATH=${Slicer_INSTALL_INCLUDE_DIR}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${proj}_DIR ${EP_BINARY_DIR})
  set(${proj}_SOURCE_DIR ${EP_SOURCE_DIR})
  set(${proj}_INCLUDE_DIR ${EP_SOURCE_DIR}/include)
  if(WIN32)
    set(lib_prefix "")
    set(lib_ext "lib")
  elseif(APPLE)
    set(lib_prefix "lib")
    set(lib_ext "dylib")
  else()
    set(lib_prefix "lib")
    set(lib_ext "so")
  endif()
  set(${proj}_LIBRARY ${${proj}_DIR}/src/lib_json/${CMAKE_CFG_INTDIR}/${lib_prefix}jsoncpp.${lib_ext})

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${JsonCpp_DIR}/src/lib_json/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS ${proj}_INCLUDE_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
mark_as_superbuild(
  VARS ${proj}_LIBRARY:PATH
  LABELS "FIND_PACKAGE"
  )
# JsonCpp is a project built using CMake. JsconCpp_DIR ensures the install
# rules will be used to create the Slicer package. However because JsonCpp
# doesn't provide a JsonCppConfig.cmake, it is not yet associated with the
# "FIND_PACKAGE" label.
# Instead the variables JsonCpp_INCLUDE_DIR and JsonCpp_LIBRARY are
# expected to be set to ensure FindJsonCpp.cmake finds everything.
mark_as_superbuild(
  VARS ${proj}_DIR:PATH
  )
