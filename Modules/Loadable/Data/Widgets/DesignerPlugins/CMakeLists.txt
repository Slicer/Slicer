set(KIT ${PROJECT_NAME}Plugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#
# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_MODULE_DATA_WIDGETS_PLUGINS_EXPORT")

# Source files
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


# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerDataModuleWidgetsPlugin.h

  qMRMLNodeAttributeTableViewPlugin.h
  qMRMLNodeAttributeTableWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerDataModuleWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

