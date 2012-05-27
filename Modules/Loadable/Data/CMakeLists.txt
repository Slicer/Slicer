
#-----------------------------------------------------------------------------
set(MODULE_NAME "Data") # Do not use 'project()'
set(MODULE_TITLE "${MODULE_NAME}")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${vtkSlicerCamerasModuleLogic_SOURCE_DIR}
  ${vtkSlicerCamerasModuleLogic_BINARY_DIR}
  )

set(MODULE_SRCS
  qSlicerDataModule.cxx
  qSlicerDataModule.h
  qSlicerDataModuleWidget.cxx
  qSlicerDataModuleWidget.h
  qSlicerSceneIO.cxx
  qSlicerSceneIO.h
  qSlicerSceneIOOptionsWidget.cxx
  qSlicerSceneIOOptionsWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerDataModule.h
  qSlicerDataModuleWidget.h
  qSlicerSceneIO.h
  qSlicerSceneIOOptionsWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerDataModule.ui
  Resources/UI/qSlicerSceneIOOptionsWidget.ui
  )

set(MODULE_TARGET_LIBRARIES
  qSlicerDataModuleWidgets
  vtkSlicerDataModuleLogic
  vtkSlicerCamerasModuleLogic
  )

set(MODULE_RESOURCES
  Resources/qSlicerDataModule.qrc
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
