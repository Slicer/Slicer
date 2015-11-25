set(KIT MultiVolumeExplorer)

set(KIT_TEST_SRCS
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

# Using SIMPLE_TEST(), you could add your test after this line.
