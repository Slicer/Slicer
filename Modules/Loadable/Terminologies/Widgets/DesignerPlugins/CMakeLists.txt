set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerTerminologiesModuleWidgetsAbstractPlugin.cxx
  qSlicerTerminologiesModuleWidgetsAbstractPlugin.h
  qSlicerTerminologiesModuleWidgetsPlugin.cxx
  qSlicerTerminologiesModuleWidgetsPlugin.h
  qSlicerTerminologyNavigatorWidgetPlugin.cxx
  qSlicerTerminologyNavigatorWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerTerminologiesModuleWidgetsPlugin.h
  qSlicerTerminologyNavigatorWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerTerminologiesModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Module-${MODULE_NAME}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
