PROJECT(qMRMLWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
SET(designer_plugin_SRCS
  qMRMLColorTableComboBoxPlugin.cxx
  qMRMLColorTableComboBoxPlugin.h
  qMRMLLinearTransformSliderPlugin.cxx
  qMRMLLinearTransformSliderPlugin.h
  qMRMLListWidgetPlugin.cxx
  qMRMLListWidgetPlugin.h
  qMRMLMatrixWidgetPlugin.cxx
  qMRMLMatrixWidgetPlugin.h
  qMRMLNodeComboBoxPlugin.cxx
  qMRMLNodeComboBoxPlugin.h
  qMRMLRangeWidgetPlugin.cxx
  qMRMLRangeWidgetPlugin.h
  qMRMLROIWidgetPlugin.cxx
  qMRMLROIWidgetPlugin.h
  qMRMLSliceViewWidgetPlugin.cxx
  qMRMLSliceViewWidgetPlugin.h
  qMRMLThreeDRenderViewPlugin.cxx
  qMRMLThreeDRenderViewPlugin.h
  qMRMLTransformSlidersPlugin.cxx
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeWidgetPlugin.cxx
  qMRMLTreeWidgetPlugin.h
  qMRMLSceneFactoryWidgetPlugin.cxx
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.cxx
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetsAbstractPlugin.cxx
  qMRMLWidgetsAbstractPlugin.h
  qMRMLWidgetPlugin.cxx
  qMRMLWidgetPlugin.h
  qMRMLWidgetsPlugin.cxx  
  qMRMLWidgetsPlugin.h
  qMRMLWindowLevelWidgetPlugin.cxx
  qMRMLWindowLevelWidgetPlugin.h
  )

# Headers that should run through moc
SET(designer_plugin_MOC_SRCS
  qMRMLWidgetsPlugin.h

  qMRMLColorTableComboBoxPlugin.h
  qMRMLLinearTransformSliderPlugin.h
  qMRMLListWidgetPlugin.h
  qMRMLMatrixWidgetPlugin.h
  qMRMLNodeComboBoxPlugin.h
  qMRMLRangeWidgetPlugin.h
  qMRMLROIWidgetPlugin.h
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLSliceViewWidgetPlugin.h
  qMRMLThreeDRenderViewPlugin.h
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetPlugin.h
  qMRMLWindowLevelWidgetPlugin.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qMRMLWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
