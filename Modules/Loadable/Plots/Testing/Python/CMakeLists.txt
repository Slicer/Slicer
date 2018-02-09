if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME PlotsSelfTest
    SCRIPTS PlotsSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT PlotsSelfTest.py)
endif()
