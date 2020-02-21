
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}TestWithImageSpacingOff ${CLP}Test.cxx)
target_link_libraries(${CLP}TestWithImageSpacingOff ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}TestWithImageSpacingOff PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}TestWithImageSpacingOff PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestWithImageSpacingOff)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}TestWithImageSpacingOff>
  --compare DATA{${BASELINE}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr,GradientAnisotropicDiffusionTestWithImageSpacingOff.raw.gz}
            ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOff.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}TestWithImageSpacingOn ${CLP}Test.cxx)
target_link_libraries(${CLP}TestWithImageSpacingOn ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}TestWithImageSpacingOn PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}TestWithImageSpacingOn PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestWithImageSpacingOn)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}TestWithImageSpacingOn>
  --compare DATA{${BASELINE}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr,GradientAnisotropicDiffusionTestWithImageSpacingOn.raw.gz}
            ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    --useImageSpacing
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/GradientAnisotropicDiffusionTestWithImageSpacingOn.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
