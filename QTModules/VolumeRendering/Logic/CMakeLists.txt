project(vtkSlicerVolumeRenderingModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  ${vtkSlicerVolumeRenderingModuleMRML_SOURCE_DIR}
  ${vtkSlicerVolumeRenderingModuleMRML_BINARY_DIR}
  ${VolumeRenderingReplacements_SOURCE_DIR}
  ${VolumeRenderingReplacements_BINARY_DIR}
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerVolumeRenderingLogic.cxx
  vtkSlicerVolumeRenderingLogic.h
  )

# Additional Target libraries
SET(module_logic_target_libraries
  vtkSlicerVolumeRenderingModuleMRML
  VolumeRenderingReplacements
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
