
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerDataModuleWidgetsAbstractPlugin.cxx
  qSlicerDataModuleWidgetsAbstractPlugin.h
  qSlicerDataModuleWidgetsPlugin.cxx
  qSlicerDataModuleWidgetsPlugin.h

  qMRMLNodeAttributeTableViewPlugin.cxx
  qMRMLNodeAttributeTableViewPlugin.h
  qMRMLNodeAttributeTableWidgetPlugin.cxx
  qMRMLNodeAttributeTableWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerDataModuleWidgetsPlugin.h

  qMRMLNodeAttributeTableViewPlugin.h
  qMRMLNodeAttributeTableWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerDataModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

