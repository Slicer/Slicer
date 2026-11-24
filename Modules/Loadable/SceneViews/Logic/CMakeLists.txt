set(classes vtkSlicerSceneViewsModuleLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSceneViewsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SceneViewsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_SCENEVIEWS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
