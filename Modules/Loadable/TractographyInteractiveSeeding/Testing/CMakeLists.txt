set(KIT qSlicer${MODULE_NAME}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerTractographyInteractiveSeedingModuleWidgetTest1.cxx
  qSlicerTractographyInteractiveSeedingModuleWidgetTest2.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

include_directories(
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  )

add_executable(${KIT}CxxTests ${Tests})
set_target_properties(${KIT}CxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})
target_link_libraries(${KIT}CxxTests ${KIT} vtkSlicerVolumesModuleLogic)

foreach(testname ${KIT_TEST_NAMES})
  simple_test( ${testname} )
endforeach()

SET(VOLUME_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/helix-DTI.nhdr")

simple_test( qSlicerTractographyInteractiveSeedingModuleWidgetTest1 )
simple_test( qSlicerTractographyInteractiveSeedingModuleWidgetTest2 ${VOLUME_FILE})

