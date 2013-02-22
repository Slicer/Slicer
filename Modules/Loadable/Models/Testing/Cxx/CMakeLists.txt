set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(MODEL_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/cube.vtk")
set(MODEL_SCENE "${MRMLCore_SOURCE_DIR}/Testing/modelHierarchy.mrml")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicerModelsModuleWidgetTest1
  qSlicerModelsModuleWidgetTestScene
  vtkSlicerModelsLogicAddFileTest
  vtkSlicerModelsLogicAutoRemoveTest
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qSlicerModelsModuleWidgetTest1 ${MODEL_FILE} )
simple_test( qSlicerModelsModuleWidgetTestScene ${MODEL_SCENE} )
simple_test(vtkSlicerModelsLogicAddFileTest ${MODEL_FILE})
simple_test(vtkSlicerModelsLogicAutoRemoveTest)
