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
int vtkMRMLProceduralColorNode::ReadFile()
{
  vtkWarningMacro("Subclass didn't define ReadFile!");
  return 0;
}
