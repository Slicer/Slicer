
#-----------------------------------------------------------------------------
set(MODULE_NAME "Markups") # Do not use 'project()'
set(MODULE_TITLE "Markups")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(VTKWidgets)
# make sure to add logic before any Qt widgets
add_subdirectory(Logic)
add_subdirectory(MRMLDM)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )


set(MODULE_SRCS
  qSlicerMarkupsIO.cxx
  qSlicerMarkupsIO.h
  qSlicerMarkupsModule.cxx
  qSlicerMarkupsModule.h
  qSlicerMarkupsModuleWidget.cxx
  qSlicerMarkupsModuleWidget.h
  qSlicerMarkupsSettingsPanel.cxx
  qSlicerMarkupsSettingsPanel.h
  )


set(MODULE_MOC_SRCS
  qSlicerMarkupsIO.h
  qSlicerMarkupsModule.h
  qSlicerMarkupsModuleWidget.h
  qSlicerMarkupsSettingsPanel.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerMarkupsModule.ui
  Resources/UI/qSlicerMarkupsSettingsPanel.ui
  )

set(MODULE_TARGET_LIBRARIES
# qSlicerBaseQTCore
# qMRMLWidgets
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleVTKWidgets
  vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicer${MODULE_NAME}ModuleWidgets
  )

set(MODULE_RESOURCES
  Resources/qSlicerMarkupsModule.qrc
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
  add_subdirectory(Testing)
endif()
