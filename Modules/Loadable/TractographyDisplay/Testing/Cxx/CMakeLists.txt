set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(KIT_TEST_SRCS
  qSlicerTractographyDisplayGlyphWidgetTest1.cxx
  vtkMRMLFiberBundleDisplayNodeTest1.cxx
  vtkMRMLFiberBundleGlyphDisplayNodeTest1.cxx
  vtkMRMLFiberBundleLineDisplayNodeTest1.cxx
  vtkMRMLFiberBundleNodeTest1.cxx
  vtkMRMLFiberBundleStorageNodeTest1.cxx
  vtkMRMLFiberBundleTubeDisplayNodeTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qSlicerTractographyDisplayGlyphWidgetTest1)
simple_test(vtkMRMLFiberBundleDisplayNodeTest1)
simple_test(vtkMRMLFiberBundleGlyphDisplayNodeTest1)
simple_test(vtkMRMLFiberBundleLineDisplayNodeTest1)
simple_test(vtkMRMLFiberBundleNodeTest1 ${INPUT}/helix_roi_tract.mrml)
simple_test(vtkMRMLFiberBundleStorageNodeTest1)
simple_test(vtkMRMLFiberBundleTubeDisplayNodeTest1)
