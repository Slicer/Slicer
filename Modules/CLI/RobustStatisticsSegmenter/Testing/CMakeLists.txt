
#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ctk_add_executable_utf8(SFLSRobustStat3DTestConsole SFLSRobustStat3DTestConsole.cxx)
target_link_libraries(SFLSRobustStat3DTestConsole ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(SFLSRobustStat3DTestConsole PROPERTIES LABELS ${CLP})
set_target_properties(SFLSRobustStat3DTestConsole PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:SFLSRobustStat3DTestConsole>
    DATA{${INPUT}/grayscale.nrrd}
    DATA{${INPUT}/grayscale-label.nrrd}
    ${TEMP}/rss-test-seg.nrrd 50 0.1 0.2)
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
