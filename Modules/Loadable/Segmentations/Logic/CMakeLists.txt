project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerTerminologiesModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}ModuleLogic.cxx
  vtkSlicer${MODULE_NAME}ModuleLogic.h
  vtkSlicerSegmentationGeometryLogic.cxx
  vtkSlicerSegmentationGeometryLogic.h
  vtkImageGrowCutSegment.cxx
  vtkImageGrowCutSegment.h
  vtkImageLabelDilate3D.cxx
  vtkImageLabelDilate3D.h
  FibHeap.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerTerminologiesModuleLogic
  vtkSlicerMarkupsModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
