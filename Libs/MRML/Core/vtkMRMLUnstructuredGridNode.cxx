/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"

// VTK includes
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"
#include <vtkVersion.h>

// STD includes

vtkCxxSetObjectMacro(vtkMRMLUnstructuredGridNode, UnstructuredGrid, vtkUnstructuredGrid);

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLUnstructuredGridNode);

vtkMRMLUnstructuredGridNode::vtkMRMLUnstructuredGridNode()
{
  this->UnstructuredGrid = NULL;
}

vtkMRMLUnstructuredGridNode::~vtkMRMLUnstructuredGridNode()
{
  if ( this->UnstructuredGrid)
    {
    this->SetAndObserveUnstructuredGrid(NULL);
    }
}

//-------------------------------
void vtkMRMLUnstructuredGridNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  if( this->UnstructuredGrid )
    {
    this->UnstructuredGrid->Print(os);
    }
  else
    {
    os << "UnstructuredGrid is NULL " << std::endl;
    }

}
//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLUnstructuredGridNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLUnstructuredGridNode *node = (vtkMRMLUnstructuredGridNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::SetAndObserveUnstructuredGrid(vtkUnstructuredGrid *unstructuredGrid)
{
if (this->UnstructuredGrid != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->UnstructuredGrid, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  vtkMTimeType mtime1, mtime2;
  mtime1 = this->GetMTime();
  this->SetUnstructuredGrid(unstructuredGrid);
  mtime2 = this->GetMTime();

  if (this->UnstructuredGrid != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->UnstructuredGrid, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  if (mtime1 != mtime2)
    {
    this->InvokeEvent( vtkCommand::ModifiedEvent , this);
    }
}

//-----------------------------------------------------------
void vtkMRMLUnstructuredGridNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  vtkMRMLUnstructuredGridDisplayNode *node  =
      vtkMRMLUnstructuredGridDisplayNode::SafeDownCast(
        scene ? scene->GetNodeByID(this->GetDisplayNodeID()) : 0);
  if (node)
    {
    // set input/output to/from display pipeline
    node->SetUnstructuredGrid(this->GetUnstructuredGrid());
    //this->SetAndObservePolyData(node->GetPolyData());
    }
}


//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
bool vtkMRMLUnstructuredGridNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridNode::ApplyTransform(vtkAbstractTransform* transform)
{
  vtkTransformFilter* transformFilter = vtkTransformFilter::New();
  transformFilter->SetInputData(this->GetUnstructuredGrid());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

//  this->SetAndObserveUnstructuredGrid(vtkUnstructuredGrid::SafeDownCast(transformFilter->GetOutput()));
  this->GetUnstructuredGrid()->DeepCopy(transformFilter->GetOutput());

  transformFilter->Delete();
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLUnstructuredGridNode::CreateDefaultStorageNode()
{
  return vtkMRMLUnstructuredGridStorageNode::New();
}

