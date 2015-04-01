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
