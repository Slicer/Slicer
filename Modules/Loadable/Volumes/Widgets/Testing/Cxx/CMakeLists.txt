set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicerDiffusionTensorVolumeDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest2.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  INCLUDE_DIRECTORIES
    ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
    ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qSlicerDiffusionTensorVolumeDisplayWidgetTest1)
simple_test(qSlicerDTISliceDisplayWidgetTest1)
simple_test(qSlicerDTISliceDisplayWidgetTest2 DATA{${INPUT}/helix-DTI.nhdr})
