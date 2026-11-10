vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKTransform
)

set(classes
  vtkSlicerTransformLogic
  vtkSlicerTransformsReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerTransformsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::TransformsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_TRANSFORMS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::TransformsModuleLogic PUBLIC ${ITK_LIBRARIES})

vtk_module_set_property(Slicer::TransformsModuleLogic PROPERTY FOLDER VALUE Module-Transforms)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
