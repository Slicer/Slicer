set(classes vtkSlicerReformatLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerReformatModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::ReformatModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_REFORMAT_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
