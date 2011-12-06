
#-----------------------------------------------------------------------------
# DiffusionTensorMathematics tests
#-----------------------------------------------------------------------------
set(CLP DiffusionTensorMathematics)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test MRMLCore vtkTeem SlicerBaseLogic)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  DiffusionTensorMathematicsTest
    --enumeration FractionalAnisotropy
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/DiffusionTensorMathematicsTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

