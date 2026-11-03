set(KIT SlicerColorsModuleLogic)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerColorLogicTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  LINK_TARGET Slicer::ColorsModuleLogic
  TARGET_LIBRARIES Slicer::ColorsModuleLogic
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerColorLogicTest1)
