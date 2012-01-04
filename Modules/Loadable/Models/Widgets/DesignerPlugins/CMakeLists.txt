set(KIT ${PROJECT_NAME}Plugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_QTMODULES_MODELS_WIDGETS_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qSlicerModelsWidgetsAbstractPlugin.cxx
  qSlicerModelsWidgetsAbstractPlugin.h
  qSlicerModelsWidgetsPlugin.cxx
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.cxx
  qMRMLModelDisplayNodeWidgetPlugin.h
  )

# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerModelsModuleWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
