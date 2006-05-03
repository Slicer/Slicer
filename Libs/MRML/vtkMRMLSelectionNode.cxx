/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSelectionNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMatrix4x4.h"

//------------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkMRMLSelectionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSelectionNode");
  if(ret)
    {
    return (vtkMRMLSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSelectionNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSelectionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSelectionNode");
  if(ret)
    {
    return (vtkMRMLSelectionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::vtkMRMLSelectionNode()
{
  this->ActiveVolumeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode::~vtkMRMLSelectionNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << "activeVolumeID=\"" << this->ActiveVolumeID << "\" ";

}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "activeVolumeID")) 
      {
      this->SetActiveVolumeID(attValue);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSelectionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSelectionNode *node = vtkMRMLSelectionNode::SafeDownCast(anode);

  this->SetActiveVolumeID(node->GetActiveVolumeID());
}

//----------------------------------------------------------------------------
void vtkMRMLSelectionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << "ActiveVolumeID: " << this->ActiveVolumeID << "\n";
}


// End
