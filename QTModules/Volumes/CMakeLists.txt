# Module name (usually identique to the directory name)
SET(qt_module_name "Volumes")

SET(qt_module_title "Volumes")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMES_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  )

# Source files
SET(qt_module_SRCS
  qSlicerLabelMapVolumeDisplayWidget.cxx
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.cxx
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumesIO.cxx
  qSlicerVolumesIO.h
  qSlicerVolumesIOOptionsWidget.cxx
  qSlicerVolumesIOOptionsWidget.h
  qSlicerVolumesModule.cxx
  qSlicerVolumesModule.h
  qSlicerVolumesModuleWidget.cxx
  qSlicerVolumesModuleWidget.h
  qSlicerVolumeDisplayWidget.cxx
  qSlicerVolumeDisplayWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumesIO.h
  qSlicerVolumesIOOptionsWidget.h
  qSlicerVolumesModule.h
  qSlicerVolumesModuleWidget.h
  qSlicerVolumeDisplayWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerLabelMapVolumeDisplayWidget.ui
  Resources/UI/qSlicerScalarVolumeDisplayWidget.ui
  Resources/UI/qSlicerVolumesIOOptionsWidget.ui
  Resources/UI/qSlicerVolumesModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerVolumesModuleLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerVolumesModule.qrc
)

Slicer_build_qtmodule(
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
  #add_subdirectory(Testing)
endif(BUILD_TESTING)
