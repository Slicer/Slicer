project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
  )

set(displayable_manager_SRCS
  vtkMRMLVolumeRenderingDisplayableManager.cxx
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
  )

set(${KIT}_VTK_LIBRARIES
  VTK::RenderingVolume
  VTK::RenderingVolumeOpenGL2
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  ${MRML_LIBRARIES}
  ${ITK_LIBRARIES}
  ${${KIT}_VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
