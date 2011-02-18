
# Add subdirectories
ADD_SUBDIRECTORY(MRML)
ADD_SUBDIRECTORY(Logic)
ADD_SUBDIRECTORY(VolumeRenderingReplacements)

# Module name (usually identique to the directory name)
SET(qt_module_name "VolumeRendering")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/VolumeRenderingReplacements
  ${CMAKE_CURRENT_BINARY_DIR}/VolumeRenderingReplacements
  )

# Source files
SET(qt_module_SRCS
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.cxx
  qSlicerVolumeRenderingModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  VolumeRenderingLogic
  VolumeRenderingMRML
  VolumeRenderingReplacements
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerVolumeRenderingModule.qrc
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
  #add_subdirectory(Testing)
endif(BUILD_TESTING)
