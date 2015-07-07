#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(testname "py_mainwindow_RestoreSceneViewWithoutCamera")
add_test(
  NAME ${testname}
  COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
  --ignore-slicerrc --disable-cli-modules --disable-scripted-loadable-modules
  --testing --python-code "getNode('vtkMRMLSceneViewNode1').RestoreScene(); slicer.app.processEvents()"
  ${INPUT}/SceneViewWithoutCamera.mrml
  )
set_property(TEST ${testname} PROPERTY LABELS ${MODULE_NAME})

include(SlicerMacroBuildScriptedModule)

# Test adding new storable data after creating a scene view
slicerMacroBuildScriptedModule(
  NAME AddStorableDataAfterSceneViewTest
  SCRIPTS AddStorableDataAfterSceneViewTest.py
)
slicer_add_python_unittest(SCRIPT AddStorableDataAfterSceneViewTest.py
                           SLICER_ARGS --disable-cli-modules)
set_property(TEST py_AddStorableDataAfterSceneViewTest PROPERTY LABELS ${MODULE_NAME})
