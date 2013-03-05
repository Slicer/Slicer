
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

# Test the CLI with image spacing turned off
set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GradientAnisotropicDiffusionTest.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


# Test the CLI with image spacing turned on.
add_executable(${CLP}Test2 ${CLP}Test.cxx)
target_link_libraries(${CLP}Test2 ${CLP}Lib)
set_target_properties(${CLP}Test2 PROPERTIES LABELS ${CLP})

set(testname2 ${CLP}Test2)
add_test(NAME ${testname2} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test2>
  --compare ${BASELINE}/GradientAnisotropicDiffusionTest2.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTest2.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    --useImageSpacing
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTest2.nhdr
  )
set_property(TEST ${testname2} PROPERTY LABELS ${CLP})

