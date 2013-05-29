project(qMRMLWidgets)


#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()


if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)

include(CheckIncludeFiles)
check_include_files("stdint.h" HAVE_STDINT_H)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/qMRMLWidgetsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qMRMLWidgetsConfigure.h
  )

# CMake module
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MRMLWidgets_SOURCE_DIR}/CMake")

#-----------------------------------------------------------------------------
# Sources
#
set(MRMLWidgets_SRCS
  qMRMLCaptureToolBar.cxx
  qMRMLCaptureToolBar.h
  qMRMLChartView.cxx
  qMRMLChartView.h
  qMRMLChartView_p.h
  qMRMLChartViewControllerWidget.cxx
  qMRMLChartViewControllerWidget.h
  qMRMLChartViewControllerWidget_p.h
  qMRMLChartWidget.cxx
  qMRMLChartWidget.h
  qMRMLCheckableNodeComboBox.cxx
  qMRMLCheckableNodeComboBox.h
  qMRMLClipNodeWidget.cxx
  qMRMLClipNodeWidget.h
  qMRMLCollapsibleButton.cxx
  qMRMLCollapsibleButton.h
  qMRMLColors.cxx
  qMRMLColors.h
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
  qMRMLCoordinatesWidget.cxx
  qMRMLCoordinatesWidget.h
  qMRMLDisplayNodeWidget.cxx
  qMRMLDisplayNodeWidget.h
  qMRMLDisplayNodeViewComboBox.cxx
  qMRMLDisplayNodeViewComboBox.h
  qMRMLLabelComboBox.h
  qMRMLLabelComboBox.cxx
  qMRMLEventBrokerConnection.cxx
  qMRMLEventBrokerConnection.h
  qMRMLEventBrokerWidget.cxx
  qMRMLEventBrokerWidget.h
  qMRMLEventLogger.cxx
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.cxx
  qMRMLEventLoggerWidget.h
  qMRMLItemDelegate.cxx
  qMRMLItemDelegate.h
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
  qMRMLNodeComboBoxDelegate.cxx
  qMRMLNodeComboBoxDelegate.h
  qMRMLNodeComboBoxMenuDelegate.cxx
  qMRMLNodeComboBoxMenuDelegate.h
  qMRMLNodeComboBox_p.h
  qMRMLNodeFactory.cxx
  qMRMLNodeFactory.h
  qMRMLRangeWidget.cxx
  qMRMLRangeWidget.h
  qMRMLROIWidget.cxx
  qMRMLROIWidget.h
  qMRMLScalarInvariantComboBox.cxx
  qMRMLScalarInvariantComboBox.h
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
  qMRMLSceneViewMenu.cxx
  qMRMLSceneViewMenu.h
  qMRMLSceneViewMenu_p.h
  qMRMLSceneTransformModel.cxx
  qMRMLSceneTransformModel.h
  qMRMLSceneHierarchyModel.cxx
  qMRMLSceneHierarchyModel.h
  qMRMLSceneDisplayableModel.cxx
  qMRMLSceneDisplayableModel.h
  qMRMLScreenShotDialog.cxx
  qMRMLScreenShotDialog.h
  qMRMLSliceControllerWidget.cxx
  qMRMLSliceControllerWidget.h
  qMRMLSliceControllerWidget_p.h
  qMRMLSliceInformationWidget.cxx
  qMRMLSliceInformationWidget.h
  qMRMLSliceInformationWidget_p.h
  qMRMLSliceView.cxx
  qMRMLSliceView.h
  qMRMLSliceView_p.h
  qMRMLSliceWidget.cxx
  qMRMLSliceWidget.h
  qMRMLSliceWidget_p.h
  qMRMLSliderWidget.cxx
  qMRMLSliderWidget.h
  qMRMLSortFilterHierarchyProxyModel.cxx
  qMRMLSortFilterHierarchyProxyModel.h
  qMRMLSortFilterProxyModel.cxx
  qMRMLSortFilterProxyModel.h
  qMRMLSpinBox.cxx
  qMRMLSpinBox.h
  qMRMLThreeDView.cxx
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewControllerWidget.cxx
  qMRMLThreeDViewControllerWidget.h
  qMRMLThreeDViewControllerWidget_p.h
  qMRMLThreeDWidget.cxx
  qMRMLThreeDWidget.h
  qMRMLTransformSliders.cxx
  qMRMLTransformSliders.h
  qMRMLTreeView.cxx
  qMRMLTreeView.h
  qMRMLTreeView_p.h
  qMRMLUtils.cxx
  qMRMLUtils.h
  qMRMLViewControllerBar.cxx
  qMRMLViewControllerBar.h
  qMRMLViewControllerBar_p.h
  qMRMLVolumeInfoWidget.cxx
  qMRMLVolumeInfoWidget.h
  qMRMLVolumeThresholdWidget.cxx
  qMRMLVolumeThresholdWidget.h
  qMRMLVolumeWidget.cxx
  qMRMLVolumeWidget.h
  qMRMLWidget.cxx
  qMRMLWidget.h
  qMRMLWindowLevelWidget.cxx
  qMRMLWindowLevelWidget.h
  )

# Headers that should run through moc
set(MRMLWidgets_MOC_SRCS
  qMRMLCaptureToolBar.h
  qMRMLChartView.h
  qMRMLChartView_p.h
  qMRMLChartViewControllerWidget.h
  qMRMLChartViewControllerWidget_p.h
  qMRMLChartWidget.h
  qMRMLCheckableNodeComboBox.h
  qMRMLClipNodeWidget.h
  qMRMLCollapsibleButton.h
  qMRMLColorListView.h
  qMRMLColorPickerWidget.h
  qMRMLColorTableComboBox.h
  qMRMLColorTableView.h
  qMRMLColorModel.h
  qMRMLCoordinatesWidget.h
  qMRMLDisplayNodeViewComboBox.h
  qMRMLDisplayNodeWidget.h
  qMRMLEventBrokerConnection.h
  qMRMLEventBrokerWidget.h
  qMRMLEventLogger.h
  qMRMLEventLoggerWidget.h
  qMRMLItemDelegate.h
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
  qMRMLNodeComboBoxDelegate.h
  qMRMLNodeComboBoxMenuDelegate.h
  qMRMLNodeFactory.h
  qMRMLRangeWidget.h
  qMRMLROIWidget.h
  qMRMLScalarInvariantComboBox.h
  qMRMLSceneCategoryModel.h
  qMRMLSceneColorTableModel.h
  qMRMLSceneFactoryWidget.h
  qMRMLSceneModel.h
  qMRMLSceneModelHierarchyModel.h
  qMRMLSceneViewMenu.h
  qMRMLSceneViewMenu_p.h
  qMRMLSceneTransformModel.h
  qMRMLSceneHierarchyModel.h
  qMRMLSceneDisplayableModel.h
  qMRMLScreenShotDialog.h
  qMRMLSliceControllerWidget.h
  qMRMLSliceControllerWidget_p.h
  qMRMLSliceInformationWidget.h
  qMRMLSliceInformationWidget_p.h
  qMRMLSliceView.h
  qMRMLSliceView_p.h
  qMRMLSliceWidget.h
  qMRMLSliceWidget_p.h
  qMRMLSliderWidget.h
  qMRMLSortFilterHierarchyProxyModel.h
  qMRMLSortFilterProxyModel.h
  qMRMLSpinBox.h
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewControllerWidget.h
  qMRMLThreeDWidget.h
  qMRMLTransformSliders.h
  qMRMLTreeView.h
  qMRMLUtils.h
  qMRMLViewControllerBar.h
  qMRMLViewControllerBar_p.h
  qMRMLVolumeInfoWidget.h
  qMRMLVolumeThresholdWidget.h
  qMRMLVolumeWidget.h
  qMRMLVolumeWidget_p.h
  qMRMLWidget.h
  qMRMLWindowLevelWidget.h
  )

# UI files
set(MRMLWidgets_UI_SRCS
  Resources/UI/qMRMLChartViewControllerWidget.ui
  Resources/UI/qMRMLClipNodeWidget.ui
  Resources/UI/qMRMLColorPickerWidget.ui
  Resources/UI/qMRMLDisplayNodeWidget.ui
  Resources/UI/qMRMLEventLoggerWidget.ui
  Resources/UI/qMRMLModelInfoWidget.ui
  Resources/UI/qMRMLROIWidget.ui
  Resources/UI/qMRMLSceneFactoryWidget.ui
  Resources/UI/qMRMLScreenShotDialog.ui
  Resources/UI/qMRMLSliceControllerWidget.ui
  Resources/UI/qMRMLSliceInformationWidget.ui
  Resources/UI/qMRMLSliceWidget.ui
  Resources/UI/qMRMLThreeDViewControllerWidget.ui
  Resources/UI/qMRMLTransformSliders.ui
  Resources/UI/qMRMLVolumeInfoWidget.ui
  Resources/UI/qMRMLVolumeThresholdWidget.ui
  Resources/UI/qMRMLWindowLevelWidget.ui
  )

configure_file(Resources/jqPlot.qrc.in ${qMRMLWidgets_BINARY_DIR}/Resources/jqPlot.qrc) 
set(MRMLWidgets_QRC_SRCS
  Resources/qMRMLWidgets.qrc
  ${qMRMLWidgets_BINARY_DIR}/Resources/jqPlot.qrc
  )

if(Slicer_USE_QtTesting)
  list(APPEND MRMLWidgets_SRCS
    qMRMLCheckableNodeComboBoxEventPlayer.cpp
    qMRMLCheckableNodeComboBoxEventPlayer.h
    qMRMLNodeComboBoxEventPlayer.cpp
    qMRMLNodeComboBoxEventPlayer.h
    qMRMLNodeComboBoxEventTranslator.cpp
    qMRMLNodeComboBoxEventTranslator.h
    qMRMLTreeViewEventPlayer.cpp
    qMRMLTreeViewEventPlayer.h
    qMRMLTreeViewEventTranslator.cpp
    qMRMLTreeViewEventTranslator.h
    )

  list(APPEND MRMLWidgets_MOC_SRCS
    qMRMLCheckableNodeComboBoxEventPlayer.h
    qMRMLNodeComboBoxEventPlayer.h
    qMRMLNodeComboBoxEventTranslator.h
    qMRMLTreeViewEventPlayer.h
    qMRMLTreeViewEventTranslator.h
    )
endif()

QT4_WRAP_CPP(MRMLWidgets_MOC_CXX ${MRMLWidgets_MOC_SRCS})
QT4_WRAP_UI(MRMLWidgets_UI_CXX ${MRMLWidgets_UI_SRCS})
QT4_ADD_RESOURCES(MRMLWidgets_QRC_CXX ${MRMLWidgets_QRC_SRCS})

source_group("Resources" FILES
  ${MRMLWidgets_UI_SRCS}
  ${MRMLWidgets_QRC_SRCS}
  )

source_group("Generated" FILES
  ${MRMLWidgets_MOC_CXX}
  ${MRMLWidgets_UI_CXX}
  ${MRMLWidgets_QRC_CXX}
  )

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
set(targets_to_export ${lib_name})

add_library(${lib_name}
  ${MRMLWidgets_SRCS}
  ${MRMLWidgets_MOC_CXX}
  ${MRMLWidgets_UI_CXX}
  ${MRMLWidgets_QRC_CXX}
  )

set(MRMLWidgets_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  ${CTK_EXTERNAL_LIBRARIES}
  MRMLLogic
  MRMLDisplayableManager
  )

target_link_libraries(${lib_name} ${MRMLWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()


# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------
if(MRMLWidgets_WRAP_PYTHON)
  ctkMacroBuildLibWrapper(
    NAMESPACE "org.slicer.libs"
    TARGET ${lib_name}
    SRCS "${MRMLWidgets_SRCS}"
    INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    )
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${targets_to_export} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Qt Designer Plugins
# --------------------------------------------------------------------------
add_subdirectory(DesignerPlugins)

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
