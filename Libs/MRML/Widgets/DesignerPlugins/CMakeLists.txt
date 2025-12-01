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

  qMRMLCheckableNodeComboBoxPlugin.cxx
  qMRMLCheckableNodeComboBoxPlugin.h
  qMRMLClipNodeDisplayWidgetPlugin.cxx
  qMRMLClipNodeDisplayWidgetPlugin.h
  qMRMLClipNodeWidgetPlugin.cxx
  qMRMLClipNodeWidgetPlugin.h
  qMRMLCollapsibleButtonPlugin.cxx
  qMRMLCollapsibleButtonPlugin.h
  qMRMLColorTableComboBoxPlugin.cxx
  qMRMLColorTableComboBoxPlugin.h
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
  qMRMLScalarsDisplayWidgetPlugin.cxx
  qMRMLScalarsDisplayWidgetPlugin.h
  qMRMLSceneFactoryWidgetPlugin.cxx
  qMRMLSceneFactoryWidgetPlugin.h
  qMRMLSliceControllerWidgetPlugin.cxx
  qMRMLSliceControllerWidgetPlugin.h
  qMRMLSliceInformationWidgetPlugin.cxx
  qMRMLSliceInformationWidgetPlugin.h
  qMRMLSliceVerticalControllerWidgetPlugin.cxx
  qMRMLSliceVerticalControllerWidgetPlugin.h
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

set(${KIT}_TARGET_LIBRARIES
  qMRMLWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "${${KIT}_FOLDER}"
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

target_precompile_headers(${PROJECT_NAME} REUSE_FROM PrecompileQtCoreHeaders)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM PrecompileQtWidgetsHeaders)
