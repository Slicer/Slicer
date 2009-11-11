PROJECT(SlicerQTGUIPlugin)
INCLUDE_DIRECTORIES(  
  ${QT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTDESIGNER_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Create the plugin for Qt Designer
SET(qSlicerQTGUIPlugin_SRCS
  qSlicerQTGUIPlugin.cxx
  qSlicerQTGUIPlugins.cxx
  qSlicerWidgetPlugin.cxx
  )

SET(qSlicerQTGUIPlugin_MOC_SRCS
  qSlicerQTGUIPlugins.h
  qSlicerWidgetPlugin.h
  )

QT4_WRAP_CPP(qSlicerQTGUIPlugin_SRCS ${qSlicerQTGUIPlugin_MOC_SRCS})
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qSlicerQTGUIPluginConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerQTGUIPluginConfigure.h
  )

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
INSTALL(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/qSlicerQTGUIPluginConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

SET(lib_name "SlicerQTGUIPlugin")

ADD_LIBRARY(${lib_name} SHARED ${qSlicerQTGUIPlugin_SRCS})
SET_TARGET_PROPERTIES(${lib_name}  PROPERTIES 
                      COMPILE_FLAGS "-DQT_PLUGIN -DQT_NO_DEBUG")

TARGET_LINK_LIBRARIES(${lib_name} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDESIGNER_LIBRARY}
  SlicerBaseQTGUI
  )
  
# --------------------------------------------------------------------------
# Install the library

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )
