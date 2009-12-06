#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

include_directories(${DicomToNrrdConverter_SOURCE_DIR})
include_directories(${DicomToNrrdConverter_BINARY_DIR})
include_directories(${ModuleDescriptionParser_SOURCE_DIR})
include_directories (${TCLAP_SOURCE_DIR}/include)

#
# Slicer3 is needed to resolve shared libraries
#
set(Slicer3_EXE ${DicomToNrrdConverter_BINARY_DIR}/DicomToNrrdConverter )
set(TEST_DATA "${Slicer3_SOURCE_DIR}/Testing/Data/Input")
set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

##################################
# DicomToNrrdConverter tests
add_test(DicomToNrrdConverterTest ${Slicer3_EXE} --launch DicomToNrrdConverterTest
  DicomToNrrdConverterTest
    --inputDicomDirectory ${TEST_DATA}/DWIDicom
    --outputVolume ${TEMP}/DWIDicom.nhdr
  )

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

