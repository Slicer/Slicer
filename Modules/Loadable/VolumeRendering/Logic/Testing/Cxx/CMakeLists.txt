set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerVolumeRenderingLogicTest.cxx
  vtkSlicerVolumeRenderingLogicAddFromFileTest.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerVolumeRenderingLogicTest ${Slicer_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME})
simple_test(vtkSlicerVolumeRenderingLogicAddFromFileTest ${Slicer_BINARY_DIR}/Testing/Temporary/)
