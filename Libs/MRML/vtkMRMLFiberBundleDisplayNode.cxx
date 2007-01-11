/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiberBundleDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiberBundleDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiberBundleDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::vtkMRMLFiberBundleDisplayNode()
{

  // Strings
  //this->Name = "FiberBundle";

  // Numbers
  
  // Arrays


  // Objects
  //this->TextureImageData = NULL;


}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{

  //this->SetAndObserveColorNodeID( NULL);

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFiberBundleDisplayNode *node = (vtkMRMLFiberBundleDisplayNode *) anode;

  // Strings
  //this->SetColor(node->Name);

  // Vectors

  
  // Numbers


}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  return;
}
