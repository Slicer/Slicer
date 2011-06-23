set(KIT ChangeTrackerGUI)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkChangeTrackerGUITest.cxx
  vtkMRMLChangeTrackerNodeTest.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Testing

set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests
  ${lib_name}
  )

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

SIMPLE_TEST( vtkChangeTrackerGUITest )
SIMPLE_TEST( vtkMRMLChangeTrackerNodeTest )


find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

include_directories(
  ${Slicer_SOURCE_DIR}/Applications/CLI
  ${Slicer_BINARY_DIR}/Applications/CLI
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${TCLAP_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../CommandLineApplication
  ${ChangeTracker_BINARY_DIR}/CommandLineApplication  #CLP.h will be there
  )

set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")

add_executable(ChangeTrackerTest ChangeTrackerTest.cxx)
#add_dependencies(ChangeTrackerTest ChangeTrackerLib)
add_dependencies(ChangeTrackerTest ChangeTrackerCommandLine)
target_link_libraries(ChangeTrackerTest
                      ChangeTracker
#                      vtkTeem
)

add_test(ChangeTrackerCL_Help
   ${Slicer_LAUNCH_COMMAND} ChangeTrackerTest
   ChangeTrackerTest --help)

add_test(ChangeTrackerCL_TestFull_Intensity
   ${Slicer_LAUNCH_COMMAND} ChangeTrackerTest
   ChangeTrackerTest --sensitivity 0.95
   --roi_xyz -4,27,28
   --roi_radius 21,22,24
   --intensity_analysis
   --scan1 ${TEST_DATA}/MRMeningioma0.nrrd
   --scan2 ${TEST_DATA}/MRMeningioma1.nrrd
   --scan2tfm ${TEST_DATA}/MRMeningioma01.tfm
   --scan1segmentation ${TEST_DATA}/MRMeningioma01segmented.nrrd
   --tmpdirname ${TEMP}
   --terminationstep 0
   --output ${TEMP}/output.nrrd
)
