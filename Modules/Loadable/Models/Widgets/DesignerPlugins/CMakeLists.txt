
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerModelsWidgetsAbstractPlugin.cxx
  qSlicerModelsWidgetsAbstractPlugin.h
  qSlicerModelsWidgetsPlugin.cxx
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.cxx
  qMRMLModelDisplayNodeWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerModelsModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
