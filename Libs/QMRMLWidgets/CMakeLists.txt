PROJECT(QMRMLWidgets)

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
SET(QMRMLWidgets_SRCS
  qVTKObject.h
  qVTKConnection.h
  qVTKConnection.cxx
  qVTKObjectEventsObserver.h
  qVTKObjectEventsObserver.cxx
  qMRMLUtils.h
  qMRMLUtils.cxx
  qMRMLNodeSelector.h
  qMRMLNodeSelector.cxx
  qMRMLLinearTransformSlider.h
  qMRMLLinearTransformSlider.cxx
  qMRMLTransformSliders.h
  qMRMLTransformSliders.cxx
  qMRMLMatrixWidget.h
  qMRMLMatrixWidget.cxx
  )

# Headers that should run through moc
SET(QMRMLWidgets_MOC_SRCS
  qVTKConnection.h
  qVTKObjectEventsObserver.h
  qMRMLNodeSelector.h
  qMRMLLinearTransformSlider.h
  qMRMLTransformSliders.h
  qMRMLMatrixWidget.h
  )
  
# UI files
SET(QMRMLWidgets_UI_SRCS
  Resources/UI/qMRMLTransformSliders.ui
)
  
QT4_WRAP_CPP(QMRMLWidgets_SRCS ${QMRMLWidgets_MOC_SRCS})
QT4_WRAP_UI(QMRMLWidgets_UI_CXX ${QMRMLWidgets_UI_SRCS})
QT4_ADD_RESOURCES(QMRMLWidgets_QRC_SRCS Resources/qMRMLWidgets.qrc)
   


# --------------------------------------------------------------------------
# Include dirs
#

SET(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

SET(QT_INCLUDE_DIRS
  ${QT_INCLUDE_DIR} 
  ${QT_QTWEBKIT_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR} 
  ${QT_QTCORE_INCLUDE_DIR} 
  )

FOREACH(package 
    QCTKWidgets
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  IF(NOT DEFINED ${package}_SOURCE_DIR)
    SET(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  ENDIF(NOT DEFINED ${package}_SOURCE_DIR)
  IF(NOT DEFINED ${package}_BINARY_DIR)
    STRING(REPLACE ${PROJECT_NAME} ${package} 
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    LINK_DIRECTORIES(${${package}_BINARY_DIR})
  ENDIF(NOT DEFINED ${package}_BINARY_DIR)

  SET(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
ENDFOREACH(package)

SET(include_dirs ${include_dirs} ${QT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${include_dirs})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qMRMLWidgetsConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/qMRMLWidgetsConfigure.h
  )

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/qMRMLWidgetsConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the library
#

SET(lib_name "QMRMLWidgets")

ADD_LIBRARY(${lib_name} 
  ${QMRMLWidgets_SRCS}
  ${QMRMLWidgets_UI_CXX}
  ${QMRMLWidgets_QRC_SRCS}
)

SET(QMRMLWidgets_LIBRARIES 
  ${QT_LIBRARIES}
  MRML
  )
  
TARGET_LINK_LIBRARIES(${lib_name} ${QMRMLWidgets_LIBRARIES})

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
