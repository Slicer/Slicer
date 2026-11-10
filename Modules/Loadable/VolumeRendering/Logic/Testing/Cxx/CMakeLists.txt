set(KIT SlicerVolumeRenderingModuleLogic)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerVolumeRenderingLogicTest.cxx
  vtkSlicerVolumeRenderingLogicAddFromFileTest.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  LINK_TARGET Slicer::VolumeRenderingModuleLogic
  TARGET_LIBRARIES Slicer::VolumeRenderingModuleLogic
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerVolumeRenderingLogicTest ${Slicer_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/VolumeRendering)
simple_test(vtkSlicerVolumeRenderingLogicAddFromFileTest ${Slicer_BINARY_DIR}/Testing/Temporary/)
