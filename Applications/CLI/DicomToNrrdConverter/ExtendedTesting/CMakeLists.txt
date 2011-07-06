#
# Test CLI executables
#
include_regular_expression("^.*$")

enable_testing()

if(NOT ITK_FOUND)
  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

include_directories(${DicomToNrrdConverter_SOURCE_DIR})
include_directories(${DicomToNrrdConverter_BINARY_DIR})
include_directories(${ModuleDescriptionParser_SOURCE_DIR})

# SlicerExecutionModel
find_package(SlicerExecutionModel REQUIRED GenerateCLP)
include(${GenerateCLP_USE_FILE})

set (CLP DWICompare)
set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)
set(DWICompareEXE ${DicomToNrrdConverter_BINARY_DIR}/../bin/${CLP})

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

set (DicomToNrrdConverterEXE ${Slicer_LAUNCH_COMMAND} ${DicomToNrrdConverter_BINARY_DIR}/../bin/${CLP} )
set (DicomToNrrdConverter_TESTS ${Slicer_LAUNCH_COMMAND} ${DicomToNrrdConverter_BINARY_DIR}/../bin/${CLP}Test )

if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${ARGV})
  endif(NOT ${KEY})
endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)

include(${BRAINS_CMAKE_HELPER_DIR}/MIDAS.cmake)
SETIFEMPTY(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")

set(MIDAS_KEY_DIR "${DicomToNrrdConverter_SOURCE_DIR}/ExtendedTesting/keys")

add_test(DicomToNrrdConverterHelpTest ${DicomToNrrdConverter_TESTS}
    DicomToNrrdConverterTest
    --help
  )

add_test(DicomToNrrdConverterDWICompareHelpTest ${Slicer_LAUNCH_COMMAND} ${DWICompareEXE}
    --help
  )

midas_add_test(NAME DicomToNrrdConverterGeSignaHdxTest COMMAND ${CMAKE_COMMAND}
        -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
        -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
        -D TEST_BASELINE=MIDAS{GeSignaHDx.nrrd.md5}
        -D TEST_INPUT=MIDAS_TGZ{GeSignaHDx.md5}
        -D TEST_TEMP_OUTPUT=${TEMP}/GeSignaHDxTest.nrrd
        -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
        )

midas_add_test(NAME DicomToNrrdConverterGeSignaHdxBigEndianTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{GeSignaHDx.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{GeSignaHDxBigEndian.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/GeSignaHDxBigEndianTest.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterGeSignaHdxBMatrixTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{GeSignaHDx.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{GeSignaHDx.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/GeSignaHDxTest.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterGeSignaHdxtTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{GeSignaHDxt.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{GeSignaHDxt.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/GeSignaHDxtTest.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterGeSignaHdxtBMatrixTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{GeSignaHDxt.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{GeSignaHDxt.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/GeSignaHDxtTest.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchievaTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva1.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva1.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva1Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchievaBigEndianTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva1.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchievaBigEndian1.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchievaBigEndian1Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchievaBMatrixTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva1.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva1.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva1Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(DicomToNrrdConverterPhilipsAchieva2Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva2.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva2.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva2Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchieva2BMatrixTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva2.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva2.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva2Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchieva3Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva3.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva3.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva3Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchieva4Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva4.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva4.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva4Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchieva6Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva6.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva6.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva6Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterPhilipsAchieva7Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{PhilipsAchieva7.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{PhilipsAchieva7.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/PhilipsAchieva7Test.nrrd
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterSiemensTrioTimTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{SiemensTrioTim1.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{SiemensTrioTim1.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/SiemensTrioTim1Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterSiemensTrioTimBigEndian1Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{SiemensTrioTim1.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{SiemensTrioTimBigEndian1.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/SiemensTrioTimBigEndian1Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterSiemensTrioTim2Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{SiemensTrioTim2.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{SiemensTrioTim2.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/SiemensTrioTim2Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterSiemensTrioTim3Test COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{SiemensTrioTim3.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{SiemensTrioTim3.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/SiemensTrioTim3Test.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

midas_add_test(NAME DicomToNrrdConverterSiemensVerioTest COMMAND ${CMAKE_COMMAND}
  ${CMAKE_COMMAND} -D TEST_PROGRAM=${DicomToNrrdConverterEXE}
                   -D TEST_COMPARE_PROGRAM=${DWICompareEXE}
                   -D TEST_BASELINE=MIDAS{SiemensVerio.nrrd.md5}
                   -D TEST_INPUT=MIDAS_TGZ{SiemensVerio.md5}
                   -D TEST_TEMP_OUTPUT=${TEMP}/SiemensVerio.nrrd
                   -D TEST_PROGRAM_ARGS=--useBMatrixGradientDirections
                   -P ${CMAKE_SOURCE_DIR}/ExtendedTesting/DicomToNrrdDWICompareTest.cmake
  )

