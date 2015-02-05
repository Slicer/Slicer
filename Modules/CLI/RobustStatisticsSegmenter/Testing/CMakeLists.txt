
#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
add_executable(SFLSRobustStat3DTestConsole SFLSRobustStat3DTestConsole.cxx)
target_link_libraries(SFLSRobustStat3DTestConsole ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(SFLSRobustStat3DTestConsole PROPERTIES LABELS ${CLP})
set_target_properties(SFLSRobustStat3DTestConsole PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:SFLSRobustStat3DTestConsole>
    ${INPUT}/grayscale.nrrd
    ${INPUT}/grayscale-label.nrrd
    ${TEMP}/rss-test-seg.nrrd 50 0.1 0.2)
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
