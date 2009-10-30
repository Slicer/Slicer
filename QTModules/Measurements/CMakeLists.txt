# Module name (usually identique to the directory name)
SET(qSlicerModule_name "Measurements")

# Additional directories to include
SET(qSlicerModule_include_directories
  #${CommandLineModule_SOURCE_DIR}
  #${CommandLineModule_BINARY_DIR}
  #${SlicerTractographyFiducialSeeding_SOURCE_DIR}
  #${SlicerTractographyFiducialSeeding_BINARY_DIR}
  #${SlicerTractographyDisplay_SOURCE_DIR}
  #${SlicerTractographyDisplay_BINARY_DIR}
  )

# Source files
SET(qSlicerModule_SRCS
  qSlicerMeasurementsModule.h
  qSlicerMeasurementsModule.cxx
  )

# Headers that should run through moc
SET(qSlicerModule_MOC_SRCS
  qSlicerMeasurementsModule.h
  )
  
# UI files
SET(qSlicerModule_UI_SRCS
  Resources/UI/qSlicerMeasurementsModule.ui
)

# Target libraries
SET(qSlicerModule_target_libraries ""
  #${KWWidgets_LIBRARIES}
  #${ITK_LIBRARIES}
  #CommandLineModule
  #SlicerTractographyDisplay
  #SlicerTractographyFiducialSeeding
  )


Slicer3_build_qtmodule( 
  NAME ${qSlicerModule_name}
  INCLUDE_DIRECTORIES ${qSlicerModule_include_directories}
  SRCS ${qSlicerModule_SRCS}
  MOC_SRCS ${qSlicerModule_MOC_SRCS}
  UI_SRCS ${qSlicerModule_UI_SRCS}
  TARGET_LIBRARIES ${qSlicerModule_target_libraries}
  NO_RESOURCES
  )
