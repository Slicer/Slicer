project(vtkSlicerTerminologiesModuleLogic)

find_package(RapidJSON REQUIRED)

set(classes
  vtkSlicerTerminologiesReader
  vtkSlicerTerminologiesModuleLogic
  vtkSlicerTerminologyCategory
  vtkSlicerTerminologyType
  vtkSlicerTerminologyEntry
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTerminologiesModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TerminologiesModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_TERMINOLOGIES_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::TerminologiesModuleLogic PRIVATE RapidJSON)
