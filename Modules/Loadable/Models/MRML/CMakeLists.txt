project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${${KIT}_SOURCE_DIR}
  ${${KIT}_BINARY_DIR}
)

set(${KIT}_SRCS
  vtkMRMLModelPickingNode.cxx
  vtkMRMLModelPickingNode.h
)

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
)

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
)

target_include_directories(
  ${KIT}
  PUBLIC
  ${${KIT}_SOURCE_DIR}
  ${${KIT}_BINARY_DIR}
)
