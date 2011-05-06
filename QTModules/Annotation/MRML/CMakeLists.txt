PROJECT(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

SET(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
SET(module_mrml_export_directive "VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT")

# Additional directories to include
SET(module_mrml_include_directories
  )

# Source files
SET(module_mrml_SRCS
  vtkMRMLAnnotationSnapshotNode.cxx
  vtkMRMLAnnotationSnapshotNode.h
  vtkMRMLAnnotationHierarchyNode.cxx
  vtkMRMLAnnotationHierarchyNode.h
  vtkMRMLAnnotationNode.cxx
  vtkMRMLAnnotationNode.h
  vtkMRMLAnnotationStorageNode.cxx
  vtkMRMLAnnotationStorageNode.h
  vtkMRMLAnnotationDisplayNode.cxx
  vtkMRMLAnnotationDisplayNode.h
  vtkMRMLAnnotationTextDisplayNode.cxx
  vtkMRMLAnnotationTextDisplayNode.h
  vtkMRMLAnnotationControlPointsNode.cxx
  vtkMRMLAnnotationControlPointsNode.h
  vtkMRMLAnnotationControlPointsStorageNode.cxx
  vtkMRMLAnnotationControlPointsStorageNode.h
  vtkMRMLAnnotationPointDisplayNode.cxx
  vtkMRMLAnnotationPointDisplayNode.h
  vtkMRMLAnnotationLinesNode.cxx
  vtkMRMLAnnotationLinesNode.h
  vtkMRMLAnnotationLinesStorageNode.cxx
  vtkMRMLAnnotationLinesStorageNode.h
  vtkMRMLAnnotationLineDisplayNode.cxx
  vtkMRMLAnnotationLineDisplayNode.h
  vtkMRMLAnnotationAngleNode.cxx
  vtkMRMLAnnotationAngleNode.h
  vtkMRMLAnnotationAngleStorageNode.cxx
  vtkMRMLAnnotationAngleStorageNode.h
  vtkMRMLAnnotationRulerNode.cxx
  vtkMRMLAnnotationRulerNode.h
  vtkMRMLAnnotationRulerStorageNode.cxx
  vtkMRMLAnnotationRulerStorageNode.h
  vtkMRMLAnnotationFiducialNode.cxx
  vtkMRMLAnnotationFiducialNode.h
  vtkMRMLAnnotationFiducialsStorageNode.cxx
  vtkMRMLAnnotationFiducialsStorageNode.h
  vtkMRMLAnnotationStickyNode.cxx
  vtkMRMLAnnotationStickyNode.h
  vtkMRMLAnnotationTextNode.cxx
  vtkMRMLAnnotationTextNode.h
  vtkMRMLAnnotationROINode.cxx
  vtkMRMLAnnotationROINode.h
  vtkMRMLAnnotationSplineNode.cxx
  vtkMRMLAnnotationSplineNode.h
  vtkMRMLAnnotationBidimensionalNode.cxx
  vtkMRMLAnnotationBidimensionalNode.h
  )

# Additional Target libraries
SET(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  )

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )
