project(VolumeRenderingLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  ${VolumeRenderingMRML_SOURCE_DIR}
  ${VolumeRenderingMRML_BINARY_DIR}
  ${VolumeRenderingReplacements_SOURCE_DIR}
  ${VolumeRenderingReplacements_BINARY_DIR}
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerVolumeRenderingLogic.cxx
  vtkMRMLVolumeRenderingDisplayableManager.cxx
  vtkSlicerVolumeRenderingLogic.h
  vtkMRMLVolumeRenderingDisplayableManager.h
  )

# Additional Target libraries
SET(module_logic_target_libraries
  VolumeRenderingMRML
  VolumeRenderingReplacements
  )


SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
