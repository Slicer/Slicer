## Registration

### Register two images with BRAINSFit

Register two brain MRI images of the same patient acquired at different timepoints using BRAINSFit ("General registration (BRAINS)" module).

```python
# Get sample input data
import SampleData
sampleDataLogic = SampleData.SampleDataLogic()
fixedVolumeNode = sampleDataLogic.downloadMRBrainTumor1()
movingVolumeNode = sampleDataLogic.downloadMRBrainTumor2()
# Create new nodes for output
transformedMovingVolumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
transformNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTransformNode")

# Run registration
parameters = {}
parameters["fixedVolume"] = fixedVolumeNode.GetID()
parameters["movingVolume"] = movingVolumeNode.GetID()
parameters["outputVolume"] = transformedMovingVolumeNode.GetID()
parameters["linearTransform"] = transformNode.GetID()
parameters["useRigid"] = True  # options include: "useRigid", "useAffine", "useBSpline"
parameters["initializeTransformMode"] = "useGeometryAlign"
parameters["samplingPercentage"] = 0.02
cliBrainsFitRigidNode = slicer.cli.run(slicer.modules.brainsfit, None, parameters, wait_for_completion=True)

# Display fused result. Computed transformNode is automatically applied to the movingVolumeNode.
# Ctrl + left-click-and-drag up/down to change opacity in slice views.
slicer.util.setSliceViewerLayers(background=fixedVolumeNode, foreground=movingVolumeNode, foregroundOpacity=0.5)
```

All parameters for BRAINSFit can be found [here](https://github.com/BRAINSia/BRAINSTools/blob/main/BRAINSFit/BRAINSFit.xml).
The module can register any kind of images, not just brains, but for general-purpose registration extensions consider using more robust registration tools, such as SlicerElastix and SlicerANTs extensions.
