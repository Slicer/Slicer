
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerMarkupsModuleWidgetsAbstractPlugin.cxx
  qSlicerMarkupsModuleWidgetsAbstractPlugin.h
  qSlicerMarkupsModuleWidgetsPlugin.cxx
  qSlicerMarkupsModuleWidgetsPlugin.h

  qMRMLMarkupsFiducialProjectionPropertyWidgetPlugin.cxx
  qMRMLMarkupsFiducialProjectionPropertyWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerMarkupsModuleWidgetsPlugin.h

  qMRMLMarkupsFiducialProjectionPropertyWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerMarkupsModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
