set(classes vtkSlicerUnitsLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerUnitsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::UnitsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_UNITS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
