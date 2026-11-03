set(classes
  vtkMRMLVolumeRenderingDisplayableManager
  vtkMRMLVolumeRenderingWindowLevelWidget
)

foreach(class IN LISTS classes)
  vtk_object_factory_declare(BASE ${class} OVERRIDE ${class})
endforeach()

set_property(GLOBAL PROPERTY "_vtk_module_autoinit_include" "\"vtkAutoInit.h\"")
vtk_object_factory_configure(
  SOURCE_FILE vtk_object_factory_source
  HEADER_FILE vtk_object_factory_header
  EXPORT_MACRO "VTK_SLICER_VOLUMERENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT"
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerVolumeRenderingModuleMRMLDisplayableManagerExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::VolumeRenderingModuleMRMLDisplayableManager
  EXPORT_MACRO_PREFIX VTK_SLICER_VOLUMERENDERING_MODULE_MRMLDISPLAYABLEMANAGER
  CLASSES ${classes}
  SOURCES ${vtk_object_factory_source}
  HEADERS ${wrapper_header_file}
  PRIVATE_HEADERS ${vtk_object_factory_header}
  EXCLUDE_HEADER_TEST
)
