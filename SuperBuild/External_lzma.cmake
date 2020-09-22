
set( proj lzma )

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(LZMA_INCLUDE_DIR CACHE)
  unset(LZMA_LIBRARY CACHE)
  find_path(LZMA_INCLUDE_PATH lzma.h)
  find_library(LZMA_LIBRARY lzma)
endif()

# Sanity checks
if(DEFINED LZMA_INCLUDE_DIR AND NOT EXISTS ${LZMA_INCLUDE_DIR})
  message(FATAL_ERROR "LZMA_INCLUDE_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED LZMA_LIBRARY AND NOT EXISTS ${LZMA_LIBRARY})
  message(FATAL_ERROR "LZMA_LIBRARY variable is defined but corresponds to nonexistent directory")
endif()

if((NOT DEFINED LZMA_INCLUDE_DIR
   OR NOT DEFINED LZMA_LIBRARY) AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/lib_lzma.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "v5.2.4"
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)
  set(EP_INSTALL_LIBDIR "lib")

  set(${proj}_CMAKE_C_FLAGS ${ep_common_c_flags})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
    set(${proj}_CMAKE_C_FLAGS "${ep_common_c_flags} -fPIC")
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${${proj}_CMAKE_C_FLAGS}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DCMAKE_INSTALL_LIBDIR:PATH=${EP_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_PREFIX:PATH=${EP_INSTALL_DIR}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(${proj}_INCLUDE_DIR ${EP_INSTALL_DIR}/include)
  if(WIN32)
    set(${proj}_LIBRARY ${EP_INSTALL_DIR}/lib/liblzma.lib)
  else()
    set(${proj}_LIBRARY ${EP_INSTALL_DIR}/lib/libliblzma.a)
  endif()

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    LZMA_INCLUDE_DIR:PATH
    LZMA_LIBRARY:FILEPATH
  LABELS "FIND_PACKAGE"
  )

if(Slicer_USE_SYSTEM_${proj})
  ExternalProject_Message(${proj} "LZMA_INCLUDE_DIR:${LibArchive_INCLUDE_DIR}")
  ExternalProject_Message(${proj} "LZMA_LIBRARY:${LibArchive_LIBRARY}")
endif()
