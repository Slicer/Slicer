project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

find_package(JsonCpp REQUIRED)

set(${KIT}_INCLUDE_DIRECTORIES
  ${JsonCpp_INCLUDE_DIR}
  )

set(${KIT}_SRCS
  vtkMRMLCPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLCPURayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLGPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkMRMLGPURayCast${MODULE_NAME}DisplayNode.h
  vtkMRMLMulti${MODULE_NAME}DisplayNode.cxx
  vtkMRMLMulti${MODULE_NAME}DisplayNode.h
  vtkMRMLShaderPropertyNode.cxx
  vtkMRMLShaderPropertyNode.h
  vtkMRMLShaderPropertyStorageNode.cxx
  vtkMRMLShaderPropertyStorageNode.h
  vtkMRMLVolumePropertyNode.cxx
  vtkMRMLVolumePropertyNode.h
  vtkMRMLVolumePropertyStorageNode.cxx
  vtkMRMLVolumePropertyStorageNode.h
  vtkMRML${MODULE_NAME}DisplayNode.cxx
  vtkMRML${MODULE_NAME}DisplayNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${JsonCpp_LIBRARY}
  ${MRML_LIBRARIES}
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
