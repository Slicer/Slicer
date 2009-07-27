/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLEMSTargetNode.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkMRMLEMSTargetNode.h"
#include <sstream>
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLEMSTargetNode* vtkMRMLEMSTargetNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");

  if (ret)
  {
    return (vtkMRMLEMSTargetNode*)ret;
  }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLEMSTargetNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");

  if (ret)
  {
    return (vtkMRMLEMSTargetNode*)ret;
  }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTargetNode::vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

//----------------------------------------------------------------------------
vtkMRMLEMSTargetNode::~vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTargetNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTargetNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  // we can assume an even number of elements

  const char* key;
  const char* val;

  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTargetNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
}

//----------------------------------------------------------------------------
void vtkMRMLEMSTargetNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

