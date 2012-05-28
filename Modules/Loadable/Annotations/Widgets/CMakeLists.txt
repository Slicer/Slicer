project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  qMRMLAnnotationROIWidget.cxx
  qMRMLAnnotationROIWidget.h
  #qMRMLAnnotationDisplayNodePropertyWidget.cxx
  #qMRMLAnnotationDisplayNodePropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget.cxx
  #qMRMLAnnotationDisplayNodePointPropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  )

set(${KIT}_MOC_SRCS
  qMRMLAnnotationROIWidget.h
  #qMRMLAnnotationDisplayNodePropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget.h
  #qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLAnnotationROIWidget.ui
  #Resources/UI/qMRMLAnnotationDisplayNodePointPropertyWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerAnnotationsModuleMRML
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
add_subdirectory(DesignerPlugins)
