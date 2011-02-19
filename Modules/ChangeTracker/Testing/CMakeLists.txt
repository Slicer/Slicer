SET(KIT ChangeTrackerGUI)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkChangeTrackerGUITest.cxx
  vtkMRMLChangeTrackerNodeTest.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Testing

SET(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests 
  ${lib_name} 
  )

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
ENDMACRO()

SIMPLE_TEST( vtkChangeTrackerGUITest )
SIMPLE_TEST( vtkMRMLChangeTrackerNodeTest )



find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
  endif(GenerateCLP_FOUND)

  include_directories(${Slicer_SOURCE_DIR}/Applications/CLI)
  include_directories(${Slicer_BINARY_DIR}/Applications/CLI)
  include_directories(${ModuleDescriptionParser_SOURCE_DIR})
  include_directories (${TCLAP_SOURCE_DIR}/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../CommandLineApplication
                      ${ChangeTracker_BINARY_DIR}/CommandLineApplication  #
                      CLP.h will be there
                                         )

#
# Slicer3 is needed to resolve shared libraries
#
set(Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )

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
   ${Slicer_BINARY_DIR}/Slicer3 --launch ChangeTrackerTest 
   ChangeTrackerTest --help)

add_test(ChangeTrackerCL_TestFull_Intensity
   ${Slicer_BINARY_DIR}/Slicer3 --launch ChangeTrackerTest
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
