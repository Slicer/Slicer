project(MRIBiasFieldCorrectionTesting)
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
include_directories(${TCLAP_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../CommandLineApplication
  ${MRIBiasFieldCorrection_BINARY_DIR}/CommandLineApplication
  # CLP.h will be there
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

add_executable(MRIBiasFieldCorrectionTest MRIBiasFieldCorrectionTest.cxx)
add_dependencies(MRIBiasFieldCorrectionTest MRIBiasFieldCorrectionCommandLineLib)
add_dependencies(MRIBiasFieldCorrectionTest MRIBiasFieldCorrectionCommandLine)
target_link_libraries(MRIBiasFieldCorrectionTest 
                      MRIBiasFieldCorrection
                      MRIBiasFieldCorrectionCommandLineLib
                     )

add_test(MRIBiasFieldCorrectionCL_Glob
         ${Slicer3_BINARY_DIR}/Slicer3 --launch
         MRIBiasFieldCorrectionCommandLine
         --iterations 10
         --numberOfFitting 4
         --threshold 10
         --wienerFilterNoise 0.1
         --biasFieldFullWidthAtHalfMaximum 0.15
         --convergenceThreshold 0.001
          ${TEST_DATA}/MRHeadResampled.nhdr
          ${TEMP}/output.nrrd
        )
