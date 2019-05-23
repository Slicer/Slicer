set(KIT ${PROJECT_NAME})
project(${KIT}Plugins)

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}PluginsExport.h' will be automatically configured.
set(${KIT}_EXPORT "QMRML_WIDGETS_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qMRMLWidgetsAbstractPlugin.cxx
  qMRMLWidgetsAbstractPlugin.h
  qMRMLWidgetsPlugin.cxx
  qMRMLWidgetsPlugin.h

  qMRMLCaptureToolBarPlugin.h
  qMRMLCaptureToolBarPlugin.cxx
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
  qMRMLCoordinatesWidgetPlugin.cxx
  qMRMLCoordinatesWidgetPlugin.h
  qMRMLDisplayNodeViewComboBoxPlugin.cxx
  qMRMLDisplayNodeViewComboBoxPlugin.h
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
  qMRMLNodeAttributeTableViewPlugin.cxx
  qMRMLNodeAttributeTableViewPlugin.h
  qMRMLNodeAttributeTableWidgetPlugin.cxx
  qMRMLNodeAttributeTableWidgetPlugin.h
  qMRMLNodeComboBoxPlugin.cxx
  qMRMLNodeComboBoxPlugin.h
  qMRMLPlotViewControllerWidgetPlugin.cxx
  qMRMLPlotViewControllerWidgetPlugin.h
  qMRMLPlotWidgetPlugin.cxx
  qMRMLPlotWidgetPlugin.h
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
  qMRMLSliderWidgetPlugin.cxx
  qMRMLSliderWidgetPlugin.h
  qMRMLSpinBoxPlugin.cxx
  qMRMLSpinBoxPlugin.h
  qMRMLTableViewPlugin.cxx
  qMRMLTableViewPlugin.h
  qMRMLThreeDViewInformationWidgetPlugin.cxx
  qMRMLThreeDViewInformationWidgetPlugin.h
  qMRMLThreeDViewPlugin.cxx
  qMRMLThreeDViewPlugin.h
  qMRMLTransformSlidersPlugin.cxx
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeViewPlugin.cxx
  qMRMLTreeViewPlugin.h
  qMRMLVolumeInfoWidgetPlugin.cxx
  qMRMLVolumeInfoWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.cxx
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetPlugin.cxx
  qMRMLWidgetPlugin.h
  qMRMLWindowLevelWidgetPlugin.cxx
  qMRMLWindowLevelWidgetPlugin.h
  )

if(MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND ${KIT}_SRCS
    qMRMLExpandingWebViewPlugin.cxx
    qMRMLExpandingWebViewPlugin.h
    )
endif()

# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qMRMLWidgetsPlugin.h

  qMRMLCaptureToolBarPlugin.h
  qMRMLCheckableNodeComboBoxPlugin.h
  qMRMLClipNodeWidgetPlugin.h
  qMRMLCollapsibleButtonPlugin.h
  qMRMLColorListViewPlugin.h
  qMRMLColorTableComboBoxPlugin.h
  qMRMLColorTableViewPlugin.h
  qMRMLCoordinatesWidgetPlugin.h
  qMRMLDisplayNodeViewComboBoxPlugin.h
  qMRMLDisplayNodeWidgetPlugin.h
  qMRMLEventBrokerWidgetPlugin.h
  qMRMLLabelComboBoxPlugin.h
  qMRMLLayoutWidgetPlugin.h
  qMRMLLinearTransformSliderPlugin.h
  qMRMLListWidgetPlugin.h
  qMRMLMatrixWidgetPlugin.h
  qMRMLModelInfoWidgetPlugin.h
  qMRMLNavigationViewPlugin.h
  qMRMLNodeAttributeTableViewPlugin.h
  qMRMLNodeAttributeTableWidgetPlugin.h
  qMRMLNodeComboBoxPlugin.h
  qMRMLPlotViewControllerWidgetPlugin.h
  qMRMLPlotWidgetPlugin.h
  qMRMLRangeWidgetPlugin.h
  qMRMLROIWidgetPlugin.h
  qMRMLScalarInvariantComboBoxPlugin.h
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLSliceControllerWidgetPlugin.h
  qMRMLSliceInformationWidgetPlugin.h
  qMRMLSliceWidgetPlugin.h
  qMRMLSliderWidgetPlugin.h
  qMRMLSpinBoxPlugin.h
  qMRMLTableViewPlugin.h
  qMRMLThreeDViewInformationWidgetPlugin.h
  qMRMLThreeDViewPlugin.h
  qMRMLTransformSlidersPlugin.h
  qMRMLTreeViewPlugin.h
  qMRMLVolumeInfoWidgetPlugin.h
  qMRMLVolumeThresholdWidgetPlugin.h
  qMRMLWidgetPlugin.h
  qMRMLWindowLevelWidgetPlugin.h
  )

if(MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND ${KIT}_MOC_SRCS
    qMRMLExpandingWebViewPlugin.h
    )
endif()

set(${KIT}_TARGET_LIBRARIES
  qMRMLWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "${${KIT}_FOLDER}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
