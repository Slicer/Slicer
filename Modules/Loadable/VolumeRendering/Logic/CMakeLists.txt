set(classes
  vtkSlicerShaderPropertyReader
  vtkSlicerVolumeRenderingLogic
  vtkSlicerVolumeRenderingReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerVolumeRenderingModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::VolumeRenderingModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_set_property(Slicer::VolumeRenderingModuleLogic PROPERTY FOLDER VALUE Module-VolumeRendering)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
