PROJECT(QCTKWidgetsPlugin)

# Find QT
FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTDESIGNER_INCLUDE_DIR}
)

# Create the plugin for Qt Designer
SET(QCTKWidgetsPlugin_SRCS
  qCTKCollapsibleGroupBoxPlugin.cxx
  qCTKCollapsibleWidgetPlugin.cxx
  qCTKCollapsibleWidgetContainerExtension.cxx
  qCTKColorPickerButtonPlugin.cxx
  qCTKCoordinatesWidgetPlugin.cxx
  qCTKExtensionFactory.cxx
  qCTKTitleComboBoxPlugin.cxx
  qCTKWidgetsPlugins.cxx  
  qCTKWidgetsPlugin.cxx
  )

SET(QCTKWidgetsPlugin_MOC_SRCS
  qCTKCollapsibleGroupBoxPlugin.h
  qCTKCollapsibleWidgetPlugin.h
  qCTKCollapsibleWidgetContainerExtension.h
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.h
  qCTKExtensionFactory.h
  qCTKTitleComboBoxPlugin.h
  qCTKWidgetsPlugins.h
  )

QT4_WRAP_CPP(QCTKWidgetsPlugin_SRCS ${QCTKWidgetsPlugin_MOC_SRCS})
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)

ADD_LIBRARY(QCTKWidgetsPlugin SHARED ${QCTKWidgetsPlugin_SRCS})
SET_TARGET_PROPERTIES(QCTKWidgetsPlugin  PROPERTIES 
                      COMPILE_FLAGS "-DQT_PLUGIN -DQT_NO_DEBUG")

TARGET_LINK_LIBRARIES(QCTKWidgetsPlugin 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDESIGNER_LIBRARY}
  QCTKWidgets
  )
