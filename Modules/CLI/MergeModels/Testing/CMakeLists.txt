
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${ITK_LIBRARIES} ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    DATA{${INPUT}/sphere.vtk}
    DATA{${INPUT}/cube.vtk}
    ${TEMP}/sphereCube.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}TestVtp)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    DATA{${INPUT}/sphere.vtp}
    DATA{${INPUT}/cube.vtp}
    ${TEMP}/sphereCube.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

# add a test that compares the outline to a baseline .vtp
set(testname ${CLP}TestCompare)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} ${CMAKE_COMMAND}
  -Dtest_cmd=$<TARGET_FILE:${CLP}Test>
  -Dtest_name=ModuleEntryPoint
  -Dmodel1=DATA{${INPUT}/sphere.vtp}
  -Dmodel2=DATA{${INPUT}/cube.vtp}
  -Doutput_model=${TEMP}/sphereCube.vtp
  -Doutput_baseline=DATA{${BASELINE}/sphereCube.vtp}
  -P ${CMAKE_CURRENT_SOURCE_DIR}/run_MergeModelsTest.cmake
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVtp)

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
