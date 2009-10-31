
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTDESIGNER_INCLUDE_DIR}
)

# Create the plugin for Qt Designer
SET(qMRMLWidgetsPlugin_SRCS
  qMRMLWidgetsPlugins.cxx  
  qMRMLWidgetsPlugin.cxx
  qMRMLLinearTransformSliderPlugin.cxx
  qMRMLMatrixWidgetPlugin.cxx
  qMRMLNodeSelectorPlugin.cxx
  qMRMLTransformSlidersPlugin.cxx
  )

SET(qMRMLWidgetsPlugin_MOC_SRCS
  qMRMLWidgetsPlugins.h
  qMRMLLinearTransformSliderPlugin.h
  qMRMLMatrixWidgetPlugin.h
  qMRMLNodeSelectorPlugin.h
  qMRMLTransformSlidersPlugin.h
  )

QT4_WRAP_CPP(qMRMLWidgetsPlugin_SRCS ${qMRMLWidgetsPlugin_MOC_SRCS})
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qMRMLWidgetsPluginConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qMRMLWidgetsPluginConfigure.h
  )

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
INSTALL(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/qMRMLWidgetsPluginConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

SET(lib_name "qMRMLWidgetsPlugin")

ADD_LIBRARY(${lib_name} SHARED ${qMRMLWidgetsPlugin_SRCS})
SET_TARGET_PROPERTIES(${lib_name}  PROPERTIES 
                      COMPILE_FLAGS "-DQT_PLUGIN -DQT_NO_DEBUG")

TARGET_LINK_LIBRARIES(${lib_name} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDESIGNER_LIBRARY}
  qMRMLWidgets
  qCTKWidgets
  )
  
# --------------------------------------------------------------------------
# Install the library

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
