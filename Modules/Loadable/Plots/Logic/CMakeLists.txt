set(classes vtkSlicerPlotsLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerPlotsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::PlotsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_PLOTS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
