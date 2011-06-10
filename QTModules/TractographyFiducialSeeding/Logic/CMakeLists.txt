project(vtkSlicerTractographyModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_SLICER_TRACTOGRAPHY_MODULE_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerTractographyFiducialSeedingLogic.h
  vtkSlicerTractographyFiducialSeedingLogic.cxx
  vtkMRMLTractographyFiducialSeedingNode.h
  vtkMRMLTractographyFiducialSeedingNode.cxx
  )

# Additional Target libraries
SET(module_logic_target_libraries
  ${ITK_LIBRARIES}
  vtkSlicerAnnotationModuleMRML
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
