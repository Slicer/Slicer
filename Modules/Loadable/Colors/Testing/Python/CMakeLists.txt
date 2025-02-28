include(SlicerMacroBuildScriptedModule)
set(COLORS_PYTHON_RESOURCES
  )

# Test vtkSlicerScalarBarActor class
slicerMacroBuildScriptedModule(
  NAME ColorLegendSelfTest
  SCRIPTS ColorLegendSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT ColorLegendSelfTest.py
                           SLICER_ARGS --disable-cli-modules)

# Test restoring scene view with a custom color node
slicer_add_python_test(SCRIPT CustomColorTableSceneViewRestoreTestBug3992.py
                       SLICER_ARGS --disable-cli-modules)

# Test color table node with terminology information
slicer_add_python_test(SCRIPT ColorTerminologiesTest.py
                       SLICER_ARGS --disable-cli-modules)
