set(KIT ${PROJECT_NAME})

set(DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Data)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerTransformLogicTest1.cxx
  vtkSlicerTransformLogicTest2.cxx
  vtkSlicerTransformLogicTest3.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( vtkSlicerTransformLogicTest1 ${DATA_DIR}/affineTransform.txt)
simple_test( vtkSlicerTransformLogicTest2 ${DATA_DIR}/cube.vtk)
simple_test( vtkSlicerTransformLogicTest3 ${DATA_DIR}/cube.vtk ${DATA_DIR}/transformedCube.vtk)
