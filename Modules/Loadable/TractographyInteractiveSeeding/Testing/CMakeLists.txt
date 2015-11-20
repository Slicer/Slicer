set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetTest1.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest2.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qSlicerTractographyInteractiveSeedingModuleWidgetTest1 )
simple_test( qSlicerTractographyInteractiveSeedingModuleWidgetTest2 ${INPUT}/helix-DTI.nhdr)
