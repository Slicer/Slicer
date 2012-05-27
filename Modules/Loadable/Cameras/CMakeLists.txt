
#-----------------------------------------------------------------------------
set(MODULE_NAME "Cameras") # Do not use 'project()'
set(MODULE_TITLE "${MODULE_NAME}")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  #${CMAKE_CURRENT_SOURCE_DIR}/MRML
  #${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  #${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  #${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

set(MODULE_SRCS
  qSlicerCamerasModule.cxx
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.cxx
  qSlicerCamerasModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerCamerasModuleLogic
  )

set(MODULE_RESOURCES
  Resources/qSlicerCamerasModule.qrc
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
