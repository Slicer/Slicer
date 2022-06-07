
set(proj HDF5)

# Set dependency list
set(${proj}_DEPENDENCIES "zlib")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED HDF5_DIR AND NOT EXISTS ${HDF5_DIR})
  message(FATAL_ERROR "HDF5_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED HDF5_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/HDFGroup/hdf5.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "hdf5-1_12_1"
    QUIET
    )

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      # CMake settings
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
      -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      # GNUInstallDirs expands CMAKE_INSTALL_LIBDIR to 'lib' or to'lib64' depending on the OS
      -DCMAKE_INSTALL_LIBDIR:PATH=lib
      # Lib settings
      -DBUILD_TESTING:BOOL=OFF
      -DHDF5_BUILD_TOOLS:BOOL=ON
      -DHDF5_BUILD_EXAMPLES:BOOL=OFF
      -DHDF5_BUILD_HL_LIB:BOOL=ON
      -DHDF5_BUILD_CPP_LIB:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_STATIC_LIBS:BOOL=OFF
      -DHDF5_EXTERNALLY_CONFIGURED:BOOL=OFF
      -DHDF5_GENERATE_HEADERS:BOOL=ON
      -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
      -DHDF_PACKAGE_NAMESPACE:STRING=hdf5::
      -DCMAKE_POLICY_DEFAULT_CMP0074:STRING=NEW 
      # find package dirs
      -DZLIB_ROOT:PATH=${ZLIB_ROOT}
    DEPENDS 
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(HDF5_ROOT ${EP_INSTALL_DIR})
  set(HDF5_DIR ${EP_INSTALL_DIR}/share/cmake/hdf5)
  set(HDF5_INCLUDE_DIR ${EP_INSTALL_DIR}/include())
  if(WIN32)
    set(HDF5_RUNTIME_DIR ${EP_INSTALL_DIR}/bin)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/hdf5_D.lib)
    else()
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/hdf5.lib)
    endif()
  elseif(APPLE)
    set(HDF5_RUNTIME_DIR ${EP_INSTALL_DIR}/lib)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/libhdf5_debug.dylib)
    else()
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/libhdf5.dylib)
    endif()
  else()
    set(HDF5_RUNTIME_DIR ${EP_INSTALL_DIR}/lib)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/libhdf5_debug.so)
    else()
      set(HDF5_C_LIBRARY ${EP_INSTALL_DIR}/lib/libhdf5.so)
    endif()
  endif()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD 
    ${EP_INSTALL_DIR}/${_lib_subdir}
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # paths
  set(${proj}_PATHS_LAUNCHER_BUILD ${EP_INSTALL_DIR}/bin)
  mark_as_superbuild(
    VARS ${proj}_PATHS_LAUNCHER_BUILD
    LABELS "PATHS_LAUNCHER_BUILD"
    )

  # lock h5 containers to disable multiple instances of the same file
  set(${proj}_ENVVARS_LAUNCHER_BUILD
    "HDF5_USE_FILE_LOCKING=TRUE"
    )
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_BUILD
    LABELS "ENVVARS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # Set env variable
  set(${proj}_ENVVARS_LAUNCHER_INSTALLED 
    "HDF5_USE_FILE_LOCKING=FALSE"
    )

  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED 
    LABELS "ENVVARS_LAUNCHER_INSTALLED"
    )

else()
  # The project is provided using HDF5_DIR, nevertheless since other project may depend on HDF5,
  # let's add an 'empty' one
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    HDF5_ROOT:PATH
    HDF5_DIR:PATH
    HDF5_RUNTIME_DIR:PATH
    HDF5_INCLUDE_DIR:PATH
    HDF5_C_LIBRARY:FILEPATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "HDF5_ROOT: ${HDF5_ROOT}")
ExternalProject_Message(${proj} "HDF5_DIR: ${HDF5_DIR}")
ExternalProject_Message(${proj} "HDF5_RUNTIME_DIR: ${HDF5_RUNTIME_DIR}")
ExternalProject_Message(${proj} "HDF5_INCLUDE_DIR: ${HDF5_INCLUDE_DIR}")
ExternalProject_Message(${proj} "HDF5_C_LIBRARY: ${HDF5_C_LIBRARY}")
