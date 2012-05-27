
#-----------------------------------------------------------------------------
set(MODULE_NAME "TractographyDisplay") # Do not use 'project()'
set(MODULE_TITLE "Tractography Display")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${vtkSlicerTractographyDisplayModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyDisplayModuleLogic_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  )

set(MODULE_SRCS
  qSlicerFiberBundleIO.cxx
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.cxx
  qSlicerTractographyDisplayModule.h
  )

set(MODULE_MOC_SRCS
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.h
  )

set(MODULE_UI_SRCS
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerTractographyDisplayModuleLogic
  vtkSlicerTractographyDisplayModuleMRML
  qSlicerTractographyDisplayModuleWidgets
  ${MRML_LIBRARIES}
  )

set(MODULE_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
