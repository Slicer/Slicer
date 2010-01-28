PROJECT(qCTKWidgetsPlugin)

#
# See qCTKWidgets/CMake/qCTKDesignerPluginMacros.cmake for details
#
  
# Source files
SET(designer_plugin_SRCS
  qCTKWidgetsPlugin.cxx  
  qCTKWidgetsAbstractPlugin.cxx

  qCTKCollapsibleButtonPlugin.cxx
  qCTKCollapsibleButtonPlugin.h
  qCTKCollapsibleGroupBoxPlugin.cxx
  qCTKCollapsibleGroupBoxPlugin.h
  qCTKColorPickerButtonPlugin.cxx
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.cxx
  qCTKCoordinatesWidgetPlugin.h
  qCTKDoubleSliderPlugin.cxx
  qCTKDoubleSliderPlugin.h
  qCTKExtensionFactory.cxx
  qCTKExtensionFactory.h
  qCTKFittedTextBrowserPlugin.cxx
  qCTKFittedTextBrowserPlugin.h  
  qCTKNumericInputPlugin.cxx
  qCTKNumericInputPlugin.h
  qCTKTitleComboBoxPlugin.cxx
  qCTKTitleComboBoxPlugin.h
  qCTKTreeComboBoxPlugin.cxx
  qCTKTreeComboBoxPlugin.h
  qCTKSliderSpinBoxLabelPlugin.cxx
  qCTKSliderSpinBoxLabelPlugin.h
  )

SET(designer_plugin_MOC_SRCS
  qCTKWidgetsPlugin.h

  qCTKCollapsibleButtonPlugin.h 
  qCTKCollapsibleGroupBoxPlugin.h
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.h
  qCTKDoubleSliderPlugin.h
  qCTKExtensionFactory.h
  qCTKFittedTextBrowserPlugin.h  
  qCTKNumericInputPlugin.h
  qCTKTitleComboBoxPlugin.h
  qCTKTreeComboBoxPlugin.h
  qCTKSliderSpinBoxLabelPlugin.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qCTKWidgets
  )

qctk_build_designer_plugin( 
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

# TODO Move to a Slicer CMakeLists.txt
qctk_install_designer_plugin(
  NAME ${PROJECT_NAME}
  INSTALL_BIN_DIR ${Slicer3_INSTALL_BIN_DIR}
  INSTALL_LIB_DIR ${Slicer3_INSTALL_LIB_DIR}
  INSTALL_HEADERS_DIR ${Slicer3_INSTALL_INCLUDE_DIR}
  )
