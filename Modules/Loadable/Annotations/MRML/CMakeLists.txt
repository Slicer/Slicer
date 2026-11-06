set(classes
  vtkMRMLAnnotationSnapshotNode
  vtkMRMLAnnotationSnapshotStorageNode
  vtkMRMLAnnotationHierarchyNode
  vtkMRMLAnnotationNode
  vtkMRMLAnnotationStorageNode
  vtkMRMLAnnotationDisplayNode
  vtkMRMLAnnotationTextDisplayNode
  vtkMRMLAnnotationControlPointsNode
  vtkMRMLAnnotationControlPointsStorageNode
  vtkMRMLAnnotationPointDisplayNode
  vtkMRMLAnnotationLinesNode
  vtkMRMLAnnotationLinesStorageNode
  vtkMRMLAnnotationLineDisplayNode
  vtkMRMLAnnotationRulerNode
  vtkMRMLAnnotationRulerStorageNode
  vtkMRMLAnnotationFiducialNode
  vtkMRMLAnnotationFiducialsStorageNode
  vtkMRMLAnnotationROINode
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerAnnotationsModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::AnnotationsModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_ANNOTATIONS_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
