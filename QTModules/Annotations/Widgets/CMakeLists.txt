project(qSlicer${qt_module_name}ModuleWidgets)

#
# See CMake/SlicerMacroBuildModuleWidgets.cmake for details
#

set(module_widgets_name "${PROJECT_NAME}")

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_EXPORT")

# Additional directories to include
set(module_widgets_include_directories
  )

# Source files
set(module_widgets_SRCS
  qMRMLAnnotationROIWidget.cxx
  qMRMLAnnotationROIWidget.h
  qMRMLAnnotationDisplayNodePropertyWidget.cxx
  qMRMLAnnotationDisplayNodePropertyWidget.h
  qMRMLAnnotationDisplayNodePointPropertyWidget.cxx
  qMRMLAnnotationDisplayNodePointPropertyWidget.h
  qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  )

# Headers that should run through moc
set(module_widgets_MOC_SRCS
  qMRMLAnnotationROIWidget.h
  qMRMLAnnotationDisplayNodePropertyWidget.h  
  qMRMLAnnotationDisplayNodePointPropertyWidget.h
  qMRMLAnnotationDisplayNodePointPropertyWidget_p.h
  )

# UI files
set(module_widgets_UI_SRCS
  Resources/UI/qMRMLAnnotationROIWidget.ui
  Resources/UI/qMRMLAnnotationDisplayNodePointPropertyWidget.ui
  )

# Resources
set(module_widgets_resources
  )

# Additional Target libraries
set(module_widgets_target_libraries
  vtkSlicerAnnotationsModuleMRML
  )

SlicerMacroBuildModuleWidgets(
  NAME ${module_widgets_name}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  INCLUDE_DIRECTORIES ${module_widgets_include_directories}
  SRCS ${module_widgets_SRCS}
  MOC_SRCS ${module_widgets_MOC_SRCS}
  UI_SRCS ${module_widgets_UI_SRCS}
  TARGET_LIBRARIES ${module_widgets_target_libraries}
  RESOURCES ${module_widgets_resources}
  WRAP_PYTHONQT
  )

# Qt Designer Plugins
add_subdirectory(DesignerPlugins)
