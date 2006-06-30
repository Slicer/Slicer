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
#include "vtkMRMLVolumeDisplayNode.h"

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
void vtkSlicerVolumesLogic::ProcessMRMLEvents()
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  this->SetMRML( vtkObjectPointer(&this->ActiveVolumeNode), activeNode );
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (char* filename)
{
  vtkMRMLVolumeNode *volumeNode = NULL;
  
  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();
  
  vtkMRMLVolumeDisplayNode *displayNode = vtkMRMLVolumeDisplayNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();

  storageNode->SetAbsoluteFileName(true);
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

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerVolumesLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
}

