set(classes vtkSlicerCamerasModuleLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerCamerasModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::CamerasModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_CAMERAS_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
