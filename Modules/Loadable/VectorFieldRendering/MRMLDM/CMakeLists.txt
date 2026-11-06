project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(${KIT}_SRCS
  vtkMRMLVectorField2DViewPipeline.cxx
  vtkMRMLVectorField2DViewPipeline.h
  vtkMRMLVectorField3DViewPipeline.cxx
  vtkMRMLVectorField3DViewPipeline.h
  vtkMRMLVectorFieldViewPipeline.cxx
  vtkMRMLVectorFieldViewPipeline.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerLayerDMModuleMRMLDisplayableManager
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
