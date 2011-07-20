# Module name (usually identique to the directory name)
set(qt_module_name "TractographyFiducialSeeding")
set(qt_module_title "Tractography Fiducial Seeding")

add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHYFIDUCIALSEEDING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${vtkSlicerTractographyModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyModuleLogic_BINARY_DIR}
  )

# Source files
set(qt_module_SRCS
  qSlicerTractographyFiducialSeedingModule.cxx
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.cxx
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerTractographyFiducialSeedingModule.h
  qSlicerTractographyFiducialSeedingModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerTractographyFiducialSeedingModule.ui
)

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerTractographyModuleLogic
  vtkSlicerAnnotationsModuleMRML
  )

# Resources
set(qt_module_resources
  Resources/qSlicerTractographyFiducialSeedingModule.qrc
)

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
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
endif()
