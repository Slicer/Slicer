#
# Test CLI executables
#
include_regular_expression("^.*$")

enable_testing()

find_package(Slicer3 REQUIRED)
include(${Slicer3_USE_FILE})

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
set(Slicer3_EXE ${Slicer3_DIR}/Slicer3 )
set(TEST_DATA "${Slicer3_SOURCE_DIR}/Testing/Data/Input")
set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

##################################
# DicomToNrrdConverter tests
add_test(DicomToNrrdConverterTest ${Slicer3_EXE} --launch DicomToNrrdConverterTest
    ModuleEntryPoint
    --inputDicomDirectory ${TEST_DATA}/DWIDicom
    --outputDirectory ${TEMP}
    --outputVolume DWIDicom.nhdr
  )

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)

