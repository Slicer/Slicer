set(classes vtkMRMLMarkupsTestLineNode)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTemplateKeyModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TemplateKeyModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_TEMPLATEKEY_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
