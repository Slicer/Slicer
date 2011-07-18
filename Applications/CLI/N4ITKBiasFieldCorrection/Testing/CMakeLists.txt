#
# Test CLI executables
#
include_regular_expression("^.*$")

# SlicerExecutionModel
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})


include_directories(${Slicer_SOURCE_DIR}/Applications/CLI)
include_directories(${Slicer_BINARY_DIR}/Applications/CLI)
include_directories(${ModuleDescriptionParser_SOURCE_DIR})
include_directories (${TCLAP_SOURCE_DIR}/include)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

#-----------------------------------------------------------------------------
# Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
# When this data is available, additional 3D tests are enabled.
find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha
  DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)

set(CLP N4ITKBiasFieldCorrection)
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/he3corrected.nii ${TEMP}/he3corrected.nii.gz
  ModuleEntryPoint
  --inputimage ${TEST_DATA}/he3volume.nii.gz --maskimage ${TEST_DATA}/he3mask.nii.gz
  --outputimage ${TEMP}/he3corrected.nii.gz --outputbiasfield ${TEMP}/he3biasfield.nii.gz
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

