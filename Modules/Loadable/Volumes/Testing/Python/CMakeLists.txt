#slicer_add_python_test(SCRIPT LoadVolumeDisplaybleSceneModelClose.py
#  SLICER_ARGS --disable-cli-modules)

slicer_add_python_unittest(SCRIPT LoadVolumeDisplaybleSceneModelClose.py)

slicer_add_python_unittest(SLICER_ARGS --disable-cli-modules SCRIPT VolumesLogicCompareVolumeGeometry.py)
