project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleLogic_INCLUDE_DIRS}
  ${qSlicerSegmentationsEditorEffects_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLSegmentsTableView.cxx
  qMRMLSegmentsTableView.h
  qMRMLSegmentationRepresentationsListView.cxx
  qMRMLSegmentationRepresentationsListView.h
  qMRMLSegmentationConversionParametersWidget.cxx
  qMRMLSegmentationConversionParametersWidget.h
  qMRMLSegmentEditorWidget.cxx
  qMRMLSegmentEditorWidget.h
  qMRMLSegmentationDisplayNodeWidget.cxx
  qMRMLSegmentationDisplayNodeWidget.h
  qMRMLDoubleSpinBoxDelegate.cxx
  qMRMLDoubleSpinBoxDelegate.h
  )

set(${KIT}_MOC_SRCS
  qMRMLSegmentsTableView.h
  qMRMLSegmentationRepresentationsListView.h
  qMRMLSegmentationConversionParametersWidget.h
  qMRMLSegmentEditorWidget.h
  qMRMLSegmentationDisplayNodeWidget.h
  qMRMLDoubleSpinBoxDelegate.h
)

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLSegmentsTableView.ui
  Resources/UI/qMRMLSegmentationRepresentationsListView.ui
  Resources/UI/qMRMLSegmentationConversionParametersWidget.ui
  Resources/UI/qMRMLSegmentEditorWidget.ui
  Resources/UI/qMRMLSegmentationDisplayNodeWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerSegmentationsModuleMRML
  vtkSlicerSegmentationsModuleLogic
  qSlicerSegmentationsEditorEffects
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
add_subdirectory(DesignerPlugins)
