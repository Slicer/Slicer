if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME SequencesSelfTest
    SCRIPTS SequencesSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT SequencesSelfTest.py)
endif()
