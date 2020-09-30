
set(proj SimpleITK)

# Set dependency list
set(${proj}_DEPENDENCIES ITK Swig python python-setuptools)

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED SimpleITK_DIR AND NOT EXISTS ${SimpleITK_DIR})
  message(FATAL_ERROR "SimpleITK_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

  include(ExternalProjectForNonCMakeProject)

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  # Variables used to update PATH, LD_LIBRARY_PATH or DYLD_LIBRARY_PATH in env. script below
  if(WIN32)
    set(_varname "PATH")
    set(_path_sep ";")
  elseif(UNIX)
    set(_path_sep ":")
    if(APPLE)
      set(_varname "DYLD_LIBRARY_PATH")
    else()
      set(_varname "LD_LIBRARY_PATH")
    endif()
  endif()

  # Used below to both set the env. script and the launcher settings
  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()

  set(_paths)

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      list(APPEND _paths
        # ITK
        ${ITK_DIR}/${_lib_subdir}/${config}
        )
    endforeach()
  else()
    list(APPEND _paths
      # ITK
      ${ITK_DIR}/${_lib_subdir}
      )
  endif()

  if(Slicer_USE_SimpleITK_SHARED)
    if(CMAKE_CONFIGURATION_TYPES)
      foreach(config ${CMAKE_CONFIGURATION_TYPES})
        list(APPEND _paths
          # SimpleITK
          ${EP_BINARY_DIR}/SimpleITK-build/${_lib_subdir}/${config}
          # DCMTK
          ${DCMTK_DIR}/${_lib_subdir}/${config}
          )
      endforeach()
    else()
      list(APPEND _paths
        # SimpleITK
        ${EP_BINARY_DIR}/SimpleITK-build/${_lib_subdir}
        # DCMTK
        ${DCMTK_DIR}/${_lib_subdir}
        )
    endif()

    # TBB
    if(Slicer_USE_TBB)
      list(APPEND _paths ${TBB_BIN_DIR})
    endif()
  endif()

  list(JOIN _paths "${_path_sep}" _paths)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetPythonSetupEnv_Commands(${_env_script})
  file(APPEND ${_env_script}
"#------------------------------------------------------------------------------
# Added by '${CMAKE_CURRENT_LIST_FILE}'
set(ENV{${_varname}} \"${_paths}${_path_sep}\$ENV{${_varname}}\")
")

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  set(SimpleITK_ADDITONAL_CMAKE_CACHE_ARGS)
  if(APPLE)
    # To ensure dynamic_cast for ITK symbols works across libraries,
    # and with the Slicer runtime, make all implicit symbols visible
    # on OSX.
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS  "-DCMAKE_CXX_VISIBILITY_PRESET:BOOL=default")
  endif()

  # install step - the working path must be set to the location of the SimpleITK.py
  # file so that it will be picked up by distuils setup, and installed
  set(_install_script ${CMAKE_BINARY_DIR}/${proj}_install_step.cmake)
  file(WRITE ${_install_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${CMAKE_BINARY_DIR}/${proj}-build/SimpleITK-build/Wrapping/Python\")
ExternalProject_Execute(${proj} \"install\" \"${PYTHON_EXECUTABLE}\" setup.py install)
")

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/SimpleITK/SimpleITK.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "v2.0.0"  # Paired with a specific SimpleITKFilters remote module git tag in the ITK project
    QUIET
    )

  # A separate project is used to download, so that the SuperBuild
  # subdirectory can be use for SimpleITK's SuperBuild to build
  # required Lua, GTest etc. dependencies not in Slicer SuperBuild
  ExternalProject_add(SimpleITK-download
    SOURCE_DIR ${EP_SOURCE_DIR}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ""
    )

  ExternalProject_GenerateProjectDescription_Step(SimpleITK-download
    SOURCE_DIR ${EP_SOURCE_DIR}
    NAME ${proj}
    )

  ExternalProject_add(SimpleITK
    ${${proj}_EP_ARGS}
    SOURCE_DIR ${EP_SOURCE_DIR}/SuperBuild
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DCMAKE_CXX_VISIBILITY_PRESET:BOOL=default
      -DBUILD_SHARED_LIBS:BOOL=${Slicer_USE_SimpleITK_SHARED}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DSimpleITK_BUILD_STRIP:BOOL=1
      -DSimpleITK_PYTHON_THREADS:BOOL=ON
      -DSimpleITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSimpleITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSimpleITK_INT64_PIXELIDS:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      -DSimpleITK_USE_SYSTEM_ITK:BOOL=ON
      -DITK_DIR:PATH=${ITK_DIR}
      -DSimpleITK_USE_SYSTEM_SWIG:BOOL=ON
      -DSWIG_EXECUTABLE:PATH=${SWIG_EXECUTABLE}
      -DPYTHON_EXECUTABLE:PATH=${PYTHON_EXECUTABLE}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_DOXYGEN:BOOL=OFF
      -DWRAP_DEFAULT:BOOL=OFF
      -DWRAP_PYTHON:BOOL=ON
      -DSimpleITK_BUILD_DISTRIBUTE:BOOL=ON # Shorten version and install path removing -g{GIT-HASH} suffix.
      -DExternalData_OBJECT_STORES:PATH=${ExternalData_OBJECT_STORES}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    #
    INSTALL_COMMAND ${CMAKE_COMMAND} -P ${_install_script}
    #
    DEPENDS SimpleITK-download ${${proj}_DEPENDENCIES}
    )
  set(SimpleITK_DIR ${CMAKE_BINARY_DIR}/SimpleITK-build/SimpleITK-build)

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${SimpleITK_DIR}/${_lib_subdir}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS SimpleITK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
