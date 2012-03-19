set(KIT qSlicer${qt_module_name}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${qt_module_name} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerTractographyFiducialSeedingModuleWidgetTest1.cxx
  qSlicerTractographyFiducialSeedingModuleWidgetTest2.cxx
  EXTRA_INCLUDE TestingMacros.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

include_directories(
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT} vtkSlicerVolumesModuleLogic)

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

SET(VOLUME_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/helix-DTI.nhdr")

SIMPLE_TEST( qSlicerTractographyFiducialSeedingModuleWidgetTest1 )
SIMPLE_TEST( qSlicerTractographyFiducialSeedingModuleWidgetTest2 ${VOLUME_FILE})

