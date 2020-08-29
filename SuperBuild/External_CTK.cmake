
set(proj CTK)

# Set dependency list
set(${proj}_DEPENDENCIES ${VTK_EXTERNAL_NAME} ${ITK_EXTERNAL_NAME})
if(Slicer_USE_PYTHONQT)
  list(APPEND ${proj}_DEPENDENCIES python)
endif()
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND ${proj}_DEPENDENCIES DCMTK)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
  unset(CTK_DIR CACHE)
  find_package(CTK 0.1.0 REQUIRED NO_MODULE)
endif()

if(NOT DEFINED CTK_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

    set(_wrap_qtwebkit 0)

  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
      )
  endif()

  if(Slicer_BUILD_DICOM_SUPPORT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DCTK_USE_SYSTEM_DCMTK:BOOL=${CTK_USE_SYSTEM_DCMTK}
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      )
  endif()

    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DQt5_DIR:FILEPATH=${Qt5_DIR}
      -DCTK_QT_VERSION:STRING=5
      )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/commontk/CTK.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "92d2191b15894f42aa7446970faa974652410536"
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
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
      -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
      -DBUILD_TESTING:BOOL=OFF
      -DCTK_BUILD_QTDESIGNER_PLUGINS:BOOL=${Slicer_BUILD_QT_DESIGNER_PLUGINS}
      -DCTK_INSTALL_BIN_DIR:STRING=${Slicer_INSTALL_BIN_DIR}
      -DCTK_INSTALL_LIB_DIR:STRING=${Slicer_INSTALL_LIB_DIR}
      -DCTK_INSTALL_QTPLUGIN_DIR:STRING=${Slicer_INSTALL_QtPlugins_DIR}
      -DCTK_USE_GIT_PROTOCOL:BOOL=${Slicer_USE_GIT_PROTOCOL}
      -DCTK_USE_SYSTEM_VTK:BOOL=${CTK_USE_SYSTEM_VTK}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DCTK_USE_SYSTEM_ITK:BOOL=${CTK_USE_SYSTEM_ITK}
      -DITK_DIR:PATH=${ITK_DIR}
      -DCTK_LIB_Widgets:BOOL=ON
      -DCTK_LIB_Visualization/VTK/Widgets:BOOL=ON
      -DCTK_LIB_Visualization/VTK/Widgets_USE_TRANSFER_FUNCTION_CHARTS:BOOL=ON
      -DCTK_LIB_ImageProcessing/ITK/Core:BOOL=ON
      -DCTK_LIB_PluginFramework:BOOL=OFF
      -DCTK_PLUGIN_org.commontk.eventbus:BOOL=OFF
      -DCTK_APP_ctkDICOM:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      -DCTK_LIB_DICOM/Core:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      -DCTK_LIB_DICOM/Widgets:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      -DCTK_USE_QTTESTING:BOOL=${Slicer_USE_QtTesting}
      -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
      # PythonQt wrapping
      -DCTK_LIB_Scripting/Python/Core:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_USE_VTK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTCORE:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTGUI:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTUITOOLS:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTNETWORK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTWEBKIT:BOOL=${_wrap_qtwebkit}
      -DCTK_LIB_Scripting/Python/Widgets:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_ENABLE_Python_Wrapping:BOOL=${Slicer_USE_PYTHONQT}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  if(Slicer_USE_PYTHONQT)
    if(APPLE)
      ExternalProject_Add_Step(${proj} fix_pythonqt_rpath
        COMMAND install_name_tool
          -id ${EP_BINARY_DIR}/CMakeExternals/Install/lib/libPythonQt.dylib
          ${EP_BINARY_DIR}/CMakeExternals/Install/lib/libPythonQt.dylib
        DEPENDEES build
        )
    endif()
  endif()

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(CTK_DIR ${EP_BINARY_DIR})

  if(NOT DEFINED CTK_VALGRIND_SUPPRESSIONS_FILE)
    set(CTK_VALGRIND_SUPPRESSIONS_FILE ${EP_SOURCE_DIR}/CMake/CTKValgrind.supp)
  endif()
  mark_as_superbuild(CTK_VALGRIND_SUPPRESSIONS_FILE:FILEPATH)

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${CTK_DIR}/CTK-build/bin/<CMAKE_CFG_INTDIR>)
  if(Slicer_USE_QtTesting)
    list(APPEND ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
      ${CTK_DIR}/QtTesting-build/<CMAKE_CFG_INTDIR>
      )
  endif()
  if(Slicer_USE_PYTHONQT)
    list(APPEND ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
      ${CTK_DIR}/PythonQt-build/<CMAKE_CFG_INTDIR>
      )
  endif()
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${CTK_DIR}/CTK-build/bin/Python
    ${CTK_DIR}/CTK-build/bin/<CMAKE_CFG_INTDIR>
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to installed tree

  if(UNIX AND NOT APPLE)
    # On windows, pythonQt libraries are installed along with the executable
    set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED <APPLAUNCHER_SETTINGS_DIR>/../lib/PythonQt)
    mark_as_superbuild(
      VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
      LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
      )
  endif()

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS CTK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
