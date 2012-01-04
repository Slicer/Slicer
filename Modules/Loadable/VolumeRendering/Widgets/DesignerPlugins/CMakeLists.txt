set(KIT ${PROJECT_NAME}Plugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}PluginsExport.h' will be automatically configured.
set(${KIT}_EXPORT "Q_VOLUMERENDERING_MODULE_WIDGETS_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qMRMLVolumePropertyNodeWidgetPlugin.cxx
  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.h
  qSlicerVolumeRenderingModuleWidgetsPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qSlicerPresetComboBoxPlugin.cxx
  qSlicerPresetComboBoxPlugin.h
  )


# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerPresetComboBoxPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerVolumeRenderingModuleWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

