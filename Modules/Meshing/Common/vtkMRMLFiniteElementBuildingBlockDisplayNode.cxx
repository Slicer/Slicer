/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiniteElementBuildingBlockDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkTubeFilter.h"
#include "vtkFeatureEdges.h"

#include "vtkMRMLFiniteElementBuildingBlockDisplayNode.h"
#include "vtkMRMLScene.h"
//#include "vtkMimxBuildingBlockSource.h"

//------------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockDisplayNode* vtkMRMLFiniteElementBuildingBlockDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementBuildingBlockDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiniteElementBuildingBlockDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiniteElementBuildingBlockDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiniteElementBuildingBlockDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiniteElementBuildingBlockDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiniteElementBuildingBlockDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiniteElementBuildingBlockDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockDisplayNode::vtkMRMLFiniteElementBuildingBlockDisplayNode()
{
  this->ShrinkFactor = 1.0;  
  this->FeatureEdges = vtkFeatureEdges::New();
  this->OutlineTube = vtkTubeFilter::New();
}


//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiniteElementBuildingBlockDisplayNode::GetPolyData()
{
   // we know these building blocks will always be in wireframe mode, so force outlines
  this->FeatureEdges->SetInput(this->GeometryFilter->GetOutput());
  this->FeatureEdges->BoundaryEdgesOn();
  this->FeatureEdges->ManifoldEdgesOn();
  this->FeatureEdges->FeatureEdgesOff();
  this->OutlineTube->SetInput(this->FeatureEdges->GetOutput());
  this->OutlineTube->SetRadius(0.15);
  return this->OutlineTube->GetOutput();  
}


//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockDisplayNode::UpdatePolyDataPipeline() 
{
  //this->ShrinkPolyData->SetShrinkFactor(this->ShrinkFactor);
};
 

//----------------------------------------------------------------------------
vtkMRMLFiniteElementBuildingBlockDisplayNode::~vtkMRMLFiniteElementBuildingBlockDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->FeatureEdges->Delete();
  this->OutlineTube->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  //of << indent << " shrinkFactor =\"" << this->ShrinkFactor << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockDisplayNode::ReadXMLAttributes(const char** atts)
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
void vtkMRMLFiniteElementBuildingBlockDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiniteElementBuildingBlockDisplayNode *node = (vtkMRMLFiniteElementBuildingBlockDisplayNode *) anode;
  this->SetShrinkFactor(node->ShrinkFactor);
}

//----------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
//  os << indent << "ShrinkFactor:             " << this->ShrinkFactor << "\n";
}


//---------------------------------------------------------------------------
void vtkMRMLFiniteElementBuildingBlockDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}
