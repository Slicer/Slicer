vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKCommon
    ITKImageFilterBase
    ITKIOImageBase
    ITKIOGDCM
)

set(classes vtkSlicerMultiVolumeExplorerLogic)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerMultiVolumeExplorerModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::MultiVolumeExplorerModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::MultiVolumeExplorerModuleLogic PUBLIC ${ITK_LIBRARIES})
