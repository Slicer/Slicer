PROJECT(qCTKWidgets)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


IF(NOT DEFINED BUILD_SHARED_LIBS)
  OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

# CMake Macros
INCLUDE(CMake/qCTKParseArgumentsMacro.cmake)
INCLUDE(CMake/qCTKDesignerPluginMacros.cmake)

# CMake module
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${qCTKWidgets_SOURCE_DIR}/CMake")

#-----------------------------------------------------------------------------
# Python wrapping using PythonQT
# If qCTKWidgets_WRAP_PYTHON is ON, PythonQT wrapping will be enabled
#

IF(qCTKWidgets_WRAP_PYTHON)
  FIND_PACKAGE(PythonQt)
  IF(PYTHONQT_FOUND)
    SET(qCTKWidgets_PYTHONQT_INCLUDE_DIR "${PYTHONQT_INCLUDE_DIR}")
    SET(qCTKWidgets_PYTHONQT_LIBRARIES ${PYTHONQT_LIBRARIES} ${PYTHON_LIBRARY})
  ELSE(PYTHONQT_FOUND)
    MESSAGE(SEND_ERROR "PythonQt not found.  Please set the required paths or disable qCTKWidgets_WRAP_PYTHON.")
  ENDIF(PYTHONQT_FOUND)
ENDIF(qCTKWidgets_WRAP_PYTHON)
SET(qCTKWidgets_WRAP_PYTHON ${qCTKWidgets_WRAP_PYTHON} CACHE INTERNAL "qCTKWidgets wrap python flag" FORCE)
SET(qCTKWidgets_PYTHONQT_INCLUDE_DIR ${qCTKWidgets_PYTHONQT_INCLUDE_DIR} CACHE INTERNAL "qCTKWidgets pythonQt includes" FORCE)
SET(qCTKWidgets_PYTHONQT_LIBRARIES ${qCTKWidgets_PYTHONQT_LIBRARIES} CACHE INTERNAL "qCTKWidgets pythonQt libraries" FORCE)

#-----------------------------------------------------------------------------
# Sources
#
SET(qCTKWidgets_SRCS
  qCTKPimpl.h
  qCTKAbstractFactory.h
  qCTKAbstractFactory.txx
  qCTKAbstractObjectFactory.h
  qCTKAbstractObjectFactory.txx
  qCTKAbstractPluginFactory.h
  qCTKAbstractPluginFactory.txx
  qCTKAbstractQObjectFactory.h
  qCTKAbstractQObjectFactory.txx  
  qCTKAbstractLibraryFactory.h
  qCTKAbstractLibraryFactory.txx
  qCTKAddRemoveComboBox.cxx
  qCTKAddRemoveComboBox.h
  qCTKButtonGroup.cxx
  qCTKButtonGroup.h
  qCTKCollapsibleButton.cxx
  qCTKCollapsibleButton.h
  qCTKCollapsibleGroupBox.cxx
  qCTKCollapsibleGroupBox.h
  qCTKColorPickerButton.cxx
  qCTKColorPickerButton.h
  qCTKConsoleWidget.cxx
  qCTKConsoleWidget.h
  qCTKCoordinatesWidget.cxx
  qCTKCoordinatesWidget.h
  qCTKDoubleSlider.cxx
  qCTKDoubleSlider.h
  qCTKDynamicSpacer.cxx
  qCTKDynamicSpacer.h
  qCTKFittedTextBrowser.cxx
  qCTKFittedTextBrowser.h
  qCTKFlowLayout.cxx
  qCTKFlowLayout.h
  qCTKMatrixWidget.cxx
  qCTKMatrixWidget.h
  qCTKMenuButton.cxx
  qCTKMenuButton.h
  qCTKModelTester.cxx
  qCTKModelTester.h
  qCTKNumericInput.cxx
  qCTKNumericInput.h
  qCTKSettings.cxx
  qCTKSettings.h
  qCTKSliderSpinBoxWidget.cxx
  qCTKSliderSpinBoxWidget.h
  qCTKTestApplication.cxx
  qCTKTestApplication.h
  qCTKTitleComboBox.cxx
  qCTKTitleComboBox.h
  qCTKTreeComboBox.cxx
  qCTKTreeComboBox.h
  qCTKUtils.cxx
  qCTKUtils.h
  )

# Headers that should run through moc
SET(qCTKWidgets_MOC_SRCS
  qCTKAddRemoveComboBox.h
  qCTKButtonGroup.h
  qCTKCollapsibleButton.h
  qCTKCollapsibleGroupBox.h
  qCTKColorPickerButton.h
  qCTKConsoleWidget.h
  qCTKCoordinatesWidget.h
  qCTKDoubleSlider.h
  qCTKDynamicSpacer.h
  qCTKFittedTextBrowser.h
  qCTKMatrixWidget.h
  qCTKMenuButton.h
  qCTKModelTester.h
  qCTKNumericInput.h
  qCTKSettings.h
  qCTKSliderSpinBoxWidget.h
  qCTKTestApplication.h
  qCTKTitleComboBox.h
  qCTKTreeComboBox.h
  )
  
# UI files
SET(qCTKWidgets_UI_SRCS
  Resources/UI/qCTKAddRemoveComboBox.ui
  Resources/UI/qCTKSliderSpinBoxWidget.ui
)

# Python wrap 
IF(qCTKWidgets_WRAP_PYTHON)

  # Sources
  SET(qCTKWidgets_SRCS
    ${qCTKWidgets_SRCS}
    qCTKAbstractPythonManager.cxx
    qCTKAbstractPythonManager.h
    qCTKPythonShell.cxx
    qCTKPythonShell.h
    )

  # Headers that should run through moc
  SET(qCTKWidgets_MOC_SRCS
    ${qCTKWidgets_MOC_SRCS}
    qCTKAbstractPythonManager.h
    qCTKPythonShell.h
    )
    
ENDIF(qCTKWidgets_WRAP_PYTHON)
  
QT4_WRAP_CPP(qCTKWidgets_SRCS ${qCTKWidgets_MOC_SRCS})
QT4_WRAP_UI(qCTKWidgets_UI_CXX ${qCTKWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qCTKWidgets_QRC_SRCS Resources/qCTKWidgets.qrc)

# --------------------------------------------------------------------------
# Include dirs
#

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${qCTKWidgets_PYTHONQT_INCLUDE_DIR}
  )

SET(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )

SET(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${include_dirs})

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers}
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the library
#

SET(lib_name "qCTKWidgets")

ADD_LIBRARY(${lib_name} 
  ${qCTKWidgets_SRCS}
  ${qCTKWidgets_UI_CXX}
  ${qCTKWidgets_QRC_SRCS}
)

SET(QT_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTWEBKIT_LIBRARY}
  )
  
SET(qCTKWidgets_LIBRARIES
  ${QT_LIBRARIES}
  ${qCTKWidgets_PYTHONQT_LIBRARIES}
  )
  
TARGET_LINK_LIBRARIES(${lib_name} ${qCTKWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer3_LIBRARY_PROPERTIES})
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
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Subdirectories
#
ADD_SUBDIRECTORY(Plugins)
