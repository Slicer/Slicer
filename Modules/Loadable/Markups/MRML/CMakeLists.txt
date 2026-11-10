vtk_module_find_package(PACKAGE RapidJSON PRIVATE_IF_SHARED)

set(classes
  vtkMRMLMarkupsJsonElement
  vtkMRMLMarkupsJsonStorageNode
  vtkMRMLMarkupsPlaneJsonStorageNode
  vtkMRMLMarkupsROIJsonStorageNode
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerMarkupsModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::MarkupsModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_MARKUPS_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::MarkupsModuleMRML PRIVATE RapidJSON)
