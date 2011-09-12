project(qMRMLWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "QMRML_WIDGETS_PLUGIN_EXPORT")

# Source files
set(designer_plugin_SRCS
  qMRMLWidgetsAbstractPlugin.cxx
  qMRMLWidgetsAbstractPlugin.h
  qMRMLWidgetsPlugin.cxx
  qMRMLWidgetsPlugin.h

  qMRMLCheckableNodeComboBoxPlugin.cxx
  qMRMLCheckableNodeComboBoxPlugin.h
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
  qMRMLEventBrokerWidgetPlugin.cxx
  qMRMLEventBrokerWidgetPlugin.h
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
  qMRMLThreeDViewPlugin.cxx
  qMRMLThreeDViewPlugin.h
  qMRMLTransformSlidersPlugin.cxx
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeViewPlugin.cxx
  qMRMLTreeViewPlugin.h
  qMRMLVolumeInfoWidgetPlugin.cxx
  qMRMLVolumeInfoWidgetPlugin.h
  qMRMLVolumePropertyNodeWidgetPlugin.cxx
  qMRMLVolumePropertyNodeWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.cxx
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetPlugin.cxx
  qMRMLWidgetPlugin.h
  qMRMLWindowLevelWidgetPlugin.cxx
  qMRMLWindowLevelWidgetPlugin.h
  )

# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qMRMLWidgetsPlugin.h

  qMRMLCheckableNodeComboBoxPlugin.h
  qMRMLClipNodeWidgetPlugin.h
  qMRMLCollapsibleButtonPlugin.h
  qMRMLColorListViewPlugin.h
  qMRMLColorTableComboBoxPlugin.h
  qMRMLColorTableViewPlugin.h
  qMRMLDisplayNodeWidgetPlugin.h
  qMRMLEventBrokerWidgetPlugin.h
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
  qMRMLThreeDViewPlugin.h
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeViewPlugin.h
  qMRMLVolumeInfoWidgetPlugin.h
  qMRMLVolumePropertyNodeWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetPlugin.h
  qMRMLWindowLevelWidgetPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qMRMLWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
