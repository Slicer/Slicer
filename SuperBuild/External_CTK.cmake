
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
  message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(CTK_DEPENDENCIES VTK Insight)
if(Slicer_USE_PYTHONQT)
  list(APPEND CTK_DEPENDENCIES python)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(CTK)
set(proj CTK)

if(NOT DEFINED CTK_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/commontk/CTK.git"
    GIT_TAG "65dce256334147fb224a4884721f6e72050cfce1"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
      -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DBUILD_TESTING:BOOL=OFF
      -DCTK_INSTALL_LIB_DIR:STRING=${Slicer_INSTALL_LIB_DIR}
      -DCTK_USE_GIT_PROTOCOL:BOOL=${Slicer_USE_GIT_PROTOCOL}
      -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
      -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
      -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
      -DCTK_LIB_Widgets:BOOL=ON
      -DCTK_LIB_Visualization/VTK/Widgets:BOOL=ON
      -DCTK_LIB_ImageProcessing/ITK/Core:BOOL=ON
      -DCTK_LIB_Scripting/Python/Core:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_USE_VTK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTCORE:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTGUI:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTUITOOLS:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Core_PYTHONQT_WRAP_QTNETWORK:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_Scripting/Python/Widgets:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_WRAP_PYTHONQT_LIGHT:BOOL=${Slicer_USE_PYTHONQT}
      -DCTK_LIB_PluginFramework:BOOL=OFF
      -DCTK_PLUGIN_org.commontk.eventbus:BOOL=OFF
      -DCTK_APP_ctkDICOM:BOOL=ON
      -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
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

