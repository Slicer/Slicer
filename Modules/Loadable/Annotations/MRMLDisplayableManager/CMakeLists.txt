project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_BINARY_DIR}
  )

set(displayable_manager_SRCS
  vtkMRMLAnnotationDisplayableManager.cxx
  vtkMRMLAnnotationDisplayableManager.h
  vtkMRMLAnnotationTextDisplayableManager.cxx
  vtkMRMLAnnotationTextDisplayableManager.h
  vtkMRMLAnnotationFiducialDisplayableManager.cxx
  vtkMRMLAnnotationFiducialDisplayableManager.h
  #vtkMRMLAnnotationStickyDisplayableManager.cxx
  #vtkMRMLAnnotationStickyDisplayableManager.h
  #vtkMRMLAnnotationAngleDisplayableManager.cxx
  #vtkMRMLAnnotationAngleDisplayableManager.h
  #vtkMRMLAnnotationSplineDisplayableManager.cxx
  #vtkMRMLAnnotationSplineDisplayableManager.h
  vtkMRMLAnnotationRulerDisplayableManager.cxx
  vtkMRMLAnnotationRulerDisplayableManager.h
  vtkMRMLAnnotationBidimensionalDisplayableManager.cxx
  vtkMRMLAnnotationBidimensionalDisplayableManager.h
  vtkMRMLAnnotationROIDisplayableManager.cxx
  vtkMRMLAnnotationROIDisplayableManager.h
  )

set(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${MODULE_NAME}Instantiator"
  displayable_manager_instantiator_SRCS
  "${displayable_manager_SRCS}"
  "${${KIT}_EXPORT_DIRECTIVE}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "${KIT}Export.h"
  )

set(${KIT}_SRCS
  ${displayable_manager_instantiator_SRCS}
  ${displayable_manager_SRCS}
  vtkMRMLAnnotationDisplayableManagerHelper.cxx
  vtkMRMLAnnotationClickCounter.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  vtkRendering
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleVTKWidgets
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
