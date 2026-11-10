project(vtkSlicerSubjectHierarchyModuleLogic)

set(classes vtkSlicerSubjectHierarchyModuleLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSubjectHierarchyModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SubjectHierarchyModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_SUBJECTHIERARCHY_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
