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
  qCTKWidgetsAbstractPlugin.cxx

  qCTKCollapsibleButtonPlugin.cxx
  qCTKCollapsibleButtonPlugin.h
  qCTKCollapsibleGroupBoxPlugin.cxx
  qCTKCollapsibleGroupBoxPlugin.h
  qCTKColorPickerButtonPlugin.cxx
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.cxx
  qCTKCoordinatesWidgetPlugin.h
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
  qCTKSliderPlugin.cxx
  qCTKSliderPlugin.h
  )

SET(qCTKWidgetsPlugin_MOC_SRCS
  qCTKWidgetsPlugin.h

  qCTKCollapsibleButtonPlugin.h 
  qCTKCollapsibleGroupBoxPlugin.h
  qCTKColorPickerButtonPlugin.h
  qCTKCoordinatesWidgetPlugin.h
  qCTKExtensionFactory.h
  qCTKFittedTextBrowserPlugin.h  
  qCTKNumericInputPlugin.h
  qCTKTitleComboBoxPlugin.h
  qCTKTreeComboBoxPlugin.h
  qCTKSliderPlugin.h
  )

QT4_WRAP_CPP(qCTKWidgetsPlugin_SRCS ${qCTKWidgetsPlugin_MOC_SRCS})
#SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
#SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)
#SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/designer)

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

ADD_LIBRARY(${PROJECT_NAME} SHARED ${qCTKWidgetsPlugin_SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME}  PROPERTIES 
                      COMPILE_FLAGS "-DQT_PLUGIN -DQT_NO_DEBUG"
                      )

TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDESIGNER_LIBRARY}
  qCTKWidgets
  )

GET_TARGET_PROPERTY(FILE_PATH ${PROJECT_NAME} LOCATION)
GET_TARGET_PROPERTY(DIR_PATH ${PROJECT_NAME} LIBRARY_OUTPUT_DIRECTORY)
GET_FILENAME_COMPONENT(LIB_NAME ${PROJECT_NAME} NAME)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    #COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${CMAKE_BINARY_DIR}/bin/designer/
    COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${DIR_PATH}/${CMAKE_CFG_INTDIR}/designer/${CMAKE_SHARED_LIBRARY_PREFIX}${LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
ELSE(WIN32)
  ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E create_symlink ${FILE_PATH} ${CMAKE_BINARY_DIR}/bin/designer/${CMAKE_SHARED_LIBRARY_PREFIX}${LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${FILE_PATH} ${DIR_PATH}/designer/${CMAKE_SHARED_LIBRARY_PREFIX}${LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )  
ENDIF(WIN32)
# --------------------------------------------------------------------------
# Install the library

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )


# The SLICRE_INSTALL_QT_PLUGIN_DIR variable sets the location
# in which the Qt plugin will be installed.  It may or may not contain
# variable references to CMAKE_INSTALL_PREFIX and SLICER_INSTALL_QT_DIR.
# The default is to install to SLICER_INSTALL_QT_DIR under the installation
# prefix.  The default SLICER_INSTALL_QT_DIR will allow the designer plugin
# path to be set to vtk-install-prefix/plugins to get the plugin.
IF(NOT SLICER_INSTALL_QT_DIR)
  SET(SLICER_INSTALL_QT_DIR /plugins/designer)
ENDIF(NOT SLICER_INSTALL_QT_DIR)

IF(NOT SLICER_INSTALL_NO_QT_PLUGIN)
  # Set default plugin install directory.
  SET(DOLLAR "$")
  IF(DEFINED SLICER_INSTALL_QT_PLUGIN_DIR)
  ELSE(DEFINED SLICER_INSTALL_QT_PLUGIN_DIR)
    SET(SLICER_INSTALL_QT_PLUGIN_DIR "${DOLLAR}{CMAKE_INSTALL_PREFIX}${DOLLAR}{SLICER_INSTALL_QT_DIR}"
      CACHE STRING "Directory in which the Slicer Qt plugin is placed during installation.")
    MARK_AS_ADVANCED(SLICER_INSTALL_QT_PLUGIN_DIR)
  ENDIF(DEFINED SLICER_INSTALL_QT_PLUGIN_DIR)

  # Configure the plugin install script.  This is used instead of
  # INSTALL TARGETS to allow the plugin to be installed outside the
  # main install prefix.  Attach the script as a post-install script.
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/PluginInstall.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/PluginInstall.cmake
                 @ONLY IMMEDIATE)
  #SET_TARGET_PROPERTIES(qCTKWidgetsPlugin PROPERTIES INSTALL
   # ${CMAKE_CURRENT_BINARY_DIR}/PluginInstall.cmake
  #  )
ENDIF(NOT SLICER_INSTALL_NO_QT_PLUGIN)


INSTALL(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/PluginInstall.cmake)
