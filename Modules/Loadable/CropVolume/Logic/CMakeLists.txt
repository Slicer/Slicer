set(classes vtkSlicerCropVolumeLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerCropVolumeModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::CropVolumeModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_CROPVOLUME_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
