set(proj vtkAddon)

# Set dependency list
set(${proj}_DEPENDENCIES VTK)
if (Slicer_USE_PYTHONQT)
  list(APPEND ${proj}_DEPENDENCIES python)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(NOT DEFINED vtkAddon_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/vtkAddon.git"
    QUIET
  )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "eece7e7c015085ad1283891bad77299e6673b89e"
    QUIET
  )

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      "-DvtkAddon_WRAP_PYTHON:BOOL=ON"
    )
  endif()

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
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_TESTING:BOOL=OFF
      -DvtkAddon_USE_UTF8:BOOL=ON
      -DvtkAddon_CMAKE_DIR:PATH=${EP_SOURCE_DIR}/CMake
      "-DvtkAddon_LAUNCH_COMMAND:STRING=${Slicer_LAUNCH_COMMAND}"
      -DvtkAddon_INSTALL_DEVELOPMENT:BOOL=ON
      -DVTK_WRAP_PYTHON_FIND_LIBS:BOOL=OFF
      -DVTK_DIR:PATH=${VTK_DIR}
      -DVTK_UNDEFINED_SYMBOLS_ALLOWED:BOOL=ON
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
  )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(vtkAddon_DIR "${EP_BINARY_DIR}")

  # Add path to SlicerLauncherSettings.ini
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${EP_BINARY_DIR}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
  )
  # Add pythonpath to SlicerLauncherSettings.ini
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD ${EP_BINARY_DIR}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
  )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # pythonpath
  if(UNIX)
    set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
      <APPLAUNCHER_SETTINGS_DIR>/../lib/python${Slicer_REQUIRED_PYTHON_VERSION_DOT}/vtkAddon
      )
  else()
    set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
      <APPLAUNCHER_SETTINGS_DIR>/../lib/vtkAddon
      )
  endif()
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_INSTALLED
    LABELS "PYTHONPATH_LAUNCHER_INSTALLED"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS vtkAddon_DIR:PATH
  LABELS "FIND_PACKAGE"
)
