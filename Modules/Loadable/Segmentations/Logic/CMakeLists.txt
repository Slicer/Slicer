project(vtkSlicerSegmentationsModuleLogic)

set(classes
  vtkSlicerSegmentationsModuleLogic
  vtkSlicerSegmentationGeometryLogic
  vtkSlicerSegmentationsReader
  vtkSlicerSegmentationsNodeWriter
  vtkImageGrowCutSegment
  FibHeap
  vtkSlicerSegmentEditorLogic
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerSegmentationsModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::SegmentationsModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_SEGMENTATIONS_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
