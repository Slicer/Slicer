/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLProceduralColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLScene.h"

#include "vtkColorTransferFunction.h"

//------------------------------------------------------------------------------
vtkMRMLProceduralColorNode* vtkMRMLProceduralColorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLProceduralColorNode");
  if(ret)
    {
    return (vtkMRMLProceduralColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLProceduralColorNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLProceduralColorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLProceduralColorNode");
  if(ret)
    {
    return (vtkMRMLProceduralColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLProceduralColorNode;
}


//----------------------------------------------------------------------------
vtkMRMLProceduralColorNode::vtkMRMLProceduralColorNode()
{

  this->Name = NULL;
  this->SetName("");
  this->FileName = NULL;

  this->ColorTransferFunction = NULL;
  this->ColorTransferFunction = vtkColorTransferFunction::New();
}

//----------------------------------------------------------------------------
vtkMRMLProceduralColorNode::~vtkMRMLProceduralColorNode()
{
  if (this->ColorTransferFunction)
    {
    this->ColorTransferFunction->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLProceduralColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLProceduralColorNode *node = (vtkMRMLProceduralColorNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  if (this->ColorTransferFunction != NULL)
    {
    os << indent << "ColorTransferFunction:" << endl;
    this->ColorTransferFunction->PrintSelf(os, indent.GetNextIndent());
    }
}

//-----------------------------------------------------------

void vtkMRMLProceduralColorNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::SetType(int type)
{
  vtkWarningMacro("Subclass didn't define SetType!");

  this->Type = type;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = " << this->GetTypeAsString());

  // Set up the custom colours here for this type
  
  // invoke a modified event
  this->Modified();
  
  // invoke a type  modified event
  this->InvokeEvent(vtkMRMLProceduralColorNode::TypeModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLProceduralColorNode::SetNamesFromColors()
{
  // get the transfer function's range and iterate over them
  if (this->ColorTransferFunction == NULL)
    {
    return;
    }
  double *range = this->ColorTransferFunction->GetRange();
  int numPoints = 0;
  double index = 0;
  if (range)
    {
    numPoints = (int)floor(range[1] - range[0]);
    if (range[0] < 0 && range[1] >= 0)
      {
      // add one for zero
      numPoints++;
      }
    index = range[0];
    }
  // reset the names
  this->Names.clear();
  this->Names.resize(numPoints);
  
  for (int i = 0; i < numPoints; i++)
    {
    double colour[3];
    double r = 0.0, g = 0.0, b = 0.0;
    this->ColorTransferFunction->GetColor(index, colour);
    r = colour[0]; //this->ColorTransferFunction->GetRedValue(i);
    g = colour[1]; //this->ColorTransferFunction->GetGreenValue(i);
    b = colour[2]; //this->ColorTransferFunction->GetBlueValue(i);
    std::stringstream ss;
    ss << "R=";
    ss << r;
    ss << " G=";
    ss << g;
    ss << " B=";
    ss << b;
    this->SetColorName(i, ss.str().c_str());
    index++;
    }
   this->NamesInitialisedOn();
}
