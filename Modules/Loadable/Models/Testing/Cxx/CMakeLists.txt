set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(MODEL_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/cube.vtk")
set(MODEL_SCENE "${MRMLCore_SOURCE_DIR}/Testing/modelHierarchy.mrml")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicerModelsModuleWidgetTest
  qSlicerModelsModuleWidgetTest1
  qSlicerModelsModuleWidgetTestScene
  vtkSlicerModelsLogicAddFileTest
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_GENERATE_MOC_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetTest.cxx
  )
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})

include_directories( ${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qSlicerModelsModuleWidgetTest1 ${MODEL_FILE} )
simple_test( qSlicerModelsModuleWidgetTestScene ${MODEL_SCENE} )
simple_test(vtkSlicerModelsLogicAddFileTest ${MODEL_FILE})
simple_test( qSlicerModelsModuleWidgetTest )
