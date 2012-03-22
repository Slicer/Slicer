set(KIT VolumesWidgets)

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

# Additional Target libraries
set(test_target_libraries
  ${PROJECT_NAME}
  vtkSlicerVolumesModuleLogic
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${test_target_libraries})

set(DTI_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/helix-DTI.nhdr")

SIMPLE_TEST( qSlicerDiffusionTensorVolumeDisplayWidgetTest1 )
SIMPLE_TEST( qSlicerDTISliceDisplayWidgetTest1 )
SIMPLE_TEST( qSlicerDTISliceDisplayWidgetTest2 ${DTI_FILE} )
