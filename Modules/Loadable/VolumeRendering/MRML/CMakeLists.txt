project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkMRMLCPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLCPURayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLGPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLGPURayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLGPUTextureMapping${MODULE_NAME}DisplayNode.cxx
  vtkMRMLGPUTextureMapping${MODULE_NAME}DisplayNode.h
  vtkMRMLNCIMultiVolumeRayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLNCIMultiVolumeRayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLNCIRayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLNCIRayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLVolumePropertyNode.cxx
  vtkMRMLVolumePropertyNode.h
  vtkMRMLVolumePropertyStorageNode.cxx
  vtkMRMLVolumePropertyStorageNode.h
  vtkMRML${MODULE_NAME}DisplayNode.cxx
  vtkMRML${MODULE_NAME}DisplayNode.h
  vtkMRML${MODULE_NAME}ScenarioNode.cxx
  vtkMRML${MODULE_NAME}ScenarioNode.h
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
