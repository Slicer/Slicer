project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkMRMLAnnotationSnapshotNode.cxx
  vtkMRMLAnnotationSnapshotNode.h
  vtkMRMLAnnotationSnapshotStorageNode.cxx
  vtkMRMLAnnotationSnapshotStorageNode.h
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
  vtkMRMLAnnotationRulerNode.cxx
  vtkMRMLAnnotationRulerNode.h
  vtkMRMLAnnotationRulerStorageNode.cxx
  vtkMRMLAnnotationRulerStorageNode.h
  vtkMRMLAnnotationFiducialNode.cxx
  vtkMRMLAnnotationFiducialNode.h
  vtkMRMLAnnotationFiducialsStorageNode.cxx
  vtkMRMLAnnotationFiducialsStorageNode.h
  vtkMRMLAnnotationROINode.cxx
  vtkMRMLAnnotationROINode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
