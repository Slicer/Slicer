set(TEST_DATA ${CMAKE_SOURCE_DIR}/Testing/Data/Input)

if (Slicer_BUILD_CLI)
  # ChangeTracker internally calls CLI
  slicer_add_python_test(SCRIPT ChangeTrackerTest1.py 
    SCRIPT_ARGS ${TEST_DATA}/MRMeningioma0.nrrd ${TEST_DATA}/MRMeningioma1.nrrd
    SLICER_ARGS --no-main-window)
endif()
  
  #add_test(NAME ChangeTrackerDemonsSegmentationTest1 COMMAND ${Launcher_Command} ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/DemonsSegmentationMetric --help)
  #add_test(NAME ChangeTrackerIntensityDifferenceTest1 COMMAND ${Launcher_Command} ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/IntensityDifferenceMetric --help)
