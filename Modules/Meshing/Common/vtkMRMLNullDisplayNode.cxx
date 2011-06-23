/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNullDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkPolyData.h"

#include "vtkMRMLNullDisplayNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLNullDisplayNode* vtkMRMLNullDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNullDisplayNode");
  if(ret)
    {
    return (vtkMRMLNullDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNullDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLNullDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLNullDisplayNode");
  if(ret)
    {
    return (vtkMRMLNullDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLNullDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLNullDisplayNode::vtkMRMLNullDisplayNode()
{
  this->NullDataset = vtkPolyData::New();
}


//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLNullDisplayNode::GetPolyData()
{
   return this->NullDataset;  
}


//----------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::UpdatePolyDataPipeline() 
{

};
 

//----------------------------------------------------------------------------
vtkMRMLNullDisplayNode::~vtkMRMLNullDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->NullDataset->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);


}


//----------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::SetUnstructuredGrid(vtkUnstructuredGrid *grid)
{
  // don't do anything because we don't need a dataset

}


//----------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

//    if (!strcmp(attName, "shrinkFactor")) 
//      {
//      std::stringstream ss;
//      ss << attValue;
//      ss >> ShrinkFactor;
//      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLNullDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);

}


//---------------------------------------------------------------------------
void vtkMRMLNullDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}
