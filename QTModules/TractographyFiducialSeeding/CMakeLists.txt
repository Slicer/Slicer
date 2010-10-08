# Module name (usually identique to the directory name)
SET(qt_module_name "TractographyFiducialSeeding")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${vtkSlicerTractographyModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyModuleLogic_BINARY_DIR}
  )

# Source files
SET(qt_module_SRCS
  qSlicerTractographyFiducialSeedingModule.cxx
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.cxx
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerTractographyFiducialSeedingModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerTractographyModuleLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerTractographyFiducialSeedingModule.qrc
)

Slicer_build_qtmodule(
  NAME ${qt_module_name}
  TITLE TractographyFiducialSeeding
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)


