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
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
INSTALL(FILES 
  ${files}
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )
ADD_LIBRARY(${PROJECT_NAME} MODULE ${qCTKWidgetsPlugin_SRCS})
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
ELSE(WIN32)
  ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME}
    POST_BUILD
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

