
#-----------------------------------------------------------------------------
# GrayscaleModelMaker tests
#-----------------------------------------------------------------------------
set (CLP GrayscaleModelMaker)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK
  ${ITK_LIBRARIES}
  ModuleDescriptionParser
  SlicerBaseCLI
  vtkHybrid
  )
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  GrayscaleModelMakerTest
    --threshold 300
    --name CTFace
    --smooth 15
    --decimate 0.95
    --splitnormals
    --pointnormals
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}GrayscaleModelMakerTest.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

