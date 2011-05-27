#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

include_directories(${DicomToNrrdConverter_SOURCE_DIR})
include_directories(${DicomToNrrdConverter_BINARY_DIR})
include_directories(${ModuleDescriptionParser_SOURCE_DIR})
include_directories (${TCLAP_SOURCE_DIR}/include)

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

# DicomToNrrdConverter tests
add_test(
    NAME ${CLP}Test 
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    ModuleEntryPoint
    --inputDicomDirectory ${TEST_DATA}/DWIDicom
    --outputDirectory ${TEMP}
    --outputVolume DWIDicom.nhdr
  )
SET_PROPERTY(TEST ${CLP}Test PROPERTY LABELS ${CLP})

