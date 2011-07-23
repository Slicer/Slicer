/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"

#include "vtkMRMLTimeSeriesVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

//------------------------------------------------------------------------------
vtkMRMLTimeSeriesVolumeNode* vtkMRMLTimeSeriesVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTimeSeriesVolumeNode");
  if(ret)
    {
    return (vtkMRMLTimeSeriesVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTimeSeriesVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLTimeSeriesVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTimeSeriesVolumeNode");
  if(ret)
    {
    return (vtkMRMLTimeSeriesVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTimeSeriesVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLTimeSeriesVolumeNode::vtkMRMLTimeSeriesVolumeNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLTimeSeriesVolumeNode::~vtkMRMLTimeSeriesVolumeNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

//  while (*atts != NULL)
//    {
//    }

} 

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTimeSeriesVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLTimeSeriesVolumeNode *node = (vtkMRMLTimeSeriesVolumeNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTimeSeriesVolumeNode::CreateDefaultStorageNode()
{
  // Superclass::CreateDefaultStorageNode();
  return vtkMRMLVolumeArchetypeStorageNode::New();
}
