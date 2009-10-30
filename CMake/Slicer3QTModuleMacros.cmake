#
#
#

MACRO(Slicer3_build_qtmodule)
  SLICER3_PARSE_ARGUMENTS(QTMODULE
    "NAME;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    "NO_RESOURCES"
    ${ARGN}
    )
  
  # --------------------------------------------------------------------------
  # Find Slicer3

  if(NOT Slicer3_SOURCE_DIR)
    find_package(Slicer3 REQUIRED)
    include(${Slicer3_USE_FILE})
    slicer3_set_default_install_prefix_for_external_projects()
  endif(NOT Slicer3_SOURCE_DIR)

  # --------------------------------------------------------------------------
  # Include dirs

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer3_Libs_INCLUDE_DIRS}
    ${Slicer3_Base_INCLUDE_DIRS}
    ${QTMODULE_INCLUDE_DIRECTORIES}
    )

  #configure_file(
  #  ${CMAKE_CURRENT_SOURCE_DIR}/vtkVolumesConfigure.h.in 
  #  ${CMAKE_CURRENT_BINARY_DIR}/vtkVolumesConfigure.h
  #  )
  
  #file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  #install(FILES 
  #  ${headers} 
  #  "${CMAKE_CURRENT_BINARY_DIR}/vtkVolumesConfigure.h"
  #  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  #  )
  
  #file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
  #install(FILES 
  #  ${files}
  #  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  #  )
  
  #-----------------------------------------------------------------------------
  # Sources
  #
  
  QT4_WRAP_CPP(qSlicerModule_SRCS ${QTMODULE_MOC_SRCS})
  QT4_WRAP_UI(qSlicerModule_UI_CXX ${QTMODULE_UI_SRCS})
  IF (NOT ${QTMODULE_NO_RESOURCES})
    QT4_ADD_RESOURCES(qSlicerModule_QRC_SRCS Resources/qSlicer${QTMODULE_NAME}Module.qrc)
  ENDIF(NOT ${QTMODULE_NO_RESOURCES})

  SET_SOURCE_FILES_PROPERTIES(
    ${qSlicerModule_UI_CXX}
    ${qSlicerModule_SRCS}
    WRAP_EXCLUDE
    )

  # --------------------------------------------------------------------------
  # Wrapping
  
  #include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  #vtk_wrap_tcl3(Volumes 
  #  Volumes_TCL_SRCS 
  #  "${Volumes_SRCS}" "")
  
  #---------------------------------------------------------------------------
  # Add Loadable Module support
  
  #generatelm(Volumes_SRCS SlicerVolumes.txt)
  
  # --------------------------------------------------------------------------
  # Build and install the library

  SET(lib_name qSlicer${QTMODULE_NAME}Module)
  ADD_LIBRARY(${lib_name}
    ${qSlicerModule_SRCS}
    ${qSlicerModule_UI_CXX}
    ${qSlicerModule_QRC_SRCS}
    #${qSlicerModule_TCL_SRCS}
    )
  slicer3_set_modules_output_path(${lib_name})

  TARGET_LINK_LIBRARIES(${lib_name}
    ${Slicer3_Libs_LIBRARIES}
    ${Slicer3_Base_LIBRARIES}
    ${QTMODULE_TARGET_LIBRARIES}
    #${KWWidgets_LIBRARIES}
    #${ITK_LIBRARIES}
    #CommandLineModule
    #SlicerTractographyDisplay
    #SlicerTractographyFiducialSeeding
    )

  # Apply user-defined properties to the library target.
  IF(Slicer3_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)
  
  slicer3_install_modules(${lib_name})

ENDMACRO(Slicer3_build_qtmodule)


