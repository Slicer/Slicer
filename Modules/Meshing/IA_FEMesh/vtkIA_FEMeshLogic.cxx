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

#include "vtkMRMLFiniteElementImageNode.h"
#include "vtkMRMLFiniteElementBuildingBlockNode.h"
#include "vtkMRMLFiniteElementMeshNode.h"
#include "vtkMRMLFESurfaceNode.h"

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
  this->First = true;
}

//----------------------------------------------------------------------------
vtkIA_FEMeshLogic::~vtkIA_FEMeshLogic()
{
  this->IA_FEMeshNode->Delete();
}

void vtkIA_FEMeshLogic::SetMRMLScene(vtkMRMLScene *mrml)
{
   cout << "IA-FEMesh Logic: Set MRML Scene" << endl;
   vtkSlicerModuleLogic::SetMRMLScene(mrml);
   this->RegisterNodes();
}

void vtkIA_FEMeshLogic::RegisterNodes()
{
 if (this->MRMLScene && this->First)
 {
   vtkMRMLFiniteElementImageNode *imgNode = vtkMRMLFiniteElementImageNode::New();
   this->MRMLScene->RegisterNodeClass(imgNode);  imgNode->Delete();
   vtkMRMLFESurfaceNode *sNode = vtkMRMLFESurfaceNode::New();
   this->MRMLScene->RegisterNodeClass(sNode);  sNode->Delete();
   vtkMRMLFiniteElementBuildingBlockNode *bbNode = vtkMRMLFiniteElementBuildingBlockNode::New();
   this->MRMLScene->RegisterNodeClass(bbNode);  bbNode->Delete();
   vtkMRMLFiniteElementMeshNode *mNode = vtkMRMLFiniteElementMeshNode::New();
   this->MRMLScene->RegisterNodeClass(mNode);  mNode->Delete();
   this->First = false;
 }
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
}


