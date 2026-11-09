project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_SRCS
  vtkMRMLVectorFieldDisplayNode.cxx
  vtkMRMLVectorFieldDisplayNode.h
  vtkMRMLVectorFieldMask.cxx
  vtkMRMLVectorFieldMask.h
  vtkMRMLVectorFieldPipeline.cxx
  vtkMRMLVectorFieldPipeline.h
  vtkMRMLVectorFieldSamplePointSource.cxx
  vtkMRMLVectorFieldSamplePointSource.h
  vtkVectorFieldGeometry.cxx
  vtkVectorFieldGeometry.h
  vtkVectorFieldGlyph.cxx
  vtkVectorFieldGlyph.h
  vtkVectorFieldGrid.cxx
  vtkVectorFieldGrid.h
  vtkVectorFieldSampler.cxx
  vtkVectorFieldSampler.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
