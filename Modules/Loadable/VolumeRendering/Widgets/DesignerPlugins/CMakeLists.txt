
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.h
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h

  qMRMLVolumePropertyNodeWidgetPlugin.cxx
  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerPresetComboBoxPlugin.cxx
  qSlicerPresetComboBoxPlugin.h
  qSlicerVolumeRenderingPresetComboBoxPlugin.cxx
  qSlicerVolumeRenderingPresetComboBoxPlugin.h
  qSlicerGPUMemoryComboBoxPlugin.cxx
  qSlicerGPUMemoryComboBoxPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h

  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerPresetComboBoxPlugin.h
  qSlicerVolumeRenderingPresetComboBoxPlugin.h
  qSlicerGPUMemoryComboBoxPlugin.h
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
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
