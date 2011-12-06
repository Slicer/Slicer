
#-----------------------------------------------------------------------------
# DiffusionWeightedMasking tests
#-----------------------------------------------------------------------------
set (CLP DiffusionWeightedMasking)
include_directories(
  ${vtkITK_INCLUDE_DIRS}
  ${MRML_INCLUDE_DIRS}
  ${vtkTeem_INCLUDE_DIRS})
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test MRML vtkTeem SlicerBaseLogic)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  DiffusionWeightedMaskingTest
    --otsuomegathreshold 0.5
    --removeislands
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_baseline.nhdr
    ${TEMP}/DiffusionWeightedMaskingTest_output.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

