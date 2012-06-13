import os
import slicer
from slicer import slicerappdatapaths

filepath = slicerappdatapaths.input + '/DiffusionTensorImagingTutorial.xml'
testUtility = slicer.app.testingUtility()
success = testUtility.playTests(filepath)
if not success :
  raise Exception('Failed to finished properly the play back !')
