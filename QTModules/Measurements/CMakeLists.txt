PROJECT(qSlicerMeasurementsModule)

# Module name (usually identique to the directory name)
SET(qt_module_name "Measurements")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT")

# Additional directories to include
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

# Source files
SET(qt_module_SRCS
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModule.cxx
  Widgets/qSlicerMeasurementsAngleWidget.h
  Widgets/qSlicerMeasurementsAngleWidget.cxx
  #qSlicerMeasurementsRulerWidget.h
  #qSlicerMeasurementsRulerWidget.cxx
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerMeasurementsModule.h
  Widgets/qSlicerMeasurementsAngleWidget.h
  )
  
# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerMeasurementsModule.ui
  Resources/UI/qSlicerMeasurementsAngleWidget.ui
)

# Additional Target libraries
SET(qt_module_target_libraries ""
  #${KWWidgets_LIBRARIES}
  #${ITK_LIBRARIES}
  #CommandLineModule
  #SlicerTractographyDisplay
  #SlicerTractographyFiducialSeeding
  )
  
Slicer3_build_qtmodule( 
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  NO_RESOURCES
  )
