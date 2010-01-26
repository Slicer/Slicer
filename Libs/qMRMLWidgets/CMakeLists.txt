PROJECT(qMRMLWidgets)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


IF(NOT DEFINED BUILD_SHARED_LIBS)
  OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ON)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# Sources
#
SET(qMRMLWidgets_SRCS
  qMRMLEventLogger.cxx
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.cxx
  qMRMLEventLoggerWidget.h
  qMRMLItemHelper.cxx
  qMRMLItemHelper.h
  qMRMLLinearTransformSlider.cxx
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.cxx
  qMRMLListWidget.h
  qMRMLMatrixWidget.cxx
  qMRMLMatrixWidget.h
  qMRMLNodeFactory.cxx
  qMRMLNodeFactory.h
  qMRMLNodeFactoryButton.cxx
  qMRMLNodeFactoryButton.h
  qMRMLNodeSelector.cxx
  qMRMLNodeSelector.h
  qMRMLNodeTreeSelector.cxx
  qMRMLNodeTreeSelector.h
  qMRMLSceneGeneratorButton.cxx
  qMRMLSceneGeneratorButton.h
  qMRMLSceneModel.cxx
  qMRMLSceneModel.h
  qMRMLTransformProxyModel.cxx
  qMRMLTransformProxyModel.h
  qMRMLTransformSliders.cxx
  qMRMLTransformSliders.h
  qMRMLTreeProxyModel.cxx
  qMRMLTreeProxyModel.h
  qMRMLTreeProxyModel_p.h
  qMRMLTreeWidget.cxx
  qMRMLTreeWidget.h
  qMRMLUtils.cxx
  qMRMLUtils.h
  )

# Headers that should run through moc
SET(qMRMLWidgets_MOC_SRCS
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.h
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.h
  qMRMLMatrixWidget.h
  qMRMLNodeFactory.h
  qMRMLNodeFactoryButton.h
  qMRMLNodeSelector.h
  qMRMLNodeTreeSelector.h
  qMRMLSceneGeneratorButton.h
  qMRMLSceneModel.h
  qMRMLTransformProxyModel.h
  qMRMLTreeProxyModel.h
  qMRMLTreeProxyModel_p.h
  qMRMLTransformSliders.h
  qMRMLTreeWidget.h
  )
  
# UI files
SET(qMRMLWidgets_UI_SRCS
  Resources/UI/qMRMLTransformSliders.ui
  Resources/UI/qMRMLEventLoggerWidget.ui
)
  
QT4_WRAP_CPP(qMRMLWidgets_SRCS ${qMRMLWidgets_MOC_SRCS})
QT4_WRAP_UI(qMRMLWidgets_UI_CXX ${qMRMLWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qMRMLWidgets_QRC_SRCS Resources/qMRMLWidgets.qrc)

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

FOREACH(package 
    qCTKWidgets
    qVTKWidgets
    MRML
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

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES 
  ${headers} 
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the library
#

SET(lib_name "qMRMLWidgets")

ADD_LIBRARY(${lib_name} 
  ${qMRMLWidgets_SRCS}
  ${qMRMLWidgets_UI_CXX}
  ${qMRMLWidgets_QRC_SRCS}
)

SET(qMRMLWidgets_LIBRARIES 
  ${QT_LIBRARIES}
  MRML
  qCTKWidgets
  qVTKWidgets
  )
  
TARGET_LINK_LIBRARIES(${lib_name} ${qMRMLWidgets_LIBRARIES})

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
# Sub directories
#
ADD_SUBDIRECTORY(Plugins)
ADD_SUBDIRECTORY(Testing)
