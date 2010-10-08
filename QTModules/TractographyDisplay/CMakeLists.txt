# Module name (usually identique to the directory name)
SET(qt_module_name "TractographyDisplay")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHYDISPLAY_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${vtkSlicerTractographyDisplayModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyDisplayModuleLogic_BINARY_DIR}
  )

# Source files
SET(qt_module_SRCS
  qSlicerFiberBundleIO.cxx
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.cxx
  qSlicerTractographyDisplayModule.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.h
  )

# UI files
SET(qt_module_UI_SRCS
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerTractographyDisplayModuleLogic
  )

# Resources
SET(qt_module_resources
)

Slicer_build_qtmodule(
  NAME ${qt_module_name}
  TITLE TractographyDisplay
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif(BUILD_TESTING)


