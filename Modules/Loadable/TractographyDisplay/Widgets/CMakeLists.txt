project(qSlicer${qt_module_name}ModuleWidgets)

#
# See CMake/SlicerMacroBuildModuleWidgets.cmake for details
#

set(module_widgets_name "${PROJECT_NAME}")

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT")

# Additional directories to include
set(module_widgets_include_directories
  ${CMAKE_BUILD_DIR}
  ${vtkSlicerTractographyDisplayModuleMRML_SOURCE_DIR}
  ${vtkSlicerTractographyDisplayModuleMRML_BINARY_DIR}
  )

# Source files
set(module_widgets_SRCS
  qSlicerTractographyDisplayModuleWidget.cxx
  qSlicerTractographyDisplayModuleWidget.h
  qSlicerTractographyDisplayBasicWidget.cxx
  qSlicerTractographyDisplayBasicWidget.h
  qSlicerTractographyEditorROIWidget.cxx
  qSlicerTractographyEditorROIWidget.h
  qSlicerTractographyDisplayGlyphWidget.cxx
  qSlicerTractographyDisplayGlyphWidget.h
  qSlicerTractographyDisplayWidget.cxx
  qSlicerTractographyDisplayWidget.h
  )

# Headers that should run through moc
set(module_widgets_MOC_SRCS
  qSlicerTractographyDisplayModuleWidget.h
  qSlicerTractographyDisplayBasicWidget.h
  qSlicerTractographyEditorROIWidget.h
  qSlicerTractographyDisplayGlyphWidget.h
  qSlicerTractographyDisplayWidget.h
  )

# UI files
set(module_widgets_UI_SRCS
  Resources/UI/qSlicerTractographyDisplayModule.ui
  Resources/UI/qSlicerTractographyDisplayBasicWidget.ui
  Resources/UI/qSlicerTractographyEditorROIWidget.ui
  Resources/UI/qSlicerTractographyDisplayGlyphWidget.ui
  Resources/UI/qSlicerTractographyDisplayWidget.ui
  )

# Resources
set(module_widgets_resources
  )

# Additional Target libraries
set(module_widgets_target_libraries
  vtkSlicerTractographyDisplayModuleLogic
  ${MRML_LIBRARIES}
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
  )

# Plugin
#add_subdirectory(Plugins)
