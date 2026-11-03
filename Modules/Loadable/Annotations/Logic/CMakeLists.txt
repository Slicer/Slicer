set(classes vtkSlicerAnnotationModuleLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerAnnotationsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::AnnotationsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_ANNOTATIONS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
