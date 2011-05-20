PROJECT(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

SET(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
SET(module_mrml_export_directive "VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT")

# Additional directories to include
SET(module_mrml_include_directories
  )

# Source files
SET(module_mrml_SRCS
  vtkMRMLVolumeRenderingDisplayNode.cxx
  vtkMRMLVolumeRenderingDisplayNode.h
  vtkMRMLVolumeRenderingScenarioNode.cxx
  vtkMRMLVolumeRenderingScenarioNode.h
  )

# Additional Target libraries
SET(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  vtkSlicerAnnotationModuleMRML
  )

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )
