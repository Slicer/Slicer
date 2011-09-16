PROJECT(qSlicerEMSegmentWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
SET(designer_plugin_SRCS
  qSlicerEMSegmentWidgetPlugin.cxx
  qSlicerEMSegmentWidgetPlugin.h
  qSlicerEMSegmentWidgetsAbstractPlugin.cxx
  qSlicerEMSegmentWidgetsAbstractPlugin.h
  qSlicerEMSegmentWidgetsPlugin.cxx
  qSlicerEMSegmentWidgetsPlugin.h

  qSlicerEMSegmentAnatomicalTreeWidgetPlugin.cxx
  qSlicerEMSegmentAnatomicalTreeWidgetPlugin.h
  qSlicerEMSegmentInputChannelListWidgetPlugin.cxx
  qSlicerEMSegmentInputChannelListWidgetPlugin.h
  qSlicerEMSegmentWidgetPlugin.cxx
  qSlicerEMSegmentWidgetPlugin.h
  )

# Headers that should run through moc
SET(designer_plugin_MOC_SRCS
  qSlicerEMSegmentWidgetPlugin.h
  qSlicerEMSegmentWidgetsPlugin.h

  qSlicerEMSegmentAnatomicalTreeWidgetPlugin.h
  qSlicerEMSegmentInputChannelListWidgetPlugin.h
  qSlicerEMSegmentWidgetPlugin.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qSlicerEMSegmentModule
  )

#qctk_build_designer_plugin(
#  NAME ${PROJECT_NAME}
#  SRCS ${designer_plugin_SRCS}
#  MOC_SRCS ${designer_plugin_MOC_SRCS}
#  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
#  )
