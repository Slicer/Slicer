/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleLineDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode* vtkMRMLFiberBundleLineDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleLineDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleLineDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleLineDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleLineDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleLineDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleLineDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleLineDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode::vtkMRMLFiberBundleLineDisplayNode()
{

}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode::~vtkMRMLFiberBundleLineDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleLineDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiberBundleLineDisplayNode *node = (vtkMRMLFiberBundleLineDisplayNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
}

 


