set(classes vtkMRMLAbstractVolumeResampler)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerGeneralizedReformatModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::GeneralizedReformatModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_GENERALIZEDREFORMAT_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
