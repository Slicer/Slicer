project(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_TRACTOGRAPHY_FIDUCIAL_SEEDING_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
  )

# Source files
set(module_logic_SRCS
  vtkSlicerTractographyFiducialSeedingLogic.h
  vtkSlicerTractographyFiducialSeedingLogic.cxx
  vtkMRMLTractographyFiducialSeedingNode.h
  vtkMRMLTractographyFiducialSeedingNode.cxx
  )

# Additional Target libraries
set(module_logic_target_libraries
  ${ITK_LIBRARIES}
  vtkSlicerAnnotationsModuleMRML
  vtkSlicerTractographyDisplayModuleMRML
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
