set(classes vtkSlicerScalarBarActor)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerColorsModuleVTKWidgetsExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::ColorsModuleVTKWidgets
  EXPORT_MACRO_PREFIX VTK_SLICER_COLORS_VTKWIDGETS
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
