
#-----------------------------------------------------------------------
set(MODULE_NAME "SceneViews") # Do not use 'project()'
set(MODULE_TITLE "Scene Views")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/GUI
  ${CMAKE_CURRENT_BINARY_DIR}/GUI
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

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

set(MODULE_SRCS
  ${GUI_SRCS}
  qSlicerSceneViewsModule.cxx
  qSlicerSceneViewsModule.h
  )

set(MODULE_MOC_SRCS
  qSlicerSceneViewsModule.h
  GUI/qMRMLSceneViewsModel.h
  GUI/qMRMLSceneViewsTreeView.h
  GUI/qSlicerSceneViewsModuleWidget.h
  GUI/qSlicerSceneViewsModuleDialog.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerSceneViewsModule.ui
  )

set(MODULE_RESOURCES
  Resources/qSlicerSceneViewsModule.qrc
  )

set(MODULE_TARGET_LIBRARIES
  qSlicerBaseQTCore
  qMRMLWidgets
  vtkSlicerSceneViewsModuleLogic
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing/Cxx)
endif()
