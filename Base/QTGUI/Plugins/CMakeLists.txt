PROJECT(qSlicerBaseQTGUIPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# Source files
SET(designer_plugin_SRCS
  qSlicerQTGUIAbstractPlugin.cxx
  qSlicerQTGUIAbstractPlugin.h
  qSlicerQTGUIPlugins.cxx
  qSlicerQTGUIPlugins.h
  qSlicerWidgetPlugin.cxx
  qSlicerWidgetPlugin.h
  qSlicerModulePanelPlugin.cxx
  qSlicerModulePanelPlugin.h
  qSlicerModuleSelectorWidgetPlugin.cxx
  qSlicerModuleSelectorWidgetPlugin.h
  )

# Headers that should run through moc
SET(designer_plugin_MOC_SRCS
  qSlicerQTGUIPlugins.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qSlicerBaseQTCLI
  qSlicerBaseQTGUI
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

qctk_install_designer_plugin(
  NAME ${PROJECT_NAME}
  INSTALL_BIN_DIR ${Slicer3_INSTALL_BIN_DIR}
  INSTALL_LIB_DIR ${Slicer3_INSTALL_LIB_DIR}/designer
  INSTALL_HEADERS_DIR ${Slicer3_INSTALL_INCLUDE_DIR}
  )
