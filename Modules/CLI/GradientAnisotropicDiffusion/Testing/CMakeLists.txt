
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}TestWithImageSpacingOff ${CLP}Test.cxx)
target_link_libraries(${CLP}TestWithImageSpacingOff ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}TestWithImageSpacingOff PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}TestWithImageSpacingOff PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestWithImageSpacingOff)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}TestWithImageSpacingOff>
  --compare ${BASELINE}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


#-----------------------------------------------------------------------------
add_executable(${CLP}TestWithImageSpacingOn ${CLP}Test.cxx)
target_link_libraries(${CLP}TestWithImageSpacingOn ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}TestWithImageSpacingOn PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}TestWithImageSpacingOn PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestWithImageSpacingOn)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}TestWithImageSpacingOn>
  --compare ${BASELINE}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    --useImageSpacing
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

