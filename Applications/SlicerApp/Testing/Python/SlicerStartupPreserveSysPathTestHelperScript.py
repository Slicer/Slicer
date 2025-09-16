import sys
import os

SENTINEL = "/test/slicer-start-preserve-sys-path/directory"
if SENTINEL not in sys.path:
    raise RuntimeError(f"{SENTINEL} not in sys.path as expected")

testOutputFileName = os.environ["SLICER_STARTUP_MODULE_TEST_OUTPUT"]
with open(testOutputFileName, "w") as file:
    file.write("SlicerStartupPreserveSysPathTestHelperScript.py generated this file")
    file.write(f"{SENTINEL} found in sys.path")

exit(EXIT_SUCCESS)
