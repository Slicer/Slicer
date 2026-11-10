set(classes
  vtkSlicerDICOMLoadable
  vtkSlicerDICOMExportable
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerDICOMLibModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::DICOMLibModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_DICOMLIB_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
