# Module name (usually identique to the directory name)
set(qt_module_name "TractographyDisplay")
set(qt_module_title "Tractography Display")

# Build module sub libraries
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Widgets)

set(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHYDISPLAY_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${vtkSlicerTractographyDisplayModuleLogic_SOURCE_DIR}
  ${vtkSlicerTractographyDisplayModuleLogic_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  )

# Source files
set(qt_module_SRCS
  qSlicerFiberBundleIO.cxx
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.cxx
  qSlicerTractographyDisplayModule.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerFiberBundleIO.h
  qSlicerTractographyDisplayModule.h
  )

# UI files
set(qt_module_UI_SRCS
)

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerTractographyDisplayModuleLogic
  vtkSlicerTractographyDisplayModuleMRML
  qSlicerTractographyDisplayModuleWidgets
  ${MRML_LIBRARIES}
  )

# Resources
set(qt_module_resources
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
