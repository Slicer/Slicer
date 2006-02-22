/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLModelNode.h"

vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerApplicationLogic);


//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
    this->MRMLScene = vtkMRMLScene::New();

    vtkMRMLVolumeNode *volumenode = vtkMRMLVolumeNode::New(); 
    this->MRMLScene->RegisterNodeClass( volumenode );

    vtkMRMLModelNode *modelnode = vtkMRMLModelNode::New(); 
    this->MRMLScene->RegisterNodeClass( modelnode );

    this->Views = NULL;
    this->Slices = NULL;
    this->Modules = NULL;
    this->ActiveSlice = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::~vtkSlicerApplicationLogic()
{
  // TODO - unregister/delete ivars
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerApplicationLogic:             " << this->GetClassName() << "\n"; } 
