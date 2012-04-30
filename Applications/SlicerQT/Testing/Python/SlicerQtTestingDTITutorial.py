import slicer
import slicer.testing
import slicer.util

filepath = os.environ['SLICER_HOME'] + '/../../Slicer4/Applications/SlicerQT/Testing/QtTesting/UnitTest_DTI_DWI.xml'

testUtility = slicer.app.testingUtility()
success = testUtility.playTests(filepath)
if success :
    slicer.util.exit(EXIT_SUCCESS)
else:
    slicer.util.exit(EXIT_FAILURE)

