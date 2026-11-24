set(classes
  vtkMRMLMultiVolumeNode
  vtkMRMLMultiVolumeDisplayNode
  vtkMRMLMultiVolumeStorageNode
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerMultiVolumeExplorerModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::MultiVolumeExplorerModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
