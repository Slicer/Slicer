
#-----------------------------------------------------------------------------
# SparseFieldLevelSetContour tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP SparseFieldLevelSetContour)

include_directories(${SlicerBaseCLI_BINARY_DIR}
                    ${vtkTeem_INCLUDE_DIRS})
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib vtkITK vtkIO SlicerBaseCLI ModuleDescriptionParser)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

# leaks from tcl file
# set(testname ${CLP}Test2)
# add_test(NAME ${testname} COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
#  --test-mode
#  --script ${Slicer_SOURCE_DIR}/Modules/CLI/SparseFieldLevelSetContour/Testing/SparseFieldLevelSetContourTest.tcl
#  ${TEST_DATA}/SparseFieldLevelSetContourTest.vtp
#  ${TEMP}/SparseFieldLevelSetContourTest.vtp
#  ${BASELINE}/SparseFieldLevelSetContourTest.vtp
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})
# use a helper to compare the baseline and output, running it from the launcher to set up the paths

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test> ${testname}
  --inputScene ${TEST_DATA}/SparseFieldLevelSetContourTest.vtp
  --contourpoints 132.059,-3.68759,-378.683
  --contourpoints 127.652,-16.0285,-381.549
  --contourpoints 120.15,-27.4267,-382.79
  --contourpoints 117.438,-26.4077,-394.14
  --contourpoints 128.782,-14.2639,-389.404
  --contourpoints 134.297,1.63338,-387.623
  --outputModel ${TEMP}/SparseFieldLevelSetContourTest.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

