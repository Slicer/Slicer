project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  )

set(${KIT}_SRCS
  #qMRMLAnnotationDisplayNodePropertyWidget.cxx
  #qMRMLAnnotationDisplayNodePropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget.cxx
  #qMRMLAnnotationDisplayNodePointPropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  qMRMLAnnotationFiducialProjectionPropertyWidget.cxx
  qMRMLAnnotationFiducialProjectionPropertyWidget.h
  qMRMLAnnotationROIWidget.cxx
  qMRMLAnnotationROIWidget.h
  qMRMLAnnotationRulerProjectionPropertyWidget.cxx
  qMRMLAnnotationRulerProjectionPropertyWidget.h
  qMRMLAnnotationTreeView.cxx
  qMRMLAnnotationTreeView.h
  qMRMLSceneAnnotationModel.cxx
  qMRMLSceneAnnotationModel.h
  )

set(${KIT}_MOC_SRCS
  #qMRMLAnnotationDisplayNodePropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  qMRMLAnnotationFiducialProjectionPropertyWidget.h
  qMRMLAnnotationROIWidget.h
  qMRMLAnnotationRulerProjectionPropertyWidget.h
  qMRMLAnnotationTreeView.h
  qMRMLSceneAnnotationModel.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLAnnotationFiducialProjectionPropertyWidget.ui
  Resources/UI/qMRMLAnnotationROIWidget.ui
  Resources/UI/qMRMLAnnotationRulerProjectionPropertyWidget.ui
  #Resources/UI/qMRMLAnnotationDisplayNodePointPropertyWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerAnnotationsModuleMRML
  vtkSlicerAnnotationsModuleLogic
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
if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()
