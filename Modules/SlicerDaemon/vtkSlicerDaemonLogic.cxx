/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerDaemonLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkSlicerDaemonLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkSlicerDaemonLogic* vtkSlicerDaemonLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerDaemonLogic");
  if(ret)
    {
      return (vtkSlicerDaemonLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerDaemonLogic;
}


//----------------------------------------------------------------------------
vtkSlicerDaemonLogic::vtkSlicerDaemonLogic()
{
  SlicerDaemonNode = vtkMRMLSlicerDaemonNode::New();
  SlicerDaemon = vtkITKSlicerDaemon::New();
}

//----------------------------------------------------------------------------
vtkSlicerDaemonLogic::~vtkSlicerDaemonLogic()
{
  this->SlicerDaemon->Delete();
  this->SlicerDaemonNode->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerDaemonLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkSlicerDaemonLogic::Apply()
{
  // chack if MRML node is present 
  if (this->SlicerDaemonNode == NULL)
    {
    vtkErrorMacro("No input SlicerDaemonNode found");
    return;
    }
  
  // find input volume
  vtkMRMLNode* inNode = this->GetMRMLScene()->GetNodeByID(this->SlicerDaemonNode->GetInputVolumeRef());
  vtkMRMLScalarVolumeNode *inVolume =  dynamic_cast<vtkMRMLScalarVolumeNode *> (inNode);
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  this->SlicerDaemon->SetInput(inVolume->GetImageData());
  
  
  // set filter parameters
  this->SlicerDaemon->SetConductanceParameter(this->SlicerDaemonNode->GetConductance());
  this->SlicerDaemon->SetNumberOfIterations(this->SlicerDaemonNode->GetNumberOfIterations());
  this->SlicerDaemon->SetTimeStep(this->SlicerDaemonNode->GetTimeStep());
  
  // find output volume
  vtkMRMLScalarVolumeNode *outVolume = NULL;
  if (this->SlicerDaemonNode->GetOutputVolumeRef() != NULL)
    {
    vtkMRMLNode* outNode = this->GetMRMLScene()->GetNodeByID(this->SlicerDaemonNode->GetOutputVolumeRef());
    outVolume =  dynamic_cast<vtkMRMLScalarVolumeNode *> (outNode);
    if (outVolume == NULL)
      {
      vtkErrorMacro("No output volume found with id= " << this->SlicerDaemonNode->GetOutputVolumeRef());
      return;
      }
    }
  else 
    {
    // create new volume Node and add it to mrml scene
    this->GetMRMLScene()->SaveStateForUndo();
    outVolume = vtkMRMLScalarVolumeNode::New();
    this->GetMRMLScene()->AddNode(outVolume);  
    outVolume->Delete();
    }

  outVolume->SetImageData(this->SlicerDaemon->GetOutput());
  this->SlicerDaemon->Update();
}
