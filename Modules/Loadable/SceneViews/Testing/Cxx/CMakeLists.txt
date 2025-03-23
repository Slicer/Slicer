set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSceneViewEventsTest.cxx
  vtkSceneViewImportSceneTest.cxx
  vtkSceneViewStoreSceneTest.cxx
  vtkSceneViewTest1.cxx
  vtkSceneViewRestoreSceneTest.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  INCLUDE_DIRECTORIES ${vtkSlicerSequencesModuleLogic_INCLUDE_DIRS}
  TARGET_LIBRARIES vtkSlicer${MODULE_NAME}ModuleLogic qSlicer${MODULE_NAME}Module vtkSlicerSequencesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test(vtkSceneViewEventsTest)
simple_test(vtkSceneViewImportSceneTest ${TEMP})
simple_test(vtkSceneViewStoreSceneTest)
simple_test(vtkSceneViewTest1)
simple_test(vtkSceneViewRestoreSceneTest)
