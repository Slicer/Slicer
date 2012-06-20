project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
#  vtkMRMLVolumePropertyNode.cxx
#  vtkMRMLVolumePropertyNode.h
#  vtkMRMLVolumePropertyStorageNode.cxx
#  vtkMRMLVolumePropertyStorageNode.h
  vtkMRMLMultiVolumeRenderingDisplayNode.cxx
  vtkMRMLMultiVolumeRenderingDisplayNode.h
#  vtkMRMLVolumeRenderingScenarioNode.cxx
#  vtkMRMLVolumeRenderingScenarioNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  vtkRendering
  vtkSlicerAnnotationsModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
