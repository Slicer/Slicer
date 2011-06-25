project(qSlicer${qt_module_name}ModuleWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
set(designer_plugin_SRCS
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.h
  qSlicerVolumeRenderingModuleWidgetsPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qSlicerPresetComboBoxPlugin.cxx
  qSlicerPresetComboBoxPlugin.h
  )


# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qSlicerPresetComboBoxPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerVolumeRenderingModuleWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

