set(classes vtkSlicerTemplateKeyLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTemplateKeyModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TemplateKeyModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_TEMPLATEKEY_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
