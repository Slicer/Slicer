slicer_add_python_test(SCRIPT AnnotationsTestingAddManyFiducials.py 
  SLICER_ARGS --disable-cli-module)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyRulers.py 
  SLICER_ARGS --disable-cli-module)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyROIs.py 
  SLICER_ARGS --disable-cli-module)
