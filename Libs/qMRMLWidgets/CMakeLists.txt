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
  qMRMLCollapsibleButton.cxx
  qMRMLCollapsibleButton.h
  qMRMLColorTableComboBox.cxx
  qMRMLColorTableComboBox.h
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
  qMRMLRangeWidget.cxx
  qMRMLRangeWidget.h
  qMRMLROIWidget.cxx
  qMRMLROIWidget.h
  qMRMLSceneCategoryModel.cxx
  qMRMLSceneCategoryModel.h
  qMRMLSceneColorTableModel.cxx
  qMRMLSceneColorTableModel.h
  qMRMLSceneFactoryWidget.cxx
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.cxx
  qMRMLSceneModel.h
  qMRMLSceneTransformModel.cxx
  qMRMLSceneTransformModel.h
  qMRMLSceneTreeModel.cxx
  qMRMLSceneTreeModel.h
  qMRMLSliceControllerWidget.cxx
  qMRMLSliceControllerWidget.h
  qMRMLSliceControllerWidget_p.h
  qMRMLSliceInformationWidget.cxx
  qMRMLSliceInformationWidget.h
  qMRMLSliceInformationWidget_p.h
  qMRMLSliceWidget.cxx
  qMRMLSliceWidget.h
  qMRMLSliceWidget_p.h
  qMRMLSlicesControllerToolBar.cxx
  qMRMLSlicesControllerToolBar.h
  qMRMLSortFilterProxyModel.cxx
  qMRMLSortFilterProxyModel.h
  qMRMLThreeDView.cxx
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewsControllerWidget.cxx
  qMRMLThreeDViewsControllerWidget.h
  qMRMLThreeDViewsControllerWidget_p.h
  qMRMLTransformSliders.cxx
  qMRMLTransformSliders.h
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
  )

# Headers that should run through moc
SET(qMRMLWidgets_MOC_SRCS
  qMRMLCollapsibleButton.h
  qMRMLColorTableComboBox.h
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.h
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.h
  qMRMLMatrixWidget.h
  qMRMLNodeComboBox.h
  qMRMLNodeFactory.h
  qMRMLRangeWidget.h
  qMRMLROIWidget.h
  qMRMLSceneCategoryModel.h
  qMRMLSceneColorTableModel.h
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.h
  qMRMLSceneTransformModel.h
  qMRMLSceneTreeModel.h
  qMRMLSliceControllerWidget.h
  qMRMLSliceControllerWidget_p.h
  qMRMLSliceInformationWidget.h
  qMRMLSliceInformationWidget_p.h
  qMRMLSliceWidget.h
  qMRMLSliceWidget_p.h
  qMRMLSlicesControllerToolBar.h
  qMRMLSortFilterProxyModel.h
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewsControllerWidget.h
  qMRMLThreeDViewsControllerWidget_p.h
  qMRMLTransformSliders.h
  qMRMLTreeWidget.h
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.h
  qMRMLWindowLevelWidget.h
  )

# UI files
SET(qMRMLWidgets_UI_SRCS
  Resources/UI/qMRMLEventLoggerWidget.ui
  Resources/UI/qMRMLROIWidget.ui
  Resources/UI/qMRMLSceneFactoryWidget.ui
  Resources/UI/qMRMLSliceControllerWidget.ui
  Resources/UI/qMRMLSliceInformationWidget.ui
  Resources/UI/qMRMLSliceWidget.ui
  Resources/UI/qMRMLSlicesControllerToolBar.ui
  Resources/UI/qMRMLThreeDViewsControllerWidget.ui
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
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  ${MRMLDisplayableManager_SOURCE_DIR}
  ${MRMLDisplayableManager_BINARY_DIR}
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
  MRMLLogic
  MRMLDisplayableManager
  )

TARGET_LINK_LIBRARIES(${lib_name} ${qMRMLWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)


# --------------------------------------------------------------------------
# PythonQt wrapping
#
IF(qMRMLWidgets_WRAP_PYTHON)
  ctkMacroWrapPythonQt("org.slicer.libs" ${lib_name}
    KIT_PYTHONQT_SRCS "${qMRMLWidgets_SRCS}" FALSE)
  ADD_LIBRARY(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
  TARGET_LINK_LIBRARIES(${lib_name}PythonQt ${lib_name})
  IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    SET_TARGET_PROPERTIES(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
  ENDIF()
ENDIF()

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
