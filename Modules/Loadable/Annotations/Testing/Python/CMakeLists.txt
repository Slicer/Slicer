slicer_add_python_test(SCRIPT AnnotationsTestingAddManyFiducials.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyRulers.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyROIs.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT RemoveAnnotationsIDFromSelectionNode.py
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT LoadAnnotationRulerScene.py
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT AnnotationsTestingFiducialWithSceneViewRestore.py
  SLICER_ARGS --disable-cli-modules)
