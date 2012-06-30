
#-----------------------------------------------------------------------
set(MODULE_NAME SceneViews)
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
  GUI/qMRML${MODULE_NAME}Model.cxx
  GUI/qMRML${MODULE_NAME}Model.h
  GUI/qMRML${MODULE_NAME}TreeView.cxx
  GUI/qMRML${MODULE_NAME}TreeView.h
  GUI/qSlicer${MODULE_NAME}ModuleWidget.cxx
  GUI/qSlicer${MODULE_NAME}ModuleWidget.h
  GUI/qSlicer${MODULE_NAME}ModuleDialog.cxx
  GUI/qSlicer${MODULE_NAME}ModuleDialog.h
  )

set(MODULE_SRCS
  ${GUI_SRCS}
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  GUI/qMRML${MODULE_NAME}Model.h
  GUI/qMRML${MODULE_NAME}TreeView.h
  GUI/qSlicer${MODULE_NAME}ModuleWidget.h
  GUI/qSlicer${MODULE_NAME}ModuleDialog.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}Module.ui
  )

set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

set(MODULE_TARGET_LIBRARIES
  qSlicerBaseQTCore
  qMRMLWidgets
  vtkSlicer${MODULE_NAME}ModuleLogic
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
