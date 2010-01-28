/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkIA_FEMeshLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkIA_FEMeshLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkIA_FEMeshLogic* vtkIA_FEMeshLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkIA_FEMeshLogic");
  if(ret)
    {
      return (vtkIA_FEMeshLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIA_FEMeshLogic;
}


//----------------------------------------------------------------------------
vtkIA_FEMeshLogic::vtkIA_FEMeshLogic()
{
  IA_FEMeshNode = vtkMRMLIA_FEMeshNode::New();

}

//----------------------------------------------------------------------------
vtkIA_FEMeshLogic::~vtkIA_FEMeshLogic()
{
  this->IA_FEMeshNode->Delete();
}

//----------------------------------------------------------------------------
void vtkIA_FEMeshLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

void vtkIA_FEMeshLogic::Apply()
{
  // chack if MRML node is present 
  if (this->IA_FEMeshNode == NULL)
    {
    vtkErrorMacro("No input IA_FEMeshNode found");
    return;
    }
  
  // find input volume
//  vtkMRMLNode* inNode = this->GetMRMLScene()->GetNodeByID(this->IA_FEMeshNode->GetInputVolumeRef());
//  vtkMRMLScalarVolumeNode *inVolume =  dynamic_cast<vtkMRMLScalarVolumeNode *> (inNode);
//  if (inVolume == NULL)
//    {
//    vtkErrorMacro("No input volume found");
//    return;
//    }
//  
  //this->IA_FEMesh->SetInput(inVolume->GetImageData());
  
  
  // set filter parameters
  // *** we don't have a subobject like the slicer daemon example had
  //this->IA_FEMesh->SetConductanceParameter(this->IA_FEMeshNode->GetConductance());
  //this->IA_FEMesh->SetNumberOfIterations(this->IA_FEMeshNode->GetNumberOfIterations());
  //this->IA_FEMesh->SetTimeStep(this->IA_FEMeshNode->GetTimeStep());
  
  // find output volume
//  vtkMRMLScalarVolumeNode *outVolume = NULL;
//  if (this->IA_FEMeshNode->GetOutputVolumeRef() != NULL)
//    {
//    vtkMRMLNode* outNode = this->GetMRMLScene()->GetNodeByID(this->IA_FEMeshNode->GetOutputVolumeRef());
//    outVolume =  dynamic_cast<vtkMRMLScalarVolumeNode *> (outNode);
//    if (outVolume == NULL)
//      {
//      vtkErrorMacro("No output volume found with id= " << this->IA_FEMeshNode->GetOutputVolumeRef());
//      return;
//      }
//    }
//  else 
//    {
//    // create new volume Node and add it to mrml scene
//    this->GetMRMLScene()->SaveStateForUndo();
//    outVolume = vtkMRMLScalarVolumeNode::New();
//    this->GetMRMLScene()->AddNode(outVolume);  
//    outVolume->Delete();
//    }

  //outVolume->SetImageData(this->IA_FEMesh->GetOutput());
  //this->IA_FEMesh->Update();
}
