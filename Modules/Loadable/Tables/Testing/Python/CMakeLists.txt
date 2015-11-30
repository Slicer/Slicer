if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME TablesSelfTest
    SCRIPTS TablesSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT TablesSelfTest.py)
endif()
