PROJECT(qMRMLWidgets)

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
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${qMRMLWidgets_SOURCE_DIR}/CMake")

#-----------------------------------------------------------------------------
# Sources
#
SET(qMRMLWidgets_SRCS
  qCTKFlowLayout.cxx
  qCTKFlowLayout.h
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
  qMRMLNodeComboBox.cxx
  qMRMLNodeComboBox.h
  qMRMLNodeFactory.cxx
  qMRMLNodeFactory.h
  qMRMLNodeSelector.cxx
  qMRMLNodeSelector.h
  qMRMLNodeTreeSelector.cxx
  qMRMLNodeTreeSelector.h
  qMRMLSceneFactoryWidget.cxx
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.cxx
  qMRMLSceneModel.h
  qMRMLSliceViewWidget.cxx
  qMRMLSliceViewWidget.h
  qMRMLSliceViewWidget_p.h
  qMRMLSortFilterProxyModel.cxx
  qMRMLSortFilterProxyModel.h
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
  qMRMLVolumeThresholdWidget.cxx
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.cxx
  qMRMLWidget.h
  qMRMLWindowLevelWidget.cxx
  qMRMLWindowLevelWidget.h
  vtkMRMLThreeDRenderViewInteractorStyle.cxx
  vtkMRMLThreeDRenderViewInteractorStyle.h
  )

# Headers that should run through moc
SET(qMRMLWidgets_MOC_SRCS
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.h
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.h
  qMRMLMatrixWidget.h
  qMRMLNodeComboBox.h
  qMRMLNodeFactory.h
  qMRMLNodeSelector.h
  qMRMLNodeTreeSelector.h
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.h
  qMRMLSliceViewWidget.h
  qMRMLSliceViewWidget_p.h
  qMRMLSortFilterProxyModel.h
  qMRMLTransformProxyModel.h
  qMRMLTreeProxyModel.h
  qMRMLTreeProxyModel_p.h
  qMRMLTransformSliders.h
  qMRMLTreeWidget.h
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.h
  qMRMLWindowLevelWidget.h
  )

# UI files
SET(qMRMLWidgets_UI_SRCS
  Resources/UI/qMRMLEventLoggerWidget.ui
  Resources/UI/qMRMLSceneFactoryWidget.ui
  Resources/UI/qMRMLSliceViewWidget.ui
  Resources/UI/qMRMLTransformSliders.ui
  Resources/UI/qMRMLVolumeThresholdWidget.ui
  Resources/UI/qMRMLWindowLevelWidget.ui
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
  ${CTK_LIBRARIES}
  ${CTK_EXTERNAL_LIBRARIES}
  MRML
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
