project(vtkSlicerEMSegmentModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_SLICER_EMSEGMENT_MODULE_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  ${EMSegmentAlgorithm_SOURCE_DIR}
  ${EMSegmentAlgorithm_BINARY_DIR}
  ${EMSegmentMRML_SOURCE_DIR}
  ${EMSegmentMRML_BINARY_DIR}
  ${EMSegmentRegistration_SOURCE_DIR}
  ${EMSegmentRegistration_BINARY_DIR}
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerEMSegmentLogic.cxx
  )

# Additional Target libraries
SET(module_logic_target_libraries
  EMSegmentMRML
  EMSegmentRegistration
  )


SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
