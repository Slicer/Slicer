project(qMRMLWidgets)


#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()


if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)

# CMake Macros
include(CMake/qCTKParseArgumentsMacro.cmake)
include(CMake/qCTKDesignerPluginMacros.cmake)

# CMake module
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${qMRMLWidgets_SOURCE_DIR}/CMake")

#-----------------------------------------------------------------------------
# Sources
#
set(qMRMLWidgets_SRCS
  qMRMLActionSignalMapper.cxx
  qMRMLActionSignalMapper.h
  qMRMLCheckableNodeComboBox.cxx
  qMRMLCheckableNodeComboBox.h
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
  qMRMLEventBrokerConnection.cxx
  qMRMLEventBrokerConnection.h
  qMRMLEventBrokerWidget.cxx
  qMRMLEventBrokerWidget.h
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
  qMRMLSortFilterModelHierarchyProxyModel.cxx
  qMRMLSortFilterModelHierarchyProxyModel.h
  qMRMLSortFilterProxyModel.cxx
  qMRMLSortFilterProxyModel.h
  qMRMLThreeDView.cxx
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewControllerWidget.cxx
  qMRMLThreeDViewControllerWidget.h
  qMRMLThreeDViewControllerWidget_p.h
  qMRMLThreeDViewsControllerWidget.cxx
  qMRMLThreeDViewsControllerWidget.h
  qMRMLThreeDViewsControllerWidget_p.h
  qMRMLThreeDWidget.cxx
  qMRMLThreeDWidget.h
  qMRMLTransformSliders.cxx
  qMRMLTransformSliders.h
  qMRMLTreeView.cxx
  qMRMLTreeView.h
  qMRMLTreeView_p.h
  qMRMLUtils.cxx
  qMRMLUtils.h
  qMRMLVolumeInfoWidget.cxx
  qMRMLVolumeInfoWidget.h
  qMRMLVolumePropertyNodeWidget.cxx
  qMRMLVolumePropertyNodeWidget.h
  qMRMLVolumeThresholdWidget.cxx
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.cxx
  qMRMLWidget.h
  qMRMLWindowLevelWidget.cxx
  qMRMLWindowLevelWidget.h
  )

# Headers that should run through moc
set(qMRMLWidgets_MOC_SRCS
  qMRMLActionSignalMapper.h
  qMRMLCheckableNodeComboBox.h
  qMRMLClipNodeWidget.h
  qMRMLCollapsibleButton.h
  qMRMLColorListView.h
  qMRMLColorPickerWidget.h
  qMRMLColorTableComboBox.h
  qMRMLColorTableView.h
  qMRMLColorModel.h
  qMRMLDisplayNodeWidget.h
  qMRMLEventBrokerConnection.h
  qMRMLEventBrokerWidget.h
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
  qMRMLSliceControllerWidget.h
  qMRMLSliceControllerWidget_p.h
  qMRMLSliceInformationWidget.h
  qMRMLSliceInformationWidget_p.h
  qMRMLSliceWidget.h
  qMRMLSliceWidget_p.h
  qMRMLSlicesControllerToolBar.h
  qMRMLSortFilterModelHierarchyProxyModel.h
  qMRMLSortFilterProxyModel.h
  qMRMLThreeDView.h
  qMRMLThreeDView_p.h
  qMRMLThreeDViewControllerWidget.h
  qMRMLThreeDViewsControllerWidget.h
  qMRMLThreeDViewsControllerWidget_p.h
  qMRMLThreeDWidget.h
  qMRMLTransformSliders.h
  qMRMLTreeView.h
  qMRMLTreeView_p.h
  qMRMLVolumeInfoWidget.h
  qMRMLVolumePropertyNodeWidget.h
  qMRMLVolumeThresholdWidget.h
  qMRMLWidget.h
  qMRMLWindowLevelWidget.h
  )

# UI files
set(qMRMLWidgets_UI_SRCS
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
  Resources/UI/qMRMLThreeDViewControllerWidget.ui
  Resources/UI/qMRMLThreeDViewsControllerWidget.ui
  Resources/UI/qMRMLTransformSliders.ui
  Resources/UI/qMRMLVolumeInfoWidget.ui
  Resources/UI/qMRMLVolumePropertyNodeWidget.ui
  Resources/UI/qMRMLVolumeThresholdWidget.ui
  Resources/UI/qMRMLWindowLevelWidget.ui
  )

set(qMRMLWidgets_QRC_SRCS
  Resources/qMRMLWidgets.qrc
  )

QT4_WRAP_CPP(qMRMLWidgets_MOC_CXX ${qMRMLWidgets_MOC_SRCS})
QT4_WRAP_UI(qMRMLWidgets_UI_CXX ${qMRMLWidgets_UI_SRCS})
QT4_ADD_RESOURCES(qMRMLWidgets_QRC_CXX ${qMRMLWidgets_QRC_SRCS})

source_group("Resources" FILES
  ${qMRMLWidgets_UI_SRCS}
  ${qMRMLWidgets_QRC_SRCS}
  )

source_group("Generated" FILES
  ${qMRMLWidgets_MOC_CXX}
  ${qMRMLWidgets_UI_CXX}
  ${qMRMLWidgets_QRC_CXX}
  )

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_INCLUDE_DIRS}
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
  ${qMRMLWidgets_SRCS}
  ${qMRMLWidgets_MOC_CXX}
  ${qMRMLWidgets_UI_CXX}
  ${qMRMLWidgets_QRC_CXX}
  )

set(qMRMLWidgets_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  ${CTK_EXTERNAL_LIBRARIES}
  MRML
  MRMLLogic
  MRMLDisplayableManager
  )

target_link_libraries(${lib_name} ${qMRMLWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()


# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------
if(qMRMLWidgets_WRAP_PYTHON)
  set(KIT_PYTHONQT_SRCS) # Clear variable
  ctkMacroWrapPythonQt("org.slicer.libs" ${lib_name}
    KIT_PYTHONQT_SRCS "${qMRMLWidgets_SRCS}" FALSE)
  add_library(${lib_name}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
  list(APPEND targets_to_export ${lib_name}PythonQt)
  target_link_libraries(${lib_name}PythonQt ${lib_name})
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set_target_properties(${lib_name}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
  endif()
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
# QtDesigner Plugins
# --------------------------------------------------------------------------
add_subdirectory(Plugins)

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
