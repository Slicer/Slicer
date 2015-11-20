set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerColorLogicTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerColorLogicTest1)

