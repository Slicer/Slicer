
# Module name (usually identique to the directory name)
set(qt_module_name "Tractography")
set(qt_module_title "Tractography")

# Build module sub libraries
add_subdirectory(MRML)
add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_TRACTOGRAPHY_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  )

# Source files
set(qt_module_SRCS
  qSlicerTractographyModule.cxx
  qSlicerTractographyModule.h
  qSlicerTractographyModuleWidget.cxx
  qSlicerTractographyModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerTractographyModule.h
  qSlicerTractographyModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerTractographyModule.ui
  )

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerTractographyModuleMRML
  vtkSlicerTractographyModuleLogic
  )

# Resources
set(qt_module_resources
  Resources/qSlicerTractographyModule.qrc
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
