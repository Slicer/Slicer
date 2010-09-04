PROJECT(qSlicerEMSegmentWorkflowWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
SET(designer_plugin_SRCS
  qSlicerEMSegmentWorkflowWidgetsAbstractPlugin.cxx
  qSlicerEMSegmentWorkflowWidgetsAbstractPlugin.h
  qSlicerEMSegmentWorkflowWidgetsPlugin.cxx
  qSlicerEMSegmentWorkflowWidgetsPlugin.h

  qSlicerEMSegmentWorkflowWidgetStepPlugin.cxx
  qSlicerEMSegmentWorkflowWidgetStepPlugin.h
  )

# Headers that should run through moc
SET(designer_plugin_MOC_SRCS
  qSlicerEMSegmentWorkflowWidgetsPlugin.h

  qSlicerEMSegmentWorkflowWidgetStepPlugin.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qSlicerEMSegmentModule
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
