# Module name (usually identique to the directory name)
SET(qt_module_name "TractographyFiducialSeeding")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT")

# Source files
SET(qt_module_SRCS
  qSlicerTractographyFiducialSeedingModule.cxx
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.cxx
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerTractographyFiducialSeedingModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  #${KWWidgets_LIBRARIES}
  #${ITK_LIBRARIES}
  #CommandLineModule
  #SlicerTractographyDisplay
  #SlicerTractographyFiducialSeeding
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerTractographyFiducialSeedingModule.qrc
)

Slicer3_build_qtmodule(
  NAME ${qt_module_name}
  TITLE TractographyFiducialSeeding
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )
