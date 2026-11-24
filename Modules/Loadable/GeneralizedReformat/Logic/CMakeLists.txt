vtk_module_find_package(PACKAGE vtkAddon)

set(classes
  vtkSlicerGeneralizedReformatLogic
  vtkMRMLScalarVectorDWIVolumeResampler
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerGeneralizedReformatModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::GeneralizedReformatModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_GENERALIZEDREFORMAT_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::GeneralizedReformatModuleLogic PUBLIC vtkAddon)
