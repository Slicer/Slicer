list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
vtk_module_find_package(PACKAGE JsonCpp PRIVATE_IF_SHARED)

set(classes
  vtkMRMLCPURayCastVolumeRenderingDisplayNode
  vtkMRMLGPURayCastVolumeRenderingDisplayNode
  vtkMRMLMultiVolumeRenderingDisplayNode
  vtkMRMLShaderPropertyNode
  vtkMRMLShaderPropertyStorageNode
  vtkMRMLVolumePropertyJsonStorageNode
  vtkMRMLVolumePropertyNode
  vtkMRMLVolumePropertyStorageNode
  vtkMRMLVolumeRenderingDisplayNode
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerVolumeRenderingModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::VolumeRenderingModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_VOLUMERENDERING_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_include(Slicer::VolumeRenderingModuleMRML PRIVATE ${JsonCpp_INCLUDE_DIRS})
vtk_module_link(Slicer::VolumeRenderingModuleMRML PRIVATE ${JsonCpp_LIBRARY})
