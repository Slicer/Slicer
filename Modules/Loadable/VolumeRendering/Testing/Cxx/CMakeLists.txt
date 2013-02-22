set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData/")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qMRMLVolumePropertyNodeWidgetTest1.cxx
  qSlicerPresetComboBoxTest.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest1.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest2.cxx
  vtkMRMLVolumePropertyNodeTest1.cxx
  vtkMRMLVolumePropertyStorageNodeTest1.cxx
  vtkMRMLVolumeRenderingDisplayableManagerTest1.cxx
  vtkMRMLVolumeRenderingMultiVolumeTest.cxx
  )

#-----------------------------------------------------------------------------
QT4_GENERATE_MOCS(
  qSlicerPresetComboBoxTest.cxx
  )
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qMRMLVolumePropertyNodeWidgetTest1)
simple_test(qSlicerPresetComboBoxTest)
simple_test(qSlicer${MODULE_NAME}ModuleWidgetTest1)
simple_test(qSlicer${MODULE_NAME}ModuleWidgetTest2 ${INPUT}/fixed.nrrd)
simple_test(vtkMRMLVolumePropertyNodeTest1)
simple_test(vtkMRMLVolumePropertyStorageNodeTest1)
simple_test(vtkMRMLVolumeRenderingDisplayableManagerTest1)
simple_test(vtkMRMLVolumeRenderingMultiVolumeTest)
