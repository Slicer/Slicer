/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkImageThreshold.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerColorLogic.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"


#ifdef USE_TEEM
  #include "vtkMRMLNRRDStorageNode.h"
#endif

#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerVolumesLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumesLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::vtkSlicerVolumesLogic()
{
  this->ActiveVolumeNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::~vtkSlicerVolumesLogic()
{
  if (this->ActiveVolumeNode != NULL)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessMRMLEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessLogicEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode );
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddHeaderVolume (const char* filename, int centerImage, int labelMap, const char* volname, 
                                                           vtkMRMLVolumeHeaderlessStorageNode *headerStorage)
{
  vtkMRMLVolumeNode *volumeNode = NULL;
  vtkMRMLVolumeDisplayNode *displayNode = NULL;

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();

  vtkMRMLVolumeHeaderlessStorageNode *storageNode = vtkMRMLVolumeHeaderlessStorageNode::New();
  storageNode->Copy(headerStorage);
  
  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

 if (storageNode->ReadData(scalarNode))
    {
    displayNode = vtkMRMLVolumeDisplayNode::New();
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    }
  else if (storageNode->ReadData(vectorNode))
    {
    displayNode = vtkMRMLVectorVolumeDisplayNode::New();
    volumeNode = vectorNode;
    }

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
      }
    else
      {
      volumeNode->SetName(volname);
      }

    this->GetMRMLScene()->SaveStateForUndo();
    vtkDebugMacro("Setting scene info");
    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());
  
    //should we give the user the chance to modify this?.
    double range[2];
    vtkDebugMacro("Set basic display info");
    volumeNode->GetImageData()->GetScalarRange(range);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]) );

    vtkDebugMacro("Adding node..");
    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);  

    //displayNode->SetDefaultColorMap();
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    if (labelMap) 
      {
      if (this->IsFreeSurferVolume(filename))
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
        }
      }
    else
      {
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
      }
    colorLogic->Delete();
    
    volumeNode->SetStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
    this->GetMRMLScene()->AddNode(volumeNode);
    vtkDebugMacro("Node added to scene");

    this->SetActiveVolumeNode(volumeNode);

    this->Modified();
    }

  scalarNode->Delete();
  vectorNode->Delete();
  storageNode->Delete();
  if (displayNode)
    {
    displayNode->Delete();
    }
  return volumeNode;
}

#ifdef USE_TEEM
//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (const char* filename, int centerImage, int labelMap, const char* volname)
{
  vtkMRMLVolumeNode *volumeNode = NULL;
  vtkMRMLVolumeDisplayNode *displayNode = NULL;

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();
  vtkMRMLDiffusionTensorVolumeNode *tensorNode = vtkMRMLDiffusionTensorVolumeNode::New();
  vtkMRMLDiffusionWeightedVolumeNode *dwiNode = vtkMRMLDiffusionWeightedVolumeNode::New();

  // Instanciation of the two I/O mechanism
  vtkMRMLNRRDStorageNode *storageNode1 = vtkMRMLNRRDStorageNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = vtkMRMLVolumeArchetypeStorageNode::New();
  vtkMRMLStorageNode *storageNode = NULL;
  
  storageNode1->SetFileName(filename);
  storageNode1->SetCenterImage(centerImage);
  storageNode1->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  storageNode2->SetFileName(filename);
  storageNode2->SetCenterImage(centerImage);
  storageNode2->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  // Try to read first with NRRD reader (look if file is a dwi or a tensor)
  vtkDebugMacro("TEST DWI: "<< storageNode1->ReadData(dwiNode));

  if (storageNode1->ReadData(dwiNode))
    {
    vtkDebugMacro("DWI HAS BEEN READ");
    displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
    // Give a chance to set/update displayNode
    volumeNode =  dwiNode;
    storageNode = storageNode1;
    vtkDebugMacro("Done setting volumeNode to class: "<<volumeNode->GetClassName());
    }
  else if (storageNode1->ReadData(tensorNode))
    {
    vtkDebugMacro("Tensor HAS BEEN READ");
    displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
    volumeNode = tensorNode;
    storageNode = storageNode1;
    }
  else if (storageNode1->ReadData(vectorNode))
    {
    vtkDebugMacro("Vector HAS BEEN READ WITH NRRD READER");
    displayNode = vtkMRMLVectorVolumeDisplayNode::New();
    volumeNode = vectorNode;
    storageNode = storageNode1;
    }
  else if (storageNode2->ReadData(vectorNode) && vectorNode->GetImageData()->GetNumberOfScalarComponents() == 3)
    {
    vtkDebugMacro("Vector HAS BEEN READ WITH ARCHTYPE READER");
    displayNode = vtkMRMLVectorVolumeDisplayNode::New();
    volumeNode = vectorNode;
    storageNode = storageNode2;
    }
  else if (storageNode2->ReadData(scalarNode))
    {
    vtkDebugMacro("Scalar HAS BEEN READ WITH ARCHTYPE READER");
    displayNode = vtkMRMLVolumeDisplayNode::New();
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    storageNode = storageNode2;
    }

  storageNode1->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);
  storageNode2->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
      }
    else
      {
      volumeNode->SetName(volname);
      }

    this->GetMRMLScene()->SaveStateForUndo();
    vtkDebugMacro("Setting scene info");
    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());
  
    //should we give the user the chance to modify this?.
    double range[2];
    vtkDebugMacro("Set basic display info");
    volumeNode->GetImageData()->GetScalarRange(range);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]) );

    vtkDebugMacro("Adding node..");
    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);  

    //displayNode->SetDefaultColorMap();
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    if (labelMap) 
      {
      if (this->IsFreeSurferVolume(filename))
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
        }
      }
    else
      {
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
      }
    colorLogic->Delete();
    
    volumeNode->SetStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
    this->GetMRMLScene()->AddNode(volumeNode);
    vtkDebugMacro("Node added to scene");

    this->SetActiveVolumeNode(volumeNode);

    this->Modified();
    }

  scalarNode->Delete();
  vectorNode->Delete();
  dwiNode->Delete();
  tensorNode->Delete();
  storageNode1->Delete();
  storageNode2->Delete();
  if (displayNode)
    {
    displayNode->Delete();
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLNRRDStorageNode *storageNode1 = NULL;
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = NULL;
  vtkMRMLStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = volumeNode->GetStorageNode();

  if (snode != NULL)
    {
    storageNode2 = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(snode);
    storageNode1 = vtkMRMLNRRDStorageNode::SafeDownCast(snode);
    }

  // Use NRRD writer if we are dealing with DWI, DTI or vector volumes

  if (volumeNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
      volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") ||
      volumeNode->IsA("vtkMRMLVectorVolumeNode"))
    {

    if (storageNode1 == NULL)
      {
      storageNode1 = vtkMRMLNRRDStorageNode::New();
      storageNode1->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode1);
      volumeNode->SetStorageNodeID(storageNode1->GetID());
      storageNode1->Delete();
      }

    storageNode1->SetFileName(filename);
    storageNode = storageNode1;
    }
  else
    {
    if (storageNode2 == NULL)
      {
      storageNode2 = vtkMRMLVolumeArchetypeStorageNode::New();
      storageNode2->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode2);
      volumeNode->SetStorageNodeID(storageNode2->GetID());
      storageNode2->Delete();
      }

    storageNode2->SetFileName(filename);
    storageNode = storageNode2;
    }

  int res = storageNode->WriteData(volumeNode);
  return res;
}



#else

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (const char* filename, int centerImage, int labelMap, const char* volname)
{
  vtkMRMLVolumeNode *volumeNode = NULL;
  
  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();
  
  vtkMRMLVolumeDisplayNode *displayNode = vtkMRMLVolumeDisplayNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();

  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (storageNode->ReadData(scalarNode))
    {
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    }
  else if (storageNode->ReadData(vectorNode))
    {
    // cannot read scalar data, try vector
    volumeNode = vectorNode;
    }

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
      }
    else
      {
      volumeNode->SetName(volname);
      }

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

    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);
    int isLabelMap = 0;
    if (vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode))
      {
      isLabelMap = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->GetLabelMap();
      }
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    if (colorLogic)
      {
      if (isLabelMap)
        {
        if (this->IsFreeSurferVolume(filename))
          {
          displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
          }
        else
          {
          displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
          }
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
        }
      colorLogic->Delete();
      }
   
    volumeNode->SetStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());    

    this->GetMRMLScene()->AddNode(volumeNode);  

    this->SetActiveVolumeNode(volumeNode);
    
    this->Modified();  
    }

  scalarNode->Delete();
  vectorNode->Delete();
  storageNode->Delete();
  displayNode->Delete();

  return volumeNode;
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLVolumeArchetypeStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = volumeNode->GetStorageNode();
  if (snode != NULL)
    {
    storageNode = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(snode);
    }
  if (storageNode == NULL)
    {
    storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
    storageNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(storageNode);  
    volumeNode->SetStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    }

  storageNode->SetFileName(filename);

  int res = storageNode->WriteData(volumeNode);

  
  return res;
}

#endif

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkSlicerVolumesLogic::CreateLabelVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, char *name)
{
  if ( volumeNode == NULL ) 
    {
    return NULL;
    }

  // create a display node
  vtkMRMLVolumeDisplayNode *labelDisplayNode  = vtkMRMLVolumeDisplayNode::New();

  scene->AddNode(labelDisplayNode);

  // create a volume node as copy of source volume
  vtkMRMLScalarVolumeNode *labelNode = vtkMRMLScalarVolumeNode::New();
  labelNode->Copy(volumeNode);
  labelNode->SetStorageNodeID(NULL);
  labelNode->SetModifiedSinceRead(1);
  labelNode->SetLabelMap(1);

  // set the display node to have a label map lookup table
  labelDisplayNode->SetAndObserveColorNodeID ("vtkMRMLColorTableNodeLabels");
  labelNode->SetName(name);
  labelNode->SetAndObserveDisplayNodeID( labelDisplayNode->GetID() );

  // make an image data of the same size and shape as the input volume,
  // but filled with zeros
  vtkImageThreshold *thresh = vtkImageThreshold::New();
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetInput( volumeNode->GetImageData() );
  thresh->GetOutput()->Update();
  labelNode->SetAndObserveImageData( thresh->GetOutput() );
  thresh->Delete();

  // add the label volume to the scene
  scene->AddNode(labelNode);

  labelNode->Delete();
  labelDisplayNode->Delete();

  return (labelNode);
}


//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerVolumesLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::IsFreeSurferVolume (const char* filename)
{
  std::string fname(filename);
  std::string::size_type loc = fname.find(".");
  if (loc != std::string::npos)
    {
    std::string extension = fname.substr(loc);
    if (extension == std::string(".mgz") ||
        extension == std::string(".mgh") ||
        extension == std::string(".mgh.gz"))
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
  else
    {
    return 0;
    }
}
