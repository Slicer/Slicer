#-----------------------------------------------------------------------
#
# SETUP
#
#-----------------------------------------------------------------------
# Module name (usually identique to the directory name)
set(qt_module_name "SceneViews")

set(qt_module_title "Scene Views")

add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/GUI
  ${CMAKE_CURRENT_BINARY_DIR}/GUI
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )


#-----------------------------------------------------------------------
#
# GUI
#
#-----------------------------------------------------------------------
set(GUI_SRCS
  GUI/qMRMLSceneViewsModel.cxx
  GUI/qMRMLSceneViewsModel.h
  GUI/qMRMLSceneViewsTreeView.cxx
  GUI/qMRMLSceneViewsTreeView.h
  GUI/qSlicerSceneViewsModuleWidget.cxx
  GUI/qSlicerSceneViewsModuleWidget.h
  GUI/qSlicerSceneViewsModuleDialog.cxx
  GUI/qSlicerSceneViewsModuleDialog.h
  )

#-----------------------------------------------------------------------
#
# ALL SOURCE FILES TOGETHER
#
#-----------------------------------------------------------------------
set(qt_module_SRCS
  ${GUI_SRCS}
  qSlicerSceneViewsModule.cxx
  qSlicerSceneViewsModule.h
  )

#-----------------------------------------------------------------------
#
# MOC HEADERS
#
#-----------------------------------------------------------------------
set(qt_module_MOC_SRCS
  qSlicerSceneViewsModule.h
  GUI/qMRMLSceneViewsModel.h
  GUI/qMRMLSceneViewsTreeView.h
  GUI/qSlicerSceneViewsModuleWidget.h
  GUI/qSlicerSceneViewsModuleDialog.h
  )

#-----------------------------------------------------------------------
#
# RESOURCES
#
#-----------------------------------------------------------------------
set(qt_module_UI_SRCS
  Resources/UI/qSlicerSceneViewsModule.ui
  )

# Resources
set(qt_module_resources
  Resources/qSlicerSceneViewsModule.qrc
)

#-----------------------------------------------------------------------
#
# LINK TARGETS
#
#-----------------------------------------------------------------------
# Additional Target libraries
set(qt_module_target_libraries
  qSlicerBaseQTCore
  qMRMLWidgets
  vtkSlicerSceneViewsModuleLogic
)

#-----------------------------------------------------------------------
#
# BUILD QTMODULE
#
#-----------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
)

#-----------------------------------------------------------------------
#
# TESTING
#
#-----------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing/Cxx)
endif()

