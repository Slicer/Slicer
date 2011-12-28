project(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

set(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
set(module_mrml_export_directive "VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT")

# Additional directories to include
set(module_mrml_include_directories
  )

# Source files
set(module_mrml_SRCS
  vtkMRMLVolumePropertyNode.cxx
  vtkMRMLVolumePropertyStorageNode.cxx
  vtkMRMLVolumeRenderingDisplayNode.cxx
  vtkMRMLVolumeRenderingDisplayNode.h
  vtkMRMLVolumeRenderingScenarioNode.cxx
  vtkMRMLVolumeRenderingScenarioNode.h
  )

# Additional Target libraries
set(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  vtkRendering
  vtkSlicerAnnotationsModuleMRML
  )

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )
