#
# Test CLI executables
#
INCLUDE_REGULAR_EXPRESSION("^.*$")

FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

INCLUDE_DIRECTORIES(${Slicer3_SOURCE_DIR}/Applications/CLI)
INCLUDE_DIRECTORIES(${Slicer3_BINARY_DIR}/Applications/CLI)
INCLUDE_DIRECTORIES(${Slicer3_SOURCE_DIR}/Libs/ModuleDescriptionParser)
INCLUDE_DIRECTORIES (${TCLAP_SOURCE_DIR}/include)

#
# Put the test executables in the Slicer3 bin directory
#
SET(EXECUTABLE_OUTPUT_PATH "${Slicer3_BINARY_DIR}/bin")

#
# Slicer3 is needed to resolve shared libraries
#
SET(SLICER_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

SET(BASELINE "${Slicer3_SOURCE_DIR}/Testing/Data/Baseline/CLI")
SET(TEST_DATA "${Slicer3_SOURCE_DIR}/Testing/Data/Input")
SET(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

SET(MRML_DATA "${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData")

# OrientImage tests
ADD_TEST(OrientImageTest1 ${SLICER_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestAxial.nrrd
            ${TEMP}/OrientImageTestAxial.nrrd
  OrientImageTest
           --orientation Axial ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestAxial.nrrd
)
ADD_TEST(OrientImageTest2 ${SLICER_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestCoronal.nrrd
            ${TEMP}/OrientImageTestCoronal.nrrd
  OrientImageTest
           --orientation Coronal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestCoronal.nrrd
)
ADD_TEST(OrientImageTest3 ${SLICER_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestSagittal.nrrd
            ${TEMP}/OrientImageTestSagittal.nrrd
  OrientImageTest
           --orientation Sagittal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestSagittal.nrrd
)

SET (CLP OrientImage)
ADD_EXECUTABLE(${CLP}Test ${CLP}Test.cxx)
ADD_DEPENDENCIES(${CLP}Test ${CLP}Lib)
ADD_DEPENDENCIES(${CLP}Test ${CLP})
TARGET_LINK_LIBRARIES(${CLP}Test ITKIO)

# Median tests
ADD_TEST(MedianImageFilterTest ${SLICER_EXE} --launch MedianImageFilterTest
  --compare ${BASELINE}/MedianImageFilterTest.nhdr
            ${TEMP}/MedianImageFilterTest.nhdr
  MedianImageFilterTest
           --neighborhood 1,2,3 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/MedianImageFilterTest.nhdr
)

SET (CLP MedianImageFilter)
ADD_EXECUTABLE(${CLP}Test ${CLP}Test.cxx)
ADD_DEPENDENCIES(${CLP}Test ${CLP}Lib)
ADD_DEPENDENCIES(${CLP}Test ${CLP})
TARGET_LINK_LIBRARIES(${CLP}Test ITKIO)

# GaussianBlur tests
ADD_TEST(GaussianBlurImageFilterTest ${SLICER_EXE} --launch GaussianBlurImageFilterTest
  --compare ${BASELINE}/GaussianBlurImageFilterTest.nhdr
            ${TEMP}/GaussianBlurImageFilterTest.nhdr
  GaussianBlurImageFilterTest
      --sigma 2.5 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/GaussianBlurImageFilterTest.nhdr
)
SET (CLP GaussianBlurImageFilter)
ADD_EXECUTABLE(${CLP}Test ${CLP}Test.cxx)
ADD_DEPENDENCIES(${CLP}Test ${CLP}Lib)
ADD_DEPENDENCIES(${CLP}Test ${CLP})
TARGET_LINK_LIBRARIES(${CLP}Test ITKIO)
