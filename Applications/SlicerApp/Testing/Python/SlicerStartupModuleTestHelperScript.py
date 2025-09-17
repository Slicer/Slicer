import sys
import os

testOutputFileName = os.environ['SLICER_STARTUP_MODULE_TEST_OUTPUT']

if '/test/directory' not in sys.path:
    raise RuntimeError('/test/directory not in sys.path as expected')

with open(testOutputFileName, 'w') as file:
    file.write('SlicerSTartypModuleTestHelperScript.py generated this file')
    file.write('/test/directory found in sys.path')

exit(EXIT_SUCCESS)
