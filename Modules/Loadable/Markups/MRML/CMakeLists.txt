project(vtkSlicer${MODULE_NAME}ModuleMRML)

find_package(RapidJSON REQUIRED)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_BINARY_DIR}
  )

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}JsonElement.cxx
  vtkMRML${MODULE_NAME}JsonElement.h
  vtkMRML${MODULE_NAME}JsonStorageNode.cxx
  vtkMRML${MODULE_NAME}JsonStorageNode.h
  vtkMRML${MODULE_NAME}PlaneJsonStorageNode.cxx
  vtkMRML${MODULE_NAME}PlaneJsonStorageNode.h
  vtkMRML${MODULE_NAME}ROIJsonStorageNode.cxx
  vtkMRML${MODULE_NAME}ROIJsonStorageNode.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  RapidJSON
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
