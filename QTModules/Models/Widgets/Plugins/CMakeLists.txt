project(qSlicerModelsWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
set(designer_plugin_SRCS
  qSlicerModelsWidgetsAbstractPlugin.cxx
  qSlicerModelsWidgetsAbstractPlugin.h
  qSlicerModelsWidgetsPlugin.cxx
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.cxx
  qMRMLModelDisplayNodeWidgetPlugin.h
  )

# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerModelsModule
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
