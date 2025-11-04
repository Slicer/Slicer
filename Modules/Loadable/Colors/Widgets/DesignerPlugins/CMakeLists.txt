
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerColorsModuleWidgetsAbstractPlugin.cxx
  qSlicerColorsModuleWidgetsAbstractPlugin.h
  qSlicerColorsModuleWidgetsPlugin.cxx
  qSlicerColorsModuleWidgetsPlugin.h
  qMRMLColorLegendDisplayNodeWidgetPlugin.cxx
  qMRMLColorLegendDisplayNodeWidgetPlugin.h
  qMRMLColorListViewPlugin.cxx
  qMRMLColorListViewPlugin.h
  qMRMLColorTableViewPlugin.cxx
  qMRMLColorTableViewPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicer${MODULE_NAME}ModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Module-${MODULE_NAME}"
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
