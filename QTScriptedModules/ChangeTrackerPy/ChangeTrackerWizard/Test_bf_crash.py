p = {}
p['fixedVolume'] = 'vtkMRMLScalarVolumeNode1'
p['movingVolume'] = 'vtkMRMLScalarVolumeNode2'
p['initializeTransformNode'] = 'useMomentsOn'
p['useRigid'] = True
tfm = slicer.mrmlScene.CreateNodeByClass('vtkMRMLLinearTransformNode')
slicer.mrmlScene.AddNode(tfm)
cliNode = None
cliNode = slicer.cli.run(slicer.modules.brainsfit, cliNode, p, 1)
