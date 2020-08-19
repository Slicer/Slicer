# Volume rendering

## Key classes
- [vtkMRMLVolumeRenderingDisplayNode](http://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayNode.html) controls the volume rendering properties. Each volume rendering technique has its own subclass.
- [vtkSlicerVolumeRenderingLogic](http://apidocs.slicer.org/master/classvtkSlicerVolumeRenderingLogic.html) contains utility functions
- [vtkMRMLScalarVolumeNode](http://apidocs.slicer.org/master/classvtkMRMLScalarVolumeNode.html) contains the volume itself
- [vtkMRMLVolumePropertyNode](http://apidocs.slicer.org/master/classvtkMRMLVolumePropertyNode.html) points to the transfer functions
- [vtkMRMLAnnotationROINode](http://apidocs.slicer.org/master/classvtkMRMLAnnotationROINode.html) controls the clipping planes
- [vtkMRMLVolumeRenderingDisplayableManager](http://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayableManager.html) responsible for adding VTK actors to the renderers

## Format of Volume Property (.vp) file

Volume properties, separated by newline characters.

Example:

```
1 => interpolation type
1 => shading enabled
0.9 => diffuse reflection
0.1 => ambient reflection
0.2 => specular reflection
10 => specular reflection power
14 -3024 0 -86.9767 0 45.3791 0.169643 139.919 0.589286 347.907 0.607143 1224.16 0.607143 3071 0.616071 => scalar opacity transfer function (total number of values, each control point is defined by a pair of values: intensity and opacity)
4 0 1 255 1 => gradient opacity transfer function (total number of values, each control point is defined by a pair of values: intensity gradient and opacity)
28 -3024 0 0 0 -86.9767 0 0.25098 1 45.3791 1 0 0 139.919 1 0.894893 0.894893 347.907 1 1 0.25098 1224.16 1 1 1 3071 0.827451 0.658824 1 => color transfer function (total number of values, each control point is defined by 4 of values: intensity and R, G, B color components)
```

## How Tos

### How to programmatically volume render your volume node

C++

```
qSlicerAbstractCoreModule* volumeRenderingModule =
  qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
  volumeRenderingModule ? vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic()) : 0;
vtkMRMLVolumeNode* volumeNode = mrmlScene->GetNodeByID('vtkMRMLScalarVolumeNode1');
if (volumeRenderingLogic)
  {
  vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> displayNode =
    vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(volumeRenderingLogic->CreateVolumeRenderingDisplayNode());
  mrmlScene->AddNode(displayNode);
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  volumeRenderingLogic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode);
  }
```

Python

```
logic = slicer.modules.volumerendering.logic()
volumeNode = slicer.mrmlScene.GetNodeByID('vtkMRMLScalarVolumeNode1')
displayNode = logic.CreateVolumeRenderingDisplayNode()
displayNode.UnRegister(logic)
slicer.mrmlScene.AddNode(displayNode)
volumeNode.AddAndObserveDisplayNodeID(displayNode.GetID())
logic.UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode)
```

### How to programmatically apply a custom color/opacity transfer function

C++

```
vtkColorTransferFunction* colors = ...
vtkPiecewiseFunction* opacities = ...
vtkMRMLVolumeRenderingDisplayNode* displayNode = ...
vtkMRMLVolumePropertyNode* propertyNode = displayNode->GetVolumePropertyNode();
propertyNode->SetColor(colorTransferFunction);
propertyNode->SetScalarOpacity(opacities);
// optionally set the gradients opacities with SetGradientOpacity
The logic has utility functions to help you create those transfer functions:
volumeRenderingLogic->[http://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#ab8dbda38ad81b39b445b01e1bf8c7a86 SetWindowLevelToVolumeProp](...)
volumeRenderingLogic->[http://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#a1dcbe614493f3cbb9aa50c68a64764ca SetThresholdToVolumeProp](...)
volumeRenderingLogic->[http://slicer.org/doc/html/classvtkSlicerVolumeRenderingLogic.html#a359314889c2b386fd4c3ffe5414522da SetLabelMapToVolumeProp](...)
```

Python

```
propertyNode = displayNode.GetVolumePropertyNode()
...
```

### How to programmatically limit volume rendering to a subset of the volume

C++

```
vtkMRMLAnnotationROINode]* roiNode =...
vtkMRMLVolumeRenderingDisplayNode* displayNode = ...
displayNode->SetAndObserveROINodeID(roiNode->GetID());
displayNode->SetCroppingEnabled(1);
```

Python

```
displayNode.SetAndObserveROINodeID(roiNode.GetID())
displayNode.CroppingEnabled = 1
```

### How to register a new Volume Rendering mapper

You need to derive from [vtkMRMLVolumeRenderingDisplayNode](http://apidocs.slicer.org/master/classvtkMRMLVolumeRenderingDisplayNode.html) and register your class within [vtkSlicerVolumeRenderingLogic](http://apidocs.slicer.org/master/classvtkSlicerVolumeRenderingLogic.html).

```
void qSlicerMyABCVolumeRenderingModule::setup()
{
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLMyABCVolumeRenderingDisplayableManager");

  this->Superclass::setup();

  qSlicerAbstractCoreModule* volumeRenderingModule =
    qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
  if (volumeRenderingModule)
    {
    vtkNew<vtkMRMLMyABCVolumeRenderingDisplayNode> displayNode;
    vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
      vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic());
    volumeRenderingLogic->RegisterRenderingMethod(
      "My ABC Volume Rendering", displayNode->GetClassName());
    }
  else
    {
    qWarning() << "Volume Rendering module is not found";
    }
}
```

If you want to expose control widgets for your volume rendering method, then register your widget with [addRenderingMethodWidget()](http://apidocs.slicer.org/master/classqSlicerVolumeRenderingModuleWidget.html#acd9cdb60f1fd260f3ebf74428bb7c45b).

### How to register custom volume rendering presets

Custom presets can be added to the volume rendering module by calling AddPreset() method of the volume rendering module logic. The example below shows how to define multiple custom volume rendering presets in an external MRML scene file and add them to the volume rendering module user interface.

Create a *MyPresets.mrml* file that describes two custom volume rendering presets:

```
<MRML version="Slicer4.4.0">
  <VolumeProperty id="vtkMRMLVolumeProperty1" name="MyPreset1"     references="IconVolume:vtkMRMLVectorVolumeNode1;" interpolation="1" shade="1" diffuse="0.66" ambient="0.1" specular="0.62" specularPower="14" scalarOpacity="10 -3.52844023704529 0 56.7852325439453 0 79.2550277709961 0.428571432828903 415.119384765625 1 641 1" gradientOpacity="4 0 1 160.25 1" colorTransfer="16 0 0 0 0 98.7223 0.196078431372549 0.945098039215686 0.956862745098039 412.406 0 0.592157 0.807843 641 1 1 1" />
  <VectorVolume id="vtkMRMLVectorVolumeNode1" references="storage:vtkMRMLVolumeArchetypeStorageNode1;" />
  <VolumeArchetypeStorage id="vtkMRMLVolumeArchetypeStorageNode1" fileName="MyPreset1.png"  fileListMember0="MyPreset1.png" />

  <VolumeProperty id="vtkMRMLVolumeProperty2" name="MyPreset2"     references="IconVolume:vtkMRMLVectorVolumeNode2;" interpolation="1" shade="1" diffuse="0.66" ambient="0.1" specular="0.62" specularPower="14" scalarOpacity="10 -3.52844023704529 0 56.7852325439453 0 79.2550277709961 0.428571432828903 415.119384765625 1 641 1" gradientOpacity="4 0 1 160.25 1" colorTransfer="16 0 0 0 0 98.7223 0.196078431372549 0.945098039215686 0.956862745098039 412.406 0 0.592157 0.807843 641 1 1 1" />
  <VectorVolume id="vtkMRMLVectorVolumeNode2" references="storage:vtkMRMLVolumeArchetypeStorageNode2;" />
  <VolumeArchetypeStorage id="vtkMRMLVolumeArchetypeStorageNode2" fileName="MyPreset2.png"  fileListMember0="MyPreset2.png" />
</MRML>
```

For this example, thumbnail images for the presets should be located in the same directory as *MyPresets.mrml*, with the file names *MyPreset1.png* and *MyPreset2.png*.

Use the following code to read all the custom presets from *MyPresets.mrml* and load it into the scene:

```
presetsScenePath = "MyPresets.mrml"

# Read presets scene
customPresetsScene = slicer.vtkMRMLScene()
vrPropNode = slicer.vtkMRMLVolumePropertyNode()
customPresetsScene.RegisterNodeClass(vrPropNode)
customPresetsScene.SetURL(presetsScenePath)
customPresetsScene.Connect()

# Add presets to volume rendering logic
vrLogic = slicer.modules.volumerendering.logic()
presetsScene = vrLogic.GetPresetsScene()
vrNodes = customPresetsScene.GetNodesByClass("vtkMRMLVolumePropertyNode")
vrNodes.UnRegister(None)
for itemNum in range(vrNodes.GetNumberOfItems()):
  node = vrNodes.GetItemAsObject(itemNum)
  vrLogic.AddPreset(node)
```
