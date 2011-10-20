set(TEST_DATA ${CMAKE_CURRENT_SOURCE_DIR}/../../Testing/Data/Input)

SLICER_ADD_PYTHON_TEST(SCRIPT ChangeTrackerTest1.py 
    SCRIPT_ARGS ${TEST_DATA}/MRMeningioma0.nrrd ${TEST_DATA}/MRMeningioma1.nrrd
    SLICER_ARGS --no-main-window)
  
  
  #add_test(NAME ChangeTrackerDemonsSegmentationTest1 COMMAND ${Launcher_Command} ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/DemonsSegmentationMetric --help)
  #add_test(NAME ChangeTrackerIntensityDifferenceTest1 COMMAND ${Launcher_Command} ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/IntensityDifferenceMetric --help)
