import slicer

# test that brainsfit is available and can run
# if this fails, changetracker cannot be used

tfm = slicer.vtkMRMLLinearTransformNode()
slicer.mrmlScene.AddNode(tfm)

vl = slicer.modules.volumes.logic()
if len(sys.argv) > 1:
  path1 = sys.argv[1]
  path2 = sys.argv[2]
else:
  path1 = slicer.app.slicerHome + "/../../Slicer/Libs/MRML/Core/Testing/TestData/fixed.nrrd"
  path2 = slicer.app.slicerHome + "/../../Slicer/Libs/MRML/Core/Testing/TestData/moving.nrrd"


vol1 = vl.AddArchetypeVolume(path1, 'fixed', 0)
vol2 = vl.AddArchetypeVolume(path2, 'moving', 1)

parameters = {}
parameters['fixedVolume'] = vol1.GetID()
parameters['movingVolume'] = vol2.GetID()
parameters['useRigid'] = True
#parameters['outputVolume'] = '/tmp/test.nrrd'
parameters['linearTransform'] = tfm.GetID()

print 'ChangeTrackerTest1 brainsfit parameters: ', parameters

cliNode = None
cliNode = slicer.cli.run(slicer.modules.brainsfit, cliNode, parameters, wait_for_completion = True)

