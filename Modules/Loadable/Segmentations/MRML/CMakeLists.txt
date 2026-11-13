project(vtkSlicerSegmentationsModuleMRMLModule)

set(classes vtkMRMLSegmentEditorNode)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSegmentationsModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SegmentationsModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_SEGMENTATIONS_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
