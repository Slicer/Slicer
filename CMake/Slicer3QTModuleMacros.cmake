#
#
#

MACRO(Slicer3_build_qtmodule)
  SLICER3_PARSE_ARGUMENTS(QTMODULE
    "NAME;EXPORT_DIRECTIVE;SRCS;MOC_SRCS;UI_SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    "NO_RESOURCES"
    ${ARGN}
    )
  
  # Sanity checks
  IF(NOT DEFINED QTMODULE_NAME)
    MESSAGE(SEND_ERROR "NAME is mandatory")
  ENDIF(NOT DEFINED QTMODULE_NAME)
  
  IF(NOT DEFINED QTMODULE_EXPORT_DIRECTIVE)
    MESSAGE(SEND_ERROR "EXPORT_DIRECTIVE is mandatory")
  ENDIF(NOT DEFINED QTMODULE_EXPORT_DIRECTIVE)
  
  # Define library name
  SET(LIBNAME qSlicer${QTMODULE_NAME}Module)
  
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
  
  CONFIGURE_FILE(
    ${QTModules_SOURCE_DIR}/qSlicerQTModulesConfigure.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/qSlicer${QTMODULE_NAME}ModuleConfigure.h
    )
  
  CONFIGURE_FILE(
    ${QTModules_SOURCE_DIR}/qSlicerQTModulesWin32Header.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/qSlicer${QTMODULE_NAME}ModuleWin32Header.h
    )
  
  FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  INSTALL(FILES 
    ${headers} 
    "${CMAKE_CURRENT_BINARY_DIR}/qSlicerQTModules${QTMODULE_NAME}Configure.h"
    "${CMAKE_CURRENT_BINARY_DIR}/qSlicerQTModules${QTMODULE_NAME}Win32Header.h"
    DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
    )
  
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
  IF (${QTMODULE_NO_RESOURCES} EQUAL FALSE)
    QT4_ADD_RESOURCES(qSlicerModule_QRC_SRCS Resources/qSlicer${QTMODULE_NAME}Module.qrc)
  ENDIF(${QTMODULE_NO_RESOURCES} EQUAL FALSE)

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

  ADD_LIBRARY(${LIBNAME}
    ${qSlicerModule_SRCS}
    ${qSlicerModule_UI_CXX}
    ${qSlicerModule_QRC_SRCS}
    #${qSlicerModule_TCL_SRCS}
    )
  slicer3_set_modules_output_path(${LIBNAME})

  TARGET_LINK_LIBRARIES(${LIBNAME}
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
    SET_TARGET_PROPERTIES(${LIBNAME} PROPERTIES
      ${Slicer3_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer3_LIBRARY_PROPERTIES)
  
  slicer3_install_modules(${LIBNAME})

ENDMACRO(Slicer3_build_qtmodule)


