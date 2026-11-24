set(classes vtkMRMLSequenceBrowserNode)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSequencesModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SequencesModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_SEQUENCES_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
