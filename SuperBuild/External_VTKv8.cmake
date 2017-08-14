
set(proj VTKv8)

# Set dependency list
set(${proj}_DEPENDENCIES "zlib")
if (Slicer_USE_PYTHONQT)
  list(APPEND ${proj}_DEPENDENCIES python)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
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


if((NOT DEFINED VTK_DIR OR NOT DEFINED VTK_SOURCE_DIR) AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  set(VTK_WRAP_TCL OFF)
  set(VTK_WRAP_PYTHON OFF)

  if(Slicer_USE_PYTHONQT)
    set(VTK_WRAP_PYTHON ON)
  endif()

  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DPYTHON_EXECUTABLE:PATH=${PYTHON_EXECUTABLE}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      )
  endif()

  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
    #-DDESIRED_QT_VERSION:STRING=4 # Unused
    -DVTK_USE_GUISUPPORT:BOOL=ON
    -DVTK_USE_QVTK_QTOPENGL:BOOL=ON
    -DVTK_USE_QT:BOOL=ON
    -DModule_vtkTestingRendering:BOOL=ON
    -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
    )
  if("${Slicer_VTK_RENDERING_BACKEND}" STREQUAL "OpenGL2")
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DModule_vtkGUISupportQtOpenGL:BOOL=ON
    )
  endif()
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
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
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
        -DModule_vtkRenderingFreeTypeFontConfig:BOOL=ON
        )
    endif()
  endif()


  # Disable Tk when Python wrapping is enabled
  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS -DVTK_USE_TK:BOOL=OFF)
  endif()

  if(VTK_WRAP_TCL)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DTCL_INCLUDE_PATH:PATH=${TCL_INCLUDE_PATH}
      -DTK_INCLUDE_PATH:PATH=${TK_INCLUDE_PATH}
      -DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}
      -DTK_LIBRARY:FILEPATH=${TK_LIBRARY}
      -DTCL_TCLSH:FILEPATH=${TCL_TCLSH}
      )
  endif()

  # Enable VTK_ENABLE_KITS only if CMake >= 3.0 is used
  set(VTK_ENABLE_KITS 0)
  if(CMAKE_MAJOR_VERSION EQUAL 3)
    set(VTK_ENABLE_KITS 1)
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_SetIfNotDefined(
    ${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY
    "${git_protocol}://github.com/Slicer/VTK.git"
    QUIET
    )

set(_git_tag)
if("${Slicer_VTK_VERSION_MAJOR}" STREQUAL "7")
  set(_git_tag "a024cefc2acf25350734e6f04d2562f9a6a3b124")
elseif("${Slicer_VTK_VERSION_MAJOR}" STREQUAL "8")
  set(_git_tag "1bd58766e90068c04d3e1b34eb43cda713e7d9e7")
else()
  message(FATAL_ERROR "error: Unsupported Slicer_VTK_VERSION_MAJOR: ${Slicer_VTK_VERSION_MAJOR}")
endif()
  ExternalProject_SetIfNotDefined(
    ${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG
    ${_git_tag}
    QUIET
    )

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${${CMAKE_PROJECT_NAME}_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${${CMAKE_PROJECT_NAME}_${proj}_GIT_TAG}"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
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
      -DVTK_WRAP_TCL:BOOL=${VTK_WRAP_TCL}
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
      -DVTK_ENABLE_KITS:BOOL=${VTK_ENABLE_KITS}
      -DVTK_RENDERING_BACKEND:STRING=${Slicer_VTK_RENDERING_BACKEND}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(VTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(VTK_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

#  set(PNG_INCLUDE_DIR ${VTK_SOURCE_DIR}/ThirdParty/png/vtkpng)

#  set(PNG_LIBRARY_DIR ${VTK_DIR}/lib)
#  if(CMAKE_CONFIGURATION_TYPES)
#    set(PNG_LIBRARY_DIR ${PNG_LIBRARY_DIR}/${CMAKE_CFG_INTDIR})
#  endif()
#  if(WIN32)
#    set(PNG_LIBRARY ${PNG_LIBRARY_DIR}/vtkpng-6.0.lib)
#  elseif(APPLE)
#    set(PNG_LIBRARY ${PNG_LIBRARY_DIR}/libvtkpng-6.0.dylib)
#  else()
#    set(PNG_LIBRARY ${PNG_LIBRARY_DIR}/libvtkpng-6.0.so)
#  endif()

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
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${VTK_DIR}/Wrapping/Python
    ${VTK_DIR}/${_library_output_subdir}/<CMAKE_CFG_INTDIR>
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # pythonpath
  if(NOT APPLE)
    set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
      <APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}/python2.7/site-packages
      )
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

ExternalProject_Message(${proj} "PNG_INCLUDE_DIR:${PNG_INCLUDE_DIR}")
ExternalProject_Message(${proj} "PNG_LIBRARY:${PNG_LIBRARY}")
