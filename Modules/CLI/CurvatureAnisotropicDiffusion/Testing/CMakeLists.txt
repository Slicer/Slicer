
#-----------------------------------------------------------------------------
#set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
#set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
#ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
#set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
#set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

#set(testname ${CLP}Test)
#ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
#  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
#  --compare DATA{${BASELINE}/${CLP}Test.nhdr,CurvatureAnisotropicDiffusionTest.raw.gz}
#            ${TEMP}/${CLP}Test.nhdr
#    ModuleEntryPoint
#    --conductance 2
#    --timeStep 0.0625
#    --iterations 2
#    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
#    ${TEMP}/${CLP}Test.nhdr
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
