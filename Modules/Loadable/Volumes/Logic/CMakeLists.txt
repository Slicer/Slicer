vtk_module_find_package(PACKAGE RapidJSON PRIVATE_IF_SHARED)

set(classes
  vtkSlicerVolumesLogic
  vtkSlicerVolumesReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerVolumesModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::VolumesModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_VOLUMES_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::VolumesModuleLogic PRIVATE RapidJSON)
