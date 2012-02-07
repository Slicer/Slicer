set(extensions DataProbe
    DICOM
    Editor
    LabelStatistics
    PerformanceTests
    SampleData)
set(KIT_UNITTEST_SCRIPTS)
foreach(extension ${extensions})
  SlicerMacroConfigureGenericPythonModuleTests("${extension}" KIT_UNITTEST_SCRIPTS)
endforeach()

foreach(script_name ${KIT_UNITTEST_SCRIPTS})
  slicer_add_python_unittest(
    SCRIPT ${script_name}
    SLICER_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_)
endforeach()

list(APPEND KIT_UNITTEST_SCRIPTS qSlicerSampleDataModuleDownloadVolumeTest.py)

foreach(script_name ${KIT_UNITTEST_SCRIPTS})
  slicer_add_python_unittest(SCRIPT ${script_name})
endforeach()

