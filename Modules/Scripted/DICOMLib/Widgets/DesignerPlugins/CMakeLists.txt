
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerDICOMLibModuleWidgetsAbstractPlugin.cxx
  qSlicerDICOMLibModuleWidgetsAbstractPlugin.h
  qSlicerDICOMLibModuleWidgetsPlugin.cxx
  qSlicerDICOMLibModuleWidgetsPlugin.h
  qSlicerDICOMTagEditorWidgetPlugin.cxx
  qSlicerDICOMTagEditorWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerDICOMLibModuleWidgetsPlugin.h
  qSlicerDICOMTagEditorWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerDICOMLibModuleWidgets
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
