project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(module_mrml_name "${PROJECT_NAME}")

set(module_mrml_include_directory
  )
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

set(module_mrml_SRCS
  vtkMRMLMultiVolumeNode.cxx
  vtkMRMLMultiVolumeNode.h
  vtkMRMLMultiVolumeDisplayNode.cxx
  vtkMRMLMultiVolumeDisplayNode.h
  vtkMRMLMultiVolumeStorageNode.cxx
  vtkMRMLMultiVolumeStorageNode.h
  )

set(module_logic_target_libraries
  ${ITK_LIBRARIES}
  )

set(module_mrml_export_directive "VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_MRML_EXPORT")

set(module_mrml_target_libraries
  ${MRML_LIBRARIES}
  ${vtkTeem_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )
