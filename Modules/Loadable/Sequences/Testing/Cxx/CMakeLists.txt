set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLSequenceBrowserNodeTest1.cxx
  vtkMRMLSequenceNodeTest1.cxx
  vtkMRMLSequenceStorageNodeTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkMRMLSequenceBrowserNodeTest1)
simple_test(vtkMRMLSequenceNodeTest1)
simple_test(vtkMRMLSequenceStorageNodeTest1)
