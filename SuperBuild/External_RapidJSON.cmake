
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
    #"${EP_GIT_PROTOCOL}://github.com/miloyip/rapidjson.git" #2023.05.09
    "${EP_GIT_PROTOCOL}://github.com/Tencent/rapidjson.git"  #2025.06.19
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    # "973dc9c06dcd3d035ebd039cfb9ea457721ec213" # 2023.05.09
    "24b5e7a8b27f42fa16b96fc70aade9106cf7102f"   # 2025.06.19
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  #RapidJSON requires full paths defined
  set(${proj}_DIR "${CMAKE_BINARY_DIR}/${proj}-install")
  set(${proj}_LIB_DIR "${${proj}_DIR}/lib")
  set(${proj}_INCLUDE_DIR "${${proj}_DIR}/include")

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
      -DRAPIDJSON_BUILD_DOC:BOOL=OFF
      -DRAPIDJSON_BUILD_EXAMPLES:BOOL=OFF
      -DRAPIDJSON_BUILD_TESTS:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=${${proj}_DIR} # RapidJSON requires full paths defined
      -DLIBRARY_INSTALL_DIR:PATH=${${proj}_LIB_DIR}
      -DRUNTIME_INSTALL_DIR:PATH=${${proj}_LIB_DIR}
      -DARCHIVE_INSTALL_DIR:PATH=${${proj}_LIB_DIR}
      -DINCLUDE_INSTALL_DIR:PATH=${${proj}_INCLUDE_DIR}
    #INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(${proj}_SOURCE_DIR ${EP_SOURCE_DIR})
  set(${proj}_INCLUDE_DIR ${${proj}_SOURCE_DIR}/include)

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    ${proj}_INCLUDE_DIR:PATH
    ${proj}_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
