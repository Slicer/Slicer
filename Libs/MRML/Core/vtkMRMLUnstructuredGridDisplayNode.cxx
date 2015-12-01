/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLUnstructuredGridDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <sstream>

#include "vtkCallbackCommand.h"
#include "vtkGeometryFilter.h"
#include "vtkObjectFactory.h"
#include "vtkShrinkPolyData.h"
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>

#include "vtkMRMLUnstructuredGridDisplayNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLUnstructuredGridDisplayNode);


//----------------------------------------------------------------------------
vtkMRMLUnstructuredGridDisplayNode::vtkMRMLUnstructuredGridDisplayNode()
{
  this->GeometryFilter = vtkGeometryFilter::New();
  this->ShrinkPolyData = vtkShrinkPolyData::New();

  this->ShrinkPolyData->SetInputConnection( this->GeometryFilter->GetOutputPort());
  this->ShrinkFactor = 0.5;
  this->ShrinkPolyData->SetShrinkFactor(this->ShrinkFactor);
}


//----------------------------------------------------------------------------
vtkMRMLUnstructuredGridDisplayNode::~vtkMRMLUnstructuredGridDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->GeometryFilter->Delete();
  this->ShrinkPolyData->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " shrinkFactor =\"" << this->ShrinkFactor << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "shrinkFactor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ShrinkFactor;
      }
    }

  this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLUnstructuredGridDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLUnstructuredGridDisplayNode *node = (vtkMRMLUnstructuredGridDisplayNode *) anode;
  this->SetShrinkFactor(node->ShrinkFactor);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;

  Superclass::PrintSelf(os,indent);
  os << indent << "ShrinkFactor:             " << this->ShrinkFactor << "\n";
}


//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::SetUnstructuredGrid(vtkUnstructuredGrid *grid)
{
  if (this->GeometryFilter)
    {
    this->GeometryFilter->SetInputData(grid);
    }
}

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLUnstructuredGridDisplayNode::GetPolyData()
{
  if (this->ShrinkPolyData)
    {
    this->ShrinkPolyData->Update();
    return this->ShrinkPolyData->GetOutput();
    }
  else
    {
    return NULL;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLUnstructuredGridDisplayNode::UpdatePolyDataPipeline()
{
  this->ShrinkPolyData->SetShrinkFactor(this->ShrinkFactor);
}
