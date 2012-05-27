
#-----------------------------------------------------------------------------
set(MODULE_NAME "ViewControllers") # Do not use 'project()'
set(MODULE_TITLE "View Controllers")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  )

set(MODULE_SRCS
  qSlicerViewControllersModule.cxx
  qSlicerViewControllersModule.h
  qSlicerViewControllersModuleWidget.cxx
  qSlicerViewControllersModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerViewControllersModule.h
  qSlicerViewControllersModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerViewControllersModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  )

set(MODULE_RESOURCES
  Resources/qSlicerViewControllersModule.qrc
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
