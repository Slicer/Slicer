PROJECT(qSlicer${qt_module_name}ModuleWidgets)

#
# See CMake/SlicerMacroBuildModuleWidgets.cmake for details
#

SET(module_widgets_name "${PROJECT_NAME}")

# The header '${module_widgets_name}Export.h' will be automatically configured.
SET(module_widgets_export_directive "Q_SLICER_MODULE_EMSEGMENT_WIDGET_EXPORT")


# Additional directories to include
SET(module_widgets_include_directories
  )

# Source files
SET(module_widgets_SRCS
  
  qSlicerEMSegmentWidget.cxx  
  qSlicerEMSegmentWidget.h
  
  qSlicerEMSegmentInputChannelListWidget.h
  qSlicerEMSegmentInputChannelListWidget.cxx
  
  qSlicerEMSegmentGraphWidget.h
  qSlicerEMSegmentGraphWidget.cxx
  
  qSlicerEMSegmentAnatomicalTreeWidget.h
  qSlicerEMSegmentAnatomicalTreeWidget.cxx
  qSlicerEMSegmentAnatomicalTreeWidget_p.h
  
  vtkPlotGaussian.h
  vtkPlotGaussian.cxx
  )

# Headers that should run through moc
SET(module_widgets_MOC_SRCS
  qSlicerEMSegmentAnatomicalTreeWidget.h
  qSlicerEMSegmentAnatomicalTreeWidget_p.h
  qSlicerEMSegmentGraphWidget.h
  qSlicerEMSegmentInputChannelListWidget.h
  qSlicerEMSegmentWidget.h
  )
  
# UI files
SET(module_widgets_UI_SRCS
  Resources/UI/qSlicerEMSegmentAnatomicalTreeWidget.ui
  Resources/UI/qSlicerEMSegmentGraphWidget.ui  
  )

# Resources
SET(module_widgets_resources
  )

# Additional Target libraries
SET(module_widgets_target_libraries
  vtkSlicerEMSegmentModuleLogic
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

# Plugin
add_subdirectory(Plugins)

