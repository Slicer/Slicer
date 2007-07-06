/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"

vtkMRMLUnstructuredGridNode::vtkMRMLUnstructuredGridNode()
{
  this->UnstructuredGrid = NULL;
}

vtkMRMLUnstructuredGridNode::~vtkMRMLUnstructuredGridNode()
{
  if ( this->UnstructuredGrid)
    {
    this->UnstructuredGrid->Delete();
    }
}

//------------------------------------------------------------------------------
vtkMRMLUnstructuredGridNode* vtkMRMLUnstructuredGridNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLUnstructuredGridNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLUnstructuredGridNode");
  if(ret)
    {
    return (vtkMRMLUnstructuredGridNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLUnstructuredGridNode;
}


//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  this->UnstructuredGrid->Print(os);

}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::SetAndObserveUnstructuredGrid(vtkUnstructuredGrid *unstructuredGrid)
{
if (this->UnstructuredGrid != NULL)
    {
    this->UnstructuredGrid->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  unsigned long mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetUnstructuredGrid(unstructuredGrid);
  mtime2 = this->GetMTime();

  if (this->UnstructuredGrid != NULL)
    {
    this->UnstructuredGrid->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  if (mtime1 != mtime2)
    {
    this->InvokeEvent( vtkCommand::ModifiedEvent , this);
    }
}


//-----------------------------------------------------------
void vtkMRMLUnstructuredGridNode::UpdateScene(vtkMRMLScene *scene)
{
  // this will call StorageNode::ReadData(this);
  Superclass::UpdateScene(scene);

  if (this->GetDisplayNodeID() == NULL) 
    {
    return;
    }

  vtkMRMLNode* mnode = scene->GetNodeByID(this->DisplayNodeID);
  if (mnode) 
    {
    vtkMRMLUnstructuredGridDisplayNode *node  = dynamic_cast < vtkMRMLUnstructuredGridDisplayNode *>(mnode);
    if (node)
      {
      // set input/output to/from display pipeline
      node->SetUnstructuredGrid(this->GetUnstructuredGrid());
      this->SetAndObservePolyData(node->GetPolyData());
      }
    }
}
