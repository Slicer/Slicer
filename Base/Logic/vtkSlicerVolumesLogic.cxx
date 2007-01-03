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

#include "vtkSlicerVolumesLogic.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

//#define USE_TEEM 1

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
void vtkSlicerVolumesLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode );
  this->Modified();
}
#ifdef USE_TEEM
//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (char* filename, int centerImage, int labelMap, const char* volname)
{
  cout<<"AddAchretypeVolume Intro"<<endl;
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
  storageNode2->SetFileName(filename);
  storageNode2->SetCenterImage(centerImage);
  
  // Try to read first with NRRD reader (look if file is a dwi or a tensor)
  cout<<"TEST DWI: "<< storageNode1->ReadData(dwiNode)<<endl;

  if (storageNode1->ReadData(dwiNode))
    {
    cout<<"DWI HAS BEEN READ"<<endl;
    displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
    // Give a chance to set/update displayNode
    volumeNode =  dwiNode;
    storageNode = storageNode1;
    cout<<"Done setting volumeNode to class: "<<volumeNode->GetClassName()<<endl;
    }
  else if (storageNode1->ReadData(tensorNode))
    {
    cout<<"Tensor HAS BEEN READ"<<endl;
    displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
    volumeNode = tensorNode;
    storageNode = storageNode1;
    }
  else if (storageNode1->ReadData(vectorNode))
    {
    cout<<"Vector HAS BEEN READ WITH NRRD READER"<<endl;
    displayNode = vtkMRMLVectorVolumeDisplayNode::New();
    volumeNode = vectorNode;
    storageNode = storageNode1;
    }
  else if (storageNode2->ReadData(scalarNode))
    {
    cout<<"Scalar HAS BEEN READ WITH ARCHTYPE READER"<<endl;
    displayNode = vtkMRMLVolumeDisplayNode::New();
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    storageNode = storageNode2;
    }
  else if (storageNode2->ReadData(vectorNode))
    {
    cout<<"Vector HAS BEEN READ WITH ARCHTYPE READER"<<endl;
    displayNode = vtkMRMLVectorVolumeDisplayNode::New();
    volumeNode = vectorNode;
    storageNode = storageNode2;
    }
  
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
    cout<<"Setting scene info"<<endl;
    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());
  
    //should we give the user the chance to modify this?.
    double range[2];
    cout<<"Set basic display info"<<endl;
    volumeNode->GetImageData()->GetScalarRange(range);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]) );

    cout<<"Adding node.."<<endl;
    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);  
    displayNode->SetDefaultColorMap();
    volumeNode->SetStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    cout<<"Name vol node "<<volumeNode->GetClassName()<<endl;
    cout<<"Display node "<<displayNode->GetClassName()<<endl;
    this->GetMRMLScene()->AddNode(volumeNode);
    cout<<"Node added to scene"<<endl;

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

#else

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (char* filename, int centerImage, int labelMap, const char* volname)
{
  cout<<"NO NRRD AddArchetypeVolume"<<endl;
  vtkMRMLVolumeNode *volumeNode = NULL;
  
  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();
  
  vtkMRMLVolumeDisplayNode *displayNode = vtkMRMLVolumeDisplayNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();

  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);
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
    displayNode->SetDefaultColorMap();
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


#endif

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::SaveArchetypeVolume (char* filename, vtkMRMLVolumeNode *volumeNode)
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

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerVolumesLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
}

