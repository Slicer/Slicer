set(classes
  vtkSlicerTextsLogic
  vtkSlicerTextsReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTextsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TextsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_TEXTS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
