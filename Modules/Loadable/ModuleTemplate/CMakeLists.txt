
#-----------------------------------------------------------------------------
set(MODULE_NAME "ModuleTemplate") # Do not use 'project()'
set(MODULE_TITLE "Module Template")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(MODULE_SRCS
  qSlicerModuleTemplateModule.cxx
  qSlicerModuleTemplateModule.h
  qSlicerModuleTemplateModuleWidget.cxx
  qSlicerModuleTemplateModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerModuleTemplateModule.h
  qSlicerModuleTemplateModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerModuleTemplateModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerModuleTemplateModuleLogic
  )

set(MODULE_RESOURCES
  Resources/qSlicerModuleTemplateModule.qrc
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
