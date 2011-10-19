import slicer.testing
import slicer

# test that brainsfit is available and can run
# if this fails, changetracker cannot be used

tfm = slicer.modulemrml.vtkMRMLLinearTransformNode()

parameters = {}
parameters['fixedVolume'] = sys.argv[1]
parameters['movingVolume'] = sys.argv[2]
parameters['useRigid'] = True
parameters['linearTransform'] = tfm.GetID()

print 'ChangeTrackerTest1 brainsfit parameters: ', parameters

cliNode = None
cliNode = slicer.cli.run(slicer.modules.brainsfit, cliNode, parameters, wait_for_completion = True)

slicer.testing.setEnabled()
slicer.testing.exitSuccess()
