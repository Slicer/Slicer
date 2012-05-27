
#-----------------------------------------------------------------------------
set(MODULE_NAME "Measurements") # Do not use 'project()'
set(MODULE_TITLE "${MODULE_NAME}")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

set(MODULE_SRCS
  qSlicerMeasurementsModule.cxx
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModuleWidget.cxx
  qSlicerMeasurementsModuleWidget.h
  Widgets/qSlicerMeasurementsAngleWidget.cxx
  Widgets/qSlicerMeasurementsAngleWidget.h
  Widgets/qSlicerMeasurementsRulerWidget.cxx
  Widgets/qSlicerMeasurementsRulerWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModuleWidget.h
  Widgets/qSlicerMeasurementsAngleWidget.h
  Widgets/qSlicerMeasurementsRulerWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerMeasurementsModule.ui
  Resources/UI/qSlicerMeasurementsAngleWidget.ui
  Resources/UI/qSlicerMeasurementsRulerWidget.ui
)

set(MODULE_RESOURCES
  Resources/qSlicerMeasurementsModule.qrc
)

set(MODULE_TARGET_LIBRARIES
  ""
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
