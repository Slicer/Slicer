set(KIT qSlicer${MODULE_NAME}ModuleWidgets)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qSlicerDiffusionTensorVolumeDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest1.cxx
  qSlicerDTISliceDisplayWidgetTest2.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(APPEND Tests ${KIT_TEST_SRCS})

# Additional directories to include
include_directories(
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  )

set(test_target_libraries
  ${KIT}
  vtkSlicerVolumesModuleLogic
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${test_target_libraries})

set(DTI_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/helix-DTI.nhdr")

simple_test( qSlicerDiffusionTensorVolumeDisplayWidgetTest1 )
simple_test( qSlicerDTISliceDisplayWidgetTest1 )
simple_test( qSlicerDTISliceDisplayWidgetTest2 ${DTI_FILE} )
