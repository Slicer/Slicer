set(classes
  vtkSlicerSequencesLogic
  vtkSlicerSequencesReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSequencesModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SequencesModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_SEQUENCES_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
