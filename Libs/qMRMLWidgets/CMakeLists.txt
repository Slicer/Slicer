PROJECT(qMRMLWidgets)


#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


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
  qMRMLActionSignalMapper.cxx
  qMRMLActionSignalMapper.h
  qMRMLClipNodeWidget.cxx
  qMRMLClipNodeWidget.h
  qMRMLCollapsibleButton.cxx
  qMRMLCollapsibleButton.h
  qMRMLColorListView.cxx
  qMRMLColorListView.h
  qMRMLColorModel.cxx
  qMRMLColorModel.h
  qMRMLColorPickerWidget.cxx
  qMRMLColorPickerWidget.h
  qMRMLColorTableComboBox.cxx
  qMRMLColorTableComboBox.h
  qMRMLColorTableView.cxx
  qMRMLColorTableView.h
  qMRMLDisplayNodeWidget.cxx
  qMRMLDisplayNodeWidget.h
  qMRMLLabelComboBox.h
  qMRMLLabelComboBox.cxx
  qMRMLEventLogger.cxx
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.cxx
  qMRMLEventLoggerWidget.h
  qMRMLLayoutManager.cxx
  qMRMLLayoutManager.h
  qMRMLLayoutManager_p.h
  qMRMLLayoutWidget.cxx
  qMRMLLayoutWidget.h
  qMRMLLinearTransformSlider.cxx
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.cxx
  qMRMLListWidget.h
  qMRMLMatrixWidget.cxx
  qMRMLMatrixWidget.h
  qMRMLModelInfoWidget.cxx
  qMRMLModelInfoWidget.h
  qMRMLNavigationView.cxx
  qMRMLNavigationView.h
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
  qMRMLSceneModelHierarchyModel.cxx
  qMRMLSceneModelHierarchyModel.h
  qMRMLSceneSnapshotMenu.cxx
  qMRMLSceneSnapshotMenu.h
  qMRMLSceneSnapshotMenu_p.h
  qMRMLSceneTransformModel.cxx
  qMRMLSceneTransformModel.h
  qMRMLSceneDisplayableModel.cxx
  qMRMLSceneDisplayableModel.h
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
  qMRMLVolumeInfoWidget.cxx
  qMRMLVolumeInfoWidget.h
  qMRMLVolumeThresholdWidget.cxx
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.cxx
  qMRMLWidget.h
  qMRMLWindowLevelWidget.cxx
  qMRMLWindowLevelWidget.h
  )

# Headers that should run through moc
SET(qMRMLWidgets_MOC_SRCS
  qMRMLActionSignalMapper.h
  qMRMLClipNodeWidget.h
  qMRMLCollapsibleButton.h
  qMRMLColorListView.h
  qMRMLColorPickerWidget.h
  qMRMLColorTableComboBox.h
  qMRMLColorTableView.h
  qMRMLColorModel.h
  qMRMLDisplayNodeWidget.h
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.h
  qMRMLLabelComboBox.h
  qMRMLLayoutManager.h
  qMRMLLayoutManager_p.h
  qMRMLLayoutWidget.h
  qMRMLLinearTransformSlider.h
  qMRMLListWidget.h
  qMRMLMatrixWidget.h
  qMRMLModelInfoWidget.h
  qMRMLNavigationView.h
  qMRMLNodeComboBox.h
  qMRMLNodeFactory.h
  qMRMLRangeWidget.h
  qMRMLROIWidget.h
  qMRMLSceneCategoryModel.h
  qMRMLSceneColorTableModel.h
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.h
  qMRMLSceneModelHierarchyModel.h
  qMRMLSceneSnapshotMenu.h
  qMRMLSceneSnapshotMenu_p.h
  qMRMLSceneTransformModel.h
  qMRMLSceneDisplayableModel.h
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
  qMRMLVolumeInfoWidget.h
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.h
  qMRMLWindowLevelWidget.h
  )

# UI files
SET(qMRMLWidgets_UI_SRCS
  Resources/UI/qMRMLClipNodeWidget.ui
  Resources/UI/qMRMLColorPickerWidget.ui
  Resources/UI/qMRMLDisplayNodeWidget.ui
  Resources/UI/qMRMLEventLoggerWidget.ui
  Resources/UI/qMRMLModelInfoWidget.ui
  Resources/UI/qMRMLROIWidget.ui
  Resources/UI/qMRMLSceneFactoryWidget.ui
  Resources/UI/qMRMLSliceControllerWidget.ui
  Resources/UI/qMRMLSliceInformationWidget.ui
  Resources/UI/qMRMLSliceWidget.ui
  Resources/UI/qMRMLSlicesControllerToolBar.ui
  Resources/UI/qMRMLThreeDViewsControllerWidget.ui
  Resources/UI/qMRMLTransformSliders.ui
  Resources/UI/qMRMLVolumeInfoWidget.ui
  Resources/UI/qMRMLVolumeThresholdWidget.ui
  Resources/UI/qMRMLWindowLevelWidget.ui
  )

SET(qMRMLWidgets_QRC_SRCS
  Resources/qMRMLWidgets.qrc
  )

QT4_WRAP_CPP(qMRMLWidgets_MOC_CXX ${qMRMLWidgets_MOC_SRCS})
QT4_WRAP_UI(qMRMLWidgets_UI_CXX ${qMRMLWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qMRMLWidgets_QRC_CXX ${qMRMLWidgets_QRC_SRCS})

SOURCE_GROUP("Resources" FILES
  ${qMRMLWidgets_UI_SRCS}
  ${qMRMLWidgets_QRC_SRCS}
  )

SOURCE_GROUP("Generated" FILES
  ${qMRMLWidgets_MOC_CXX}
  ${qMRMLWidgets_UI_CXX}
  ${qMRMLWidgets_QRC_CXX}
  )
  

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

SET(lib_name ${PROJECT_NAME})

ADD_LIBRARY(${lib_name}
  ${qMRMLWidgets_SRCS}
  ${qMRMLWidgets_MOC_CXX}
  ${qMRMLWidgets_UI_CXX}
  ${qMRMLWidgets_QRC_CXX}
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
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)


# --------------------------------------------------------------------------
# PythonQt wrapping
#
IF(qMRMLWidgets_WRAP_PYTHON)
  SET(KIT_PYTHONQT_SRCS) # Clear variable
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

IF(NOT DEFINED ${lib_name}_INSTALL_BIN_DIR)
  SET(${lib_name}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${lib_name}_INSTALL_LIB_DIR)
  SET(${lib_name}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${lib_name}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Sub directories
#
ADD_SUBDIRECTORY(Plugins)
