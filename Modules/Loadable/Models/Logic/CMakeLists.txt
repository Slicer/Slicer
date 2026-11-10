set(classes
  vtkSlicerModelsLogic
  vtkSlicerModelsReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerModelsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::ModelsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_MODELS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
