project(ChangeTrackerTesting)
#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

include_directories(${Slicer3_SOURCE_DIR}/Applications/CLI)
include_directories(${Slicer3_BINARY_DIR}/Applications/CLI)
include_directories(${ModuleDescriptionParser_SOURCE_DIR})
include_directories (${TCLAP_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../CommandLineApplication
                    ${ChangeTracker_BINARY_DIR}/CommandLineApplication  # CLP.h will be there
                   )

#-----------------------------------------------------------------------------
# Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
# When this data is available, additional 3D tests are enabled.
find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha 
  DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)


#
# Slicer3 is needed to resolve shared libraries
#
set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

set(BASELINE "${Slicer3_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(TEST_DATA "${Slicer3_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer3_HOME}/share/MRML/Testing/TestData")

add_executable(ChangeTrackerTest ChangeTrackerTest.cxx)
add_dependencies(ChangeTrackerTest ChangeTrackerCommandLineLib)
add_dependencies(ChangeTrackerTest ChangeTrackerCommandLine)
target_link_libraries(ChangeTrackerTest 
                      ChangeTracker
                      ChangeTrackerCommandLineLib
                     )

add_test(ChangeTrackerCL_Help
         ${Slicer3_BINARY_DIR}/Slicer3 --launch
         ChangeTrackerTest 
         ChangeTrackerTest --help
        )

add_test(ChangeTrackerCL_TestGlobalReg
         ${Slicer3_BINARY_DIR}/Slicer3 --launch
         ChangeTrackerTest
#         --compare ${TEST_DATA}/MRMeningioma0.nrrd ${TEST_DATA}/MRMeningioma1.nrrd
         ChangeTrackerTest
         --sensitivity 0.96
         --threshold 145,411
         --roi_min 107,68,59
         --roi_max 153,123,94
         --intensity_analysis
         --scan1 ${TEST_DATA}/MRMeningioma0.nrrd
         --scan2 ${TEST_DATA}/MRMeningioma1.nrrd
         --terminationstep 1
         --output ${TEMP}/output.nrrd
        )
        
add_test(ChangeTrackerCL_TestFull_Intensity
         ${Slicer3_BINARY_DIR}/Slicer3 --launch
         ChangeTrackerTest
#         --compare ${TEST_DATA}/MRMeningioma0.nrrd ${TEST_DATA}/MRMeningioma1.nrrd
         ChangeTrackerTest
         --sensitivity 0.96
         --threshold 145,411
         --roi_min 107,68,59
         --roi_max 153,123,94
         --intensity_analysis
         --scan1 ${TEST_DATA}/MRMeningioma0.nrrd
         --scan2 ${TEST_DATA}/MRMeningioma1.nrrd
         --terminationstep 0
         --output ${TEMP}/output.nrrd
        )
