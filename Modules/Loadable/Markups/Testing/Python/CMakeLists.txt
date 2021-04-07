include(SlicerMacroBuildScriptedModule)
set(MARKUPS_PYTHON_RESOURCES
  )

# Test adding many fiducials
slicerMacroBuildScriptedModule(
  NAME AddManyMarkupsFiducialTest
  SCRIPTS AddManyMarkupsFiducialTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT AddManyMarkupsFiducialTest.py
                           SLICER_ARGS --disable-cli-modules)

# Disabled scene view tests for now - they will be fixed in upcoming commit
# # Test scene views
# slicer_add_python_test(SCRIPT MarkupsSceneViewRestoreTestSimple.py
#                        SLICER_ARGS --disable-cli-modules)
# slicer_add_python_test(SCRIPT MarkupsSceneViewRestoreTestManyLists.py
#                        SLICER_ARGS --disable-cli-modules)

# Test fiducials in the neurosurgical planning tutorial
slicerMacroBuildScriptedModule(
  NAME NeurosurgicalPlanningTutorialMarkupsSelfTest
  SCRIPTS NeurosurgicalPlanningTutorialMarkupsSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT NeurosurgicalPlanningTutorialMarkupsSelfTest.py)

# Test fiducials in compare viewers
slicerMacroBuildScriptedModule(
  NAME MarkupsInCompareViewersSelfTest
  SCRIPTS MarkupsInCompareViewersSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT MarkupsInCompareViewersSelfTest.py)

# Test fiducials in different views
slicerMacroBuildScriptedModule(
  NAME MarkupsInViewsSelfTest
  SCRIPTS MarkupsInViewsSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT MarkupsInViewsSelfTest.py
                           SLICER_ARGS --disable-cli-modules)

# Test curvature computation
slicer_add_python_test(SCRIPT MarkupsCurveMeasurementsTest.py
                       SLICER_ARGS --disable-cli-modules)

# Visual test of coordinate system along the curve (normal, binormal, tangent computation)
slicer_add_python_test(SCRIPT MarkupsCurveCoordinateFrameTest.py
                       SLICER_ARGS --disable-cli-modules)

# Test other measurements
slicer_add_python_test(SCRIPT MarkupsMeasurementsTest.py
                       SLICER_ARGS --disable-cli-modules)

# Test markups pluggable architecture
slicerMacroBuildScriptedModule(
  NAME PluggableMarkupsSelfTest
  SCRIPTS PluggableMarkupsSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )
slicer_add_python_unittest(SCRIPT PluggableMarkupsSelfTest.py)
