set(classes vtkMRMLColorLegendDisplayNode)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerColorsModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::ColorsModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_COLORS_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
