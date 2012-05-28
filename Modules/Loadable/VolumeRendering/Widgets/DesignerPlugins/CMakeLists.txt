
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

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

set(${KIT}_MOC_SRCS
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerPresetComboBoxPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerVolumeRenderingModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
