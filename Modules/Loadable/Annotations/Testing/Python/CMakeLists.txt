#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
#-----------------------------------------------------------------------------

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyFiducials.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyRulers.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT AnnotationsTestingAddManyROIs.py 
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT RemoveAnnotationsIDFromSelectionNode.py
  SLICER_ARGS --disable-cli-modules)

slicer_add_python_test(SCRIPT LoadAnnotationRulerScene.py
  SCRIPT_ARGS ${INPUT}/ruler.mrml
  SLICER_ARGS --disable-cli-modules)

# Disabled scene view tests for now - they will be fixed in upcoming commit
# slicer_add_python_test(SCRIPT AnnotationsTestingFiducialWithSceneViewRestore.py
#   SLICER_ARGS --disable-cli-modules)
