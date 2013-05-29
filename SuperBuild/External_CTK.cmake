
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
  message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(CTK_DEPENDENCIES VTK ${ITK_EXTERNAL_NAME})
if(Slicer_USE_PYTHONQT)
  list(APPEND CTK_DEPENDENCIES python)
endif()
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND CTK_DEPENDENCIES DCMTK)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(CTK)
set(proj CTK)

if(NOT DEFINED CTK_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # Set CMake OSX variable to pass down the external project
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT CMAKE_CONFIGURATION_TYPES)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  endif()

  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
      -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
      -DCTK_LIB_Scripting/Python/Core:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_USE_VTK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTCORE:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTGUI:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTUITOOLS:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTNETWORK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTWEBKIT:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Widgets:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_ENABLE_Python_Wrapping:BOOL=${Slicer_USE_PYTHONQT}
      )
  endif()

  if(Slicer_BUILD_DICOM_SUPPORT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      )
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/commontk/CTK.git"
    GIT_TAG "2c24951c992e7e8f3ec8ca49dc0cb26fee03ea37"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
      -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
      -DBUILD_TESTING:BOOL=OFF
      -DCTK_INSTALL_BIN_DIR:STRING=${Slicer_INSTALL_BIN_DIR}
      -DCTK_INSTALL_LIB_DIR:STRING=${Slicer_INSTALL_LIB_DIR}
      -DCTK_INSTALL_QTPLUGIN_DIR:STRING=${Slicer_INSTALL_QtPlugins_DIR}
      -DCTK_USE_GIT_PROTOCOL:BOOL=${Slicer_USE_GIT_PROTOCOL}
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      -DVTK_DIR:PATH=${VTK_DIR}
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
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${CTK_DEPENDENCIES}
    )
  set(CTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  # The project is provided using CTK_DIR, nevertheless since other project may depend on CTK,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${CTK_DEPENDENCIES}")
endif()
