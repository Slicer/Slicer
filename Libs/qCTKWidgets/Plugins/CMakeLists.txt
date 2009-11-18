PROJECT(qCTKWidgetsPlugin)

INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTDESIGNER_INCLUDE_DIR}
)

# Create the plugin for Qt Designer
SET(qCTKWidgetsPlugin_SRCS
  qCTKWidgetsPlugin.cxx  
  qCTKWidgetsBasePlugin.cxx

  qCTKCollapsibleGroupBoxPlugin.cxx
  qCTKCollapsibleWidgetPlugin.cxx
  qCTKCollapsibleWidget2Plugin.cxx
  qCTKCollapsibleWidgetContainerExtension.cxx
  qCTKColorPickerButtonPlugin.cxx
  qCTKCoordinatesWidgetPlugin.cxx
  qCTKExtensionFactory.cxx
  qCTKNumericInputPlugin.cxx
  qCTKTitleComboBoxPlugin.cxx
  qCTKTreeComboBoxPlugin.cxx
  )

SET(qCTKWidgetsPlugin_MOC_SRCS
  qCTKWidgetsPlugin.h

  qCTKCollapsibleGroupBoxPlugin.h
  qCTKCollapsibleWidgetPlugin.h 
  qCTKCollapsibleWidget2Plugin.h
  qCTKCollapsibleWidgetContainerExtension.h
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.h
  qCTKExtensionFactory.h
  qCTKNumericInputPlugin.h
  qCTKTitleComboBoxPlugin.h
  qCTKTreeComboBoxPlugin.h
  )

QT4_WRAP_CPP(qCTKWidgetsPlugin_SRCS ${qCTKWidgetsPlugin_MOC_SRCS})
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qCTKWidgetsPluginConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qCTKWidgetsPluginConfigure.h
  )

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
INSTALL(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/qCTKWidgetsPluginConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

SET(lib_name "qCTKWidgetsPlugin")

ADD_LIBRARY(${lib_name} SHARED ${qCTKWidgetsPlugin_SRCS})
SET_TARGET_PROPERTIES(${lib_name}  PROPERTIES 
                      COMPILE_FLAGS "-DQT_PLUGIN -DQT_NO_DEBUG")

TARGET_LINK_LIBRARIES(${lib_name} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDESIGNER_LIBRARY}
  qCTKWidgets
  )

# --------------------------------------------------------------------------
# Install the library

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
