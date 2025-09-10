project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES)

set(displayable_manager_SRCS
  vtkMRMLLayerDisplayableManager.cxx
)

SlicerConfigureDisplayableManagerObjectFactory(
  TARGET_NAME ${KIT}
  SRCS "${displayable_manager_SRCS}"
  EXPORT_MACRO "${${KIT}_EXPORT_DIRECTIVE}"
  EXPORT_HEADER "${KIT}Export.h"
  OUTPUT_SRCS_VAR displayable_manager_instantiator_SRCS
)

set(${KIT}_SRCS
  ${displayable_manager_instantiator_SRCS}
  ${displayable_manager_SRCS}
  vtkMRMLLayerDMCameraSynchronizer.cxx
  vtkMRMLLayerDMCameraSynchronizer.h
  vtkMRMLLayerDMInteractionLogic.cxx
  vtkMRMLLayerDMInteractionLogic.h
  vtkMRMLLayerDMLayerManager.cxx
  vtkMRMLLayerDMLayerManager.h
  vtkMRMLLayerDMPipelineCallbackCreator.cxx
  vtkMRMLLayerDMPipelineCallbackCreator.h
  vtkMRMLLayerDMPipelineCreateHelper.h
  vtkMRMLLayerDMPipelineCreatorI.cxx
  vtkMRMLLayerDMPipelineCreatorI.h
  vtkMRMLLayerDMPipelineFactory.cxx
  vtkMRMLLayerDMPipelineFactory.h
  vtkMRMLLayerDMPipelineI.cxx
  vtkMRMLLayerDMPipelineI.h
  vtkMRMLLayerDMPipelineManager.cxx
  vtkMRMLLayerDMPipelineManager.h
  vtkMRMLLayerDisplayableManager.h
  vtkObjectEventObserver.cxx
  vtkObjectEventObserver.h
)

if (VTK_WRAP_PYTHON)
  list(APPEND ${KIT}_SRCS
    vtkMRMLLayerDMPipelineScriptedCreator.cxx
    vtkMRMLLayerDMPipelineScriptedCreator.h
    vtkMRMLLayerDMScriptedPipelineBridge.cxx
    vtkMRMLLayerDMScriptedPipelineBridge.h
  )

  add_subdirectory(Python)
endif ()


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

target_compile_features(${KIT} PUBLIC cxx_std_17)
