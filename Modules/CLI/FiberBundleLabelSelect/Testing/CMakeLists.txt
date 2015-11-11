
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(TEST_DATA ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    ${TEST_DATA}/mask.nrrd
    ${TEST_DATA}/tractography.vtk
    ${TEMP}/selectedfibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


add_executable(CompareTensorMeasure CompareTensorMeasure.cxx)
target_link_libraries(CompareTensorMeasure ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(CompareTensorMeasure PROPERTIES LABELS ${CLP})
set_target_properties(CompareTensorMeasure PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestCompare)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTensorMeasure>
  ${TEMP}/selectedfibers.vtk  ${BASELINE}/selectedfibersbaseline.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}Test)
