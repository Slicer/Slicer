set(classes
  vtkSlicerMarkupsLogic
  vtkSlicerMarkupsReader
  vtkSlicerAnnotationsReader
  vtkSlicerMarkupsWriter
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerMarkupsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::MarkupsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_MARKUPS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
