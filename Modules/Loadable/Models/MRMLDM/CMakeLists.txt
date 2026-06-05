# --------------------------------------------------------------------------
project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${${KIT}_SOURCE_DIR}
  ${${KIT}_BINARY_DIR}
)

set(${KIT}_SRCS
  vtkMRMLModelDMPipeline.cxx
  vtkMRMLModelDMPipeline.h
  vtkMRMLModelDMPipelineCreatorLogic.cxx
  vtkMRMLModelDMPipelineCreatorLogic.h
  vtkMRMLModelPickingDMPipeline.cxx
  vtkMRMLModelPickingDMPipeline.h
)

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerLayerDMModuleMRMLDisplayableManager
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
