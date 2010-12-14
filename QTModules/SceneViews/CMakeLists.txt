#-----------------------------------------------------------------------
#
# SETUP
#
#-----------------------------------------------------------------------
# Module name (usually identique to the directory name)
SET(qt_module_name "SceneViews")

SET(qt_module_title "SceneViews")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories

  )
  
#-----------------------------------------------------------------------
#
# GUI
#
#-----------------------------------------------------------------------
SET(GUI_SRCS
  GUI/qMRMLSceneViewsModel.cxx
  GUI/qMRMLSceneViewsModel.h
  GUI/qMRMLSceneViewsTreeWidget.cxx
  GUI/qMRMLSceneViewsTreeWidget.h
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
SET(qt_module_SRCS
  ${GUI_SRCS}
  qSlicerSceneViewsModule.cxx
  qSlicerSceneViewsModule.h
  )

#-----------------------------------------------------------------------
#
# MOC HEADERS
#
#-----------------------------------------------------------------------
SET(qt_module_MOC_SRCS
  qSlicerSceneViewsModule.h
  GUI/qMRMLSceneViewsModel.h
  GUI/qMRMLSceneViewsTreeWidget.h
  GUI/qSlicerSceneViewsModuleWidget.h
  GUI/qSlicerSceneViewsModuleDialog.h
  )

#-----------------------------------------------------------------------
#
# RESOURCES
#
#-----------------------------------------------------------------------
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerSceneViewsModule.ui
  Resources/UI/qSlicerSceneViewsModuleDialog.ui
  )
 
# Resources
SET(qt_module_resources
  Resources/qSlicerSceneViewsModule.qrc
)

#-----------------------------------------------------------------------
#
# LINK TARGETS
#
#-----------------------------------------------------------------------
# Additional Target libraries
SET(qt_module_target_libraries
  qSlicerBaseQTCore
  qMRMLWidgets
  MRMLLogic
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
  #ADD_SUBDIRECTORY(Testing/Cxx)
endif(BUILD_TESTING)

