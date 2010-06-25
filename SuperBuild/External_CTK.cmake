#-----------------------------------------------------------------------------
# Get and build CTK
if (Slicer3_USE_QT)

  # Sanity checks
  if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
    message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
  endif()
  
  set(proj CTK)
  
  if(NOT DEFINED CTK_DIR)
#    message(STATUS "Adding project:${proj}")
    ExternalProject_Add(${proj}
      GIT_REPOSITORY "${git_protocol}://github.com/commontk/CTK.git"
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCTK_USE_GIT_PROTOCOL:BOOL=${Slicer3_USE_GIT_PROTOCOL}
        -DBUILD_TESTING:BOOL=OFF
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        -DVTK_DIR:PATH=${VTK_DIR}
        -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
        -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
        -DCTK_LIB_Widgets:BOOL=ON
        -DCTK_LIB_Visualization/VTK/Widgets:BOOL=ON
        -DCTK_LIB_Scripting/Python/Core:BOOL=${Slicer3_USE_PYTHONQT}
        -DCTK_LIB_Scripting/Python/Widgets:BOOL=${Slicer3_USE_PYTHONQT}
        -DCTK_LIB_PluginFramework:BOOL=OFF
        -DCTK_PLUGIN_org.commontk.eventbus:BOOL=OFF
        -Dgit_EXECUTABLE:FILEPATH=${git_EXECUTABLE}
      INSTALL_COMMAND ""
      DEPENDS 
        ${CTK_DEPENDENCIES}
      )
    set(CTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
    
  else()
    # The project is provided using CTK_DIR, nevertheless since other project may depend on CTK, 
    # let's add an 'empty' one
    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      BINARY_DIR ${proj}-build
      DOWNLOAD_COMMAND "" 
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS 
        ${CTK_DEPENDENCIES}
      )
  endif()
endif()
