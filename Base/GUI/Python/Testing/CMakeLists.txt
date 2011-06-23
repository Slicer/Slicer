# --------------------------------------------------------------------------
# Testing
add_test(Slicer3PythonTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Python/Testing/SimpleTest.py)
