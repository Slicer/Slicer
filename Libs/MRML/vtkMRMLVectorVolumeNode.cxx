/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLVectorVolumeNode* vtkMRMLVectorVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVectorVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeNode::vtkMRMLVectorVolumeNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeNode::~vtkMRMLVectorVolumeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVectorVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVectorVolumeNode *node = (vtkMRMLVectorVolumeNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


 
