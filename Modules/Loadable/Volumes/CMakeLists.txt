# Module name (usually identique to the directory name)
set(qt_module_name "Volumes")

# Build module sub libraries
#add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Widgets)

set(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMES_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

# Source files
set(qt_module_SRCS
  qSlicerVolumesIO.cxx
  qSlicerVolumesIO.h
  qSlicerVolumesIOOptionsWidget.cxx
  qSlicerVolumesIOOptionsWidget.h
  qSlicerVolumesModule.cxx
  qSlicerVolumesModule.h
  qSlicerVolumesModuleWidget.cxx
  qSlicerVolumesModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerVolumesIO.h
  qSlicerVolumesIOOptionsWidget.h
  qSlicerVolumesModule.h
  qSlicerVolumesModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumesIOOptionsWidget.ui
  Resources/UI/qSlicerVolumesModule.ui
  )

# Additional Target libraries
set(qt_module_target_libraries
  qSlicerVolumesModuleWidgets
  vtkSlicerVolumesModuleLogic
  )

# Resources
set(qt_module_resources
  Resources/qSlicerVolumesModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
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
