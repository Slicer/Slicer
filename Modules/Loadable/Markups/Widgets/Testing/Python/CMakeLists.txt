if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME MarkupsWidgetsSelfTest
    SCRIPTS MarkupsWidgetsSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT MarkupsWidgetsSelfTest.py)
endif()
