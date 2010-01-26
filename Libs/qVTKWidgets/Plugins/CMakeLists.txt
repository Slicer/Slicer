PROJECT(qVTKWidgetsPlugin)

#
# See qCTKWidgets/CMake/qCTKDesignerPluginMacros.cmake for details
#
  
# Source files
SET(designer_plugin_SRCS
  qVTKRenderViewPlugin.cxx
  qVTKRenderViewPlugin.h
  qVTKWidgetsAbstractPlugin.cxx
  qVTKWidgetsAbstractPlugin.h
  qVTKWidgetsPlugin.cxx
  qVTKWidgetsPlugin.h
  )

# Headers that should run through moc
SET(designer_plugin_MOC_SRCS
  qVTKRenderViewPlugin.h
  qVTKWidgetsPlugin.h
  )

SET(designer_plugin_TARGET_LIBRARIES
  qVTKWidgets
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
