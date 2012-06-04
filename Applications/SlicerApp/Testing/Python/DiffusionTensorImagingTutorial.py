import os
import slicer
import ApplicationsSlicerAppData as data


filepath = data.input + '/DiffusionTensorImagingTutorial.xml'
testUtility = slicer.app.testingUtility()
success = testUtility.playTests(filepath)
if not success :
  raise Exception('Failed to finished properly the play back !')
