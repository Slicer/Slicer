
catch "t Delete"
catch "f Delete"
catch "app Delete"
catch "appLogic Delete"
catch "scene Delete"
catch "sliceGUI Delete"


vtkSlicerApplication app
vtkSlicerApplicationLogic appLogic
vtkMRMLScene scene
appLogic SetAndObserveMRMLScene scene

vtkKWTopLevel t
t SetApplication app
t Create

vtkKWFrame f
f SetParent t
f Create
pack [f GetWidgetName] -fill both -expand true

vtkSlicerSliceGUI sliceGUI
sliceGUI SetApplication app
sliceGUI SetApplicationLogic appLogic
sliceGUI SetAndObserveMRMLScene scene
sliceGUI BuildGUI f

t Display




set dicomArchetype [tk_getOpenFile]

catch "volumeNode Delete"
catch "displayNode Delete"
catch "storageNode Delete"

vtkMRMLScalarNode volumeNode
vtkMRMLVolumeDisplayNode displayNode
vtkMRMLVolumeArchetypeStorageNode storageNode

set dicomArchetype [tk_getOpenFile]
storageNode SetAbsoluteFileName $dicomArchetype


tk_messageBox -message "finished"

  storageNode->SetFileArchetype(filename);
  if (storageNode->ReadData(scalarNode) == 0)
    {
    // cannot read scalar data, try vector
    if (storageNode->ReadData(vectorNode) != 0)
      {
      volumeNode = vectorNode;
      }
    }
  else
    {
    volumeNode = scalarNode;
    }
  
  if (volumeNode != NULL)
    {
    const vtksys_stl::string fname(filename);
    vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
    volumeNode->SetName(name.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());

    double range[2];
    volumeNode->GetImageData()->GetScalarRange(range);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]) );

    this->GetMRMLScene()->AddNode(volumeNode);  
    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);  

    volumeNode->SetStorageNodeID(storageNode->GetID());

    volumeNode->SetDisplayNodeID(displayNode->GetID());
    
    this->SetActiveVolumeNode(volumeNode);
    
    this->Modified();  
    }

  scalarNode->Delete();
  vectorNode->Delete();
  storageNode->Delete();
  displayNode->Delete();

  return volumeNode;
}

