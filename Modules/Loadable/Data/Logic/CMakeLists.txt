set(classes
  vtkSlicerDataModuleLogic
  vtkSlicerSceneReader
  vtkSlicerSceneWriter
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerDataModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::DataModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_DATA_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
