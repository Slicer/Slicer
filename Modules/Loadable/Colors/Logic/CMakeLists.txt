set(classes
  vtkSlicerColorLogic
  vtkSlicerColorsReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerColorsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::ColorsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_COLORS_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_set_property(Slicer::ColorsModuleLogic PROPERTY FOLDER VALUE Module-Colors)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
