project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT "${PROJECT_NAME}")

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

set(${KIT}_SRCS
  vtkMRMLMultiVolumeNode.cxx
  vtkMRMLMultiVolumeNode.h
  vtkMRMLMultiVolumeDisplayNode.cxx
  vtkMRMLMultiVolumeDisplayNode.h
  vtkMRMLMultiVolumeStorageNode.cxx
  vtkMRMLMultiVolumeStorageNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  ${vtkTeem_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
