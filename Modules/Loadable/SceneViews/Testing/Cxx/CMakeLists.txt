set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerSceneViewRestoreSceneTest.cxx
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

#-----------------------------------------------------------------------------
simple_test(vtkSlicerSceneViewRestoreSceneTest)
