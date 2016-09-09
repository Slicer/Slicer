
set( proj JsonCpp )

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

# Sanity checks
if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  if (NOT WIN32)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      )
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/open-source-parsers/jsoncpp.git"
    GIT_TAG "0.10.6"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
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
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(${proj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(${proj}_INCLUDE_DIR ${${proj}_SOURCE_DIR}/include)
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
