PROJECT(QCTKWidgets)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

IF(NOT DEFINED BUILD_SHARED_LIBS)
  OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Sources
#
SET(QCTKWidgets_SRCS
  qCTKAbstractObjectFactory.h
  qCTKCollapsibleGroupBox.h
  qCTKCollapsibleGroupBox.cxx
  qCTKCollapsibleWidget.h
  qCTKCollapsibleWidget.cxx
  qCTKColorPickerButton.h
  qCTKColorPickerButton.cxx
  qCTKComboBoxAddRemove.cxx
  qCTKComboBoxAddRemove.h
  qCTKMatrixWidget.cxx
  qCTKMatrixWidget.h
  qCTKPositionWidget.cxx
  qCTKPositionWidget.h
  qCTKSlider.cxx
  qCTKSlider.h
  qCTKSliderSpinBoxLabel.cxx
  qCTKSliderSpinBoxLabel.h
  )

# Headers that should run through moc
SET(QCTKWidgets_MOC_SRCS
  qCTKCollapsibleGroupBox.h
  qCTKCollapsibleWidget.h
  qCTKColorPickerButton.h
  qCTKComboBoxAddRemove.h
  qCTKMatrixWidget.h
  qCTKPositionWidget.h
  qCTKSlider.h
  qCTKSliderSpinBoxLabel.h
  )
  
# UI files
SET(QCTKWidgets_UI_SRCS
  Resources/UI/qCTKComboBoxAddRemove.ui
  Resources/UI/qCTKSliderSpinBoxLabel.ui
)
  
QT4_WRAP_CPP(QCTKWidgets_SRCS ${QCTKWidgets_MOC_SRCS})
QT4_WRAP_UI(QCTKWidgets_UI_CXX ${QCTKWidgets_UI_SRCS})
QT4_ADD_RESOURCES(QCTKWidgets_QRC_SRCS Resources/qCTKWidgets.qrc)
   


# --------------------------------------------------------------------------
# Include dirs
#

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

SET(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )

SET(QT_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTWEBKIT_LIBRARY}
  )

SET(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${include_dirs})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qCTKWidgetsConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qCTKWidgetsConfigure.h
  )

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/qCTKWidgetsConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the library
#

SET(lib_name "QCTKWidgets")

ADD_LIBRARY(${lib_name} 
  ${QCTKWidgets_SRCS}
  ${QCTKWidgets_UI_CXX}
  ${QCTKWidgets_QRC_SRCS}
)

SET(QCTKWidgets_LIBRARIES
  ${QT_LIBRARIES}
  )
  
TARGET_LINK_LIBRARIES(${lib_name} 
  ${QCTKWidgets_LIBRARIES}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

# --------------------------------------------------------------------------
# Install
#
INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Subdirectories
#
ADD_SUBDIRECTORY(Plugins)
