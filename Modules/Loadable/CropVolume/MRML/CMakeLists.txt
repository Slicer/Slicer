set(classes vtkMRMLCropVolumeParametersNode)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerCropVolumeModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::CropVolumeModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_CROPVOLUME_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
