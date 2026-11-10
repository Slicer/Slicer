set(classes
  vtkMRMLSegmentationsDisplayableManager3D
  vtkMRMLSegmentationsDisplayableManager2D
  vtkSegmentationLabelmapSurfaceMapper
  )

foreach(class IN LISTS classes)
  vtk_object_factory_declare(BASE ${class} OVERRIDE ${class})
endforeach()

vtk_object_factory_configure(
  SOURCE_FILE vtk_object_factory_source
  HEADER_FILE vtk_object_factory_header
  EXPORT_MACRO "VTK_SLICER_SEGMENTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT"
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSegmentationsModuleMRMLDisplayableManagerExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SegmentationsModuleMRMLDisplayableManager
  EXPORT_MACRO_PREFIX VTK_SLICER_SEGMENTATIONS_MODULE_MRMLDISPLAYABLEMANAGER
  CLASSES ${classes}
  SOURCES ${vtk_object_factory_source}
  HEADERS ${wrapper_header_file}
  PRIVATE_HEADERS ${vtk_object_factory_header}
  EXCLUDE_HEADER_TEST
)
