set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLVectorFieldDisplayNodeTest1.cxx
  vtkMRMLVectorFieldGeometryTest1.cxx
  vtkMRMLVectorFieldPipelineTest1.cxx
  vtkMRMLVectorFieldSamplePointSourceTest1.cxx
  vtkVectorFieldGeometryTest1.cxx
  vtkVectorFieldGlyphTest1.cxx
  vtkVectorFieldGridTest1.cxx
  vtkVectorFieldSamplerTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkMRMLVectorFieldDisplayNodeTest1)
simple_test(vtkMRMLVectorFieldGeometryTest1)
simple_test(vtkMRMLVectorFieldPipelineTest1)
simple_test(vtkMRMLVectorFieldSamplePointSourceTest1)
simple_test(vtkVectorFieldGeometryTest1)
simple_test(vtkVectorFieldGlyphTest1)
simple_test(vtkVectorFieldGridTest1)
simple_test(vtkVectorFieldSamplerTest1)
