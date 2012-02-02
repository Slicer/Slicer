
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP MeshContourSegmentation)

include_directories(${SlicerBaseCLI_BINARY_DIR}
                    ${vtkTeem_INCLUDE_DIRS})
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib vtkITK vtkIO SlicerBaseCLI ModuleDescriptionParser)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

# TODO: Port Tcl test
# See http://viewvc.slicer.org/viewvc.cgi/Slicer3/trunk/Applications/CLI/SparseFieldLevelSetContour/

# add a test that compares the outline to a baseline .vtp
set(testname ${CLP}TestCompare)
add_test(NAME ${testname} COMMAND ${Launcher_Command} ${CMAKE_COMMAND}
  -Dtest_cmd=$<TARGET_FILE:${CLP}Test>
  -Dtest_name=${CLP}Test
  -Dinput_scene=${INPUT}/${CLP}Test.vtp
  -Dc1=132.059,-3.68759,-378.683
  -Dc2=127.652,-16.0285,-381.549
  -Dc3=120.15,-27.4267,-382.79
  -Dc4=117.438,-26.4077,-394.14
  -Dc5=128.782,-14.2639,-389.404
  -Dc6=134.297,1.63338,-387.623
  -Doutput_model=${TEMP}/${CLP}Test.vtp
  -Doutput_baseline=${BASELINE}/${CLP}Test.vtp
  -P ${CMAKE_CURRENT_SOURCE_DIR}/run_MeshContourSegmentationTest.cmake
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
