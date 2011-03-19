PROJECT(qMRMLWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
SET(designer_plugin_SRCS
  qMRMLClipNodeWidgetPlugin.cxx
  qMRMLClipNodeWidgetPlugin.h
  qMRMLCollapsibleButtonPlugin.cxx
  qMRMLCollapsibleButtonPlugin.h
  qMRMLColorListViewPlugin.cxx
  qMRMLColorListViewPlugin.h
  qMRMLColorTableComboBoxPlugin.cxx
  qMRMLColorTableComboBoxPlugin.h
  qMRMLColorTableViewPlugin.cxx
  qMRMLColorTableViewPlugin.h
  qMRMLDisplayNodeWidgetPlugin.cxx
  qMRMLDisplayNodeWidgetPlugin.h
  qMRMLLabelComboBoxPlugin.cxx
  qMRMLLabelComboBoxPlugin.h
  qMRMLLayoutWidgetPlugin.cxx
  qMRMLLayoutWidgetPlugin.h
  qMRMLLinearTransformSliderPlugin.cxx
  qMRMLLinearTransformSliderPlugin.h
  qMRMLListWidgetPlugin.cxx
  qMRMLListWidgetPlugin.h
  qMRMLMatrixWidgetPlugin.cxx
  qMRMLMatrixWidgetPlugin.h
  qMRMLModelInfoWidgetPlugin.cxx
  qMRMLModelInfoWidgetPlugin.h
  qMRMLNavigationViewPlugin.cxx
  qMRMLNavigationViewPlugin.h
  qMRMLNodeComboBoxPlugin.cxx
  qMRMLNodeComboBoxPlugin.h
  qMRMLRangeWidgetPlugin.cxx
  qMRMLRangeWidgetPlugin.h
  qMRMLROIWidgetPlugin.cxx
  qMRMLROIWidgetPlugin.h
  qMRMLScalarInvariantComboBoxPlugin.cxx
  qMRMLScalarInvariantComboBoxPlugin.h
  qMRMLSceneFactoryWidgetPlugin.cxx
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLSliceControllerWidgetPlugin.cxx
  qMRMLSliceControllerWidgetPlugin.h
  qMRMLSliceInformationWidgetPlugin.cxx
  qMRMLSliceInformationWidgetPlugin.h
  qMRMLSliceWidgetPlugin.cxx
  qMRMLSliceWidgetPlugin.h
  qMRMLSlicesControllerToolBarPlugin.cxx
  qMRMLSlicesControllerToolBarPlugin.h
  qMRMLThreeDViewPlugin.cxx
  qMRMLThreeDViewPlugin.h
  qMRMLThreeDViewsControllerWidgetPlugin.cxx
  qMRMLThreeDViewsControllerWidgetPlugin.h
  qMRMLTransformSlidersPlugin.cxx
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeWidgetPlugin.cxx
  qMRMLTreeWidgetPlugin.h
  qMRMLVolumeInfoWidgetPlugin.cxx
  qMRMLVolumeInfoWidgetPlugin.h
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

  qMRMLClipNodeWidgetPlugin.h
  qMRMLCollapsibleButtonPlugin.h
  qMRMLColorListViewPlugin.h
  qMRMLColorTableComboBoxPlugin.h
  qMRMLColorTableViewPlugin.h
  qMRMLDisplayNodeWidgetPlugin.h
  qMRMLLabelComboBoxPlugin.h
  qMRMLLayoutWidgetPlugin.h
  qMRMLLinearTransformSliderPlugin.h
  qMRMLListWidgetPlugin.h
  qMRMLMatrixWidgetPlugin.h
  qMRMLModelInfoWidgetPlugin.h
  qMRMLNavigationViewPlugin.h
  qMRMLNodeComboBoxPlugin.h
  qMRMLRangeWidgetPlugin.h
  qMRMLROIWidgetPlugin.h
  qMRMLScalarInvariantComboBoxPlugin.h
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLSliceControllerWidgetPlugin.h
  qMRMLSliceInformationWidgetPlugin.h
  qMRMLSliceWidgetPlugin.h
  qMRMLSlicesControllerToolBarPlugin.h
  qMRMLThreeDViewPlugin.h
  qMRMLThreeDViewsControllerWidgetPlugin.h
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeWidgetPlugin.h
  qMRMLVolumeInfoWidgetPlugin.h
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
