
#-----------------------------------------------------------------------------
# CurvatureAnisotropicDiffusion tests
#-----------------------------------------------------------------------------
#set (CLP CurvatureAnisotropicDiffusion)

#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)

#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  --compare ${BASELINE}/${CLP}Test.nhdr
#            ${TEMP}/${CLP}Test.nhdr
#    ModuleEntryPoint
#    --conductance 2
#    --timeStep 0.0625
#    --iterations 2
#    ${TEST_DATA}/MRHeadResampled.nhdr
#    ${TEMP}/${CLP}Test.nhdr
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})

