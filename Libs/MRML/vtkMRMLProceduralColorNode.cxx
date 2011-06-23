/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLProceduralColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkObjectFactory.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

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

//-----------------------------------------------------------
vtkScalarsToColors* vtkMRMLProceduralColorNode::GetScalarsToColors()
{
  return this->GetColorTransferFunction();
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
bool vtkMRMLProceduralColorNode::SetNameFromColor(int index)
{
  double colour[4];
  this->GetColor(index, colour);
  //this->ColorTransferFunction->GetColor(index, colour);
  std::stringstream ss;
  ss.precision(3);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss << "R=";
  ss << colour[0];
  ss << " G=";
  ss << colour[1];
  ss << " B=";
  ss << colour[2];
  if (this->SetColorName(index, ss.str().c_str()) == 0)
    {
    vtkErrorMacro("SetNamesFromColors: error setting name " <<  ss.str().c_str() << " for color index " << index);
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLProceduralColorNode::GetNumberOfColors()
{
  /*
  double *range = this->ColorTransferFunction->GetRange();
  if (!range)
    {
    return 0;
    }
  int numPoints = static_cast<int>(floor(range[1] - range[0]));
  if (range[0] < 0 && range[1] >= 0)
    {
    // add one for zero
    numPoints++;
    }
  return numPoints;
  */
  return this->ColorTransferFunction->GetSize();
}

//---------------------------------------------------------------------------
bool vtkMRMLProceduralColorNode::GetColor(int entry, double* color)
{
  if (entry < 0 || entry >= this->GetNumberOfColors())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetNumberOfColors() << ", call SetNumberOfColors" << endl);
    return false;
    }
  /*
  double *range = this->ColorTransferFunction->GetRange();
  if (!range)
    {
    return false;
    }
  this->ColorTransferFunction->GetColor(range[0] + entry, color);
  color[3] = this->ColorTransferFunction->GetAlpha();
  return true;
  */
  double val[6];
  this->ColorTransferFunction->GetNodeValue(entry, val);
  color[0] = val[1]; // r
  color[1] = val[2]; // g
  color[2] = val[3]; // b
  color[3] = this->ColorTransferFunction->GetAlpha();
  return true;
}
