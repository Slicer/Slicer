set(extensions DataProbe
    LabelStatistics
    PerformanceTests
    SampleData
    )

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND extensions
      DICOM
      )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND extensions
    Editor
    )
endif()

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

