if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)
  slicerMacroBuildScriptedModule(
    NAME CropVolumeSelfTest
    SCRIPTS CropVolumeSelfTest.py
    )
  slicer_add_python_unittest(SCRIPT CropVolumeSelfTest.py)
endif()
