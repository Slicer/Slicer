
# Module name (usually identique to the directory name)
SET(qt_module_name "Measurements")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

# Source files
SET(qt_module_SRCS
  qSlicerMeasurementsModule.cxx
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModuleWidget.cxx
  qSlicerMeasurementsModuleWidget.h
  Widgets/qSlicerMeasurementsAngleWidget.cxx
  Widgets/qSlicerMeasurementsAngleWidget.h
  Widgets/qSlicerMeasurementsRulerWidget.cxx
  Widgets/qSlicerMeasurementsRulerWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModuleWidget.h
  Widgets/qSlicerMeasurementsAngleWidget.h
  Widgets/qSlicerMeasurementsRulerWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerMeasurementsModule.ui
  Resources/UI/qSlicerMeasurementsAngleWidget.ui
  Resources/UI/qSlicerMeasurementsRulerWidget.ui
)

# Resources
SET(qt_module_resources
  Resources/qSlicerMeasurementsModule.qrc
)

# Additional Target libraries
SET(qt_module_target_libraries
  ""
  )

Slicer_build_qtmodule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )
