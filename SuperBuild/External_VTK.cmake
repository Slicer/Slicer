
set(proj VTK)

# Set dependency list
set(${proj}_DEPENDENCIES "zlib")
if (Slicer_USE_PYTHONQT)
  list(APPEND ${proj}_DEPENDENCIES python)
endif()
if(Slicer_USE_TBB)
  list(APPEND ${proj}_DEPENDENCIES tbb)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(VTK_DIR CACHE)
  unset(VTK_SOURCE_DIR CACHE)
  find_package(VTK REQUIRED NO_MODULE)
endif()

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(DEFINED VTK_SOURCE_DIR AND NOT EXISTS ${VTK_SOURCE_DIR})
  message(FATAL_ERROR "VTK_SOURCE_DIR variable is defined but corresponds to nonexistent directory")
endif()


if((NOT DEFINED VTK_DIR OR NOT DEFINED VTK_SOURCE_DIR) AND NOT Slicer_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  set(VTK_WRAP_PYTHON OFF)

  if(Slicer_USE_PYTHONQT)
    set(VTK_WRAP_PYTHON ON)
  endif()

  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DPYTHON_EXECUTABLE:PATH=${PYTHON_EXECUTABLE}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      )
  endif()

  # Markups module needs vtkFrenetSerretFrame, which is available in
  # SplineDrivenImageSlicer remote module.
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DModule_SplineDrivenImageSlicer:BOOL=ON
    )

  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DVTK_USE_GUISUPPORT:BOOL=ON
    -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
    -DModule_vtkTestingRendering:BOOL=ON
    )
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DVTK_QT_VERSION:STRING=5
      -DVTK_Group_Qt:BOOL=ON
      -DQt5_DIR:FILEPATH=${Qt5_DIR}
      )
  if("${Slicer_VTK_RENDERING_BACKEND}" STREQUAL "OpenGL2")
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DModule_vtkGUISupportQtOpenGL:BOOL=ON
    )
  endif()
  if(Slicer_USE_TBB)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DTBB_INCLUDE_DIR:PATH=${TBB_INCLUDE_DIR}
      -DTBB_LIBRARY_DEBUG:FILEPATH=${TBB_LIBRARY_DEBUG}
      -DTBB_LIBRARY_RELEASE:FILEPATH=${TBB_LIBRARY_RELEASE}
      -DTBB_MALLOC_INCLUDE_DIR:PATH=${TBB_MALLOC_INCLUDE_DIR}
      -DTBB_MALLOC_LIBRARY_DEBUG:FILEPATH=${TBB_MALLOC_LIBRARY_DEBUG}
      -DTBB_MALLOC_LIBRARY_RELEASE:FILEPATH=${TBB_MALLOC_LIBRARY_RELEASE}
      -DTBB_MALLOC_PROXY_INCLUDE_DIR:PATH=${TBB_MALLOC_PROXY_INCLUDE_DIR}
      -DTBB_MALLOC_PROXY_LIBRARY_DEBUG:FILEPATH=${TBB_MALLOC_PROXY_LIBRARY_DEBUG}
      -DTBB_MALLOC_PROXY_LIBRARY_RELEASE:FILEPATH=${TBB_MALLOC_PROXY_LIBRARY_RELEASE}
      )
  endif()
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DVTK_USE_CARBON:BOOL=OFF
      -DVTK_USE_COCOA:BOOL=ON # Default to Cocoa, VTK/CMakeLists.txt will enable Carbon and disable cocoa if needed
      -DVTK_USE_X:BOOL=OFF
      -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
      #-DVTK_USE_RPATH:BOOL=ON # Unused
      )
  endif()
  if(UNIX AND NOT APPLE)
    find_package(FontConfig QUIET)
    if(FONTCONFIG_FOUND)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
        -DModule_vtkRenderingFreeTypeFontConfig:BOOL=ON
        )
    endif()

    # OpenGL_GL_PREFERENCE
    if(NOT "${OpenGL_GL_PREFERENCE}" MATCHES "^(LEGACY|GLVND)$")
      message(FATAL_ERROR "OpenGL_GL_PREFERENCE variable is expected to be set to LEGACY or GLVND")
    endif()
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DOpenGL_GL_PREFERENCE:STRING=${OpenGL_GL_PREFERENCE}
      )
  endif()

  # Disable Tk when Python wrapping is enabled
  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DVTK_USE_TK:BOOL=OFF
      )
  endif()

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/slicer/VTK.git"
    QUIET
    )

set(_git_tag)
if("${Slicer_VTK_VERSION_MAJOR}" STREQUAL "8")
  set(_git_tag "97904fdcc7e73446b3131f32eac9fc9781b23c2d") # slicer-v8.2.0-2018-10-02-74d9488523
else()
  message(FATAL_ERROR "error: Unsupported Slicer_VTK_VERSION_MAJOR: ${Slicer_VTK_VERSION_MAJOR}")
endif()
  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    ${_git_tag}
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
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DVTK_USE_PARALLEL:BOOL=ON
      -DVTK_DEBUG_LEAKS:BOOL=${VTK_DEBUG_LEAKS}
      -DVTK_LEGACY_REMOVE:BOOL=ON
      -DVTK_WRAP_TCL:BOOL=OFF
      #-DVTK_USE_RPATH:BOOL=ON # Unused
      -DVTK_WRAP_PYTHON:BOOL=${VTK_WRAP_PYTHON}
      -DVTK_INSTALL_RUNTIME_DIR:PATH=${Slicer_INSTALL_BIN_DIR}
      -DVTK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DVTK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DVTK_Group_Qt:BOOL=ON
      -DVTK_USE_SYSTEM_ZLIB:BOOL=ON
      -DZLIB_ROOT:PATH=${ZLIB_ROOT}
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      -DVTK_ENABLE_KITS:BOOL=ON
      -DVTK_RENDERING_BACKEND:STRING=${Slicer_VTK_RENDERING_BACKEND}
      -DVTK_SMP_IMPLEMENTATION_TYPE:STRING=${Slicer_VTK_SMP_IMPLEMENTATION_TYPE}
      -DModule_vtkAcceleratorsVTKm:BOOL=OFF
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  if(Slicer_USE_PYTHONQT AND NOT Slicer_USE_SYSTEM_python)
    # Create the vtk-*.egg-info directory to prevent pip from re-installing
    # vtk package as a wheel when listed as dependency in Slicer extension.
    set(_vtk_egg_info_dir "${python_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/vtk-8.2.0-py3.6.egg-info")
    ExternalProject_Add_Step(${proj} create_egg_info
      COMMAND ${CMAKE_COMMAND} -E make_directory ${_vtk_egg_info_dir}
      COMMAND ${CMAKE_COMMAND} -E touch ${_vtk_egg_info_dir}/PKG-INFO
      COMMENT "Creating '${_vtk_egg_info_dir}' directory"
      DEPENDEES build
      )
  endif()

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(VTK_DIR ${EP_BINARY_DIR})
  set(VTK_SOURCE_DIR ${EP_SOURCE_DIR})

  if(NOT DEFINED VTK_VALGRIND_SUPPRESSIONS_FILE)
    set(VTK_VALGRIND_SUPPRESSIONS_FILE ${EP_SOURCE_DIR}/CMake/VTKValgrindSuppressions.supp)
  endif()
  mark_as_superbuild(VTK_VALGRIND_SUPPRESSIONS_FILE:FILEPATH)

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(_library_output_subdir bin)
  if(UNIX)
    set(_library_output_subdir lib)
  endif()
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${VTK_DIR}/${_library_output_subdir}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # pythonpath
    if(UNIX)
      set(${proj}_PYTHONPATH_LAUNCHER_BUILD
        ${VTK_DIR}/${_library_output_subdir}/python3.6/site-packages
        )
    else()
      set(${proj}_PYTHONPATH_LAUNCHER_BUILD
        ${VTK_DIR}/${_library_output_subdir}/<CMAKE_CFG_INTDIR>/Lib/site-packages
        )
    endif()

  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # pythonpath
  if(NOT APPLE)
    # This is not required for macOS where VTK python package is installed
    # in a standard location using CMake/SlicerBlockInstallExternalPythonModules.cmake
    if(UNIX)
      set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
        <APPLAUNCHER_SETTINGS_DIR>/../${Slicer_INSTALL_LIB_DIR}/python3.6/site-packages
        )
    else()
      set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
        <APPLAUNCHER_SETTINGS_DIR>/../${Slicer_INSTALL_BIN_DIR}/Lib/site-packages
        # Adding the following line is needed only for (<VTK_MAJOR_VERSION>.<VTK_MINOR_VERSION> <= 8.1),
        # but since we only have Slicer_VTK_VERSION_MAJOR variable, following the update of VTK
        # version from 9.0 to 8.2, there is no way to discriminate between 8.1 and 8.2.
        <APPLAUNCHER_SETTINGS_DIR>/../${Slicer_INSTALL_LIB_DIR}/python3.6/site-packages
        )
    endif()
    mark_as_superbuild(
      VARS ${proj}_PYTHONPATH_LAUNCHER_INSTALLED
      LABELS "PYTHONPATH_LAUNCHER_INSTALLED"
      )
  endif()

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(VTK_SOURCE_DIR:PATH)

mark_as_superbuild(
  VARS VTK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
