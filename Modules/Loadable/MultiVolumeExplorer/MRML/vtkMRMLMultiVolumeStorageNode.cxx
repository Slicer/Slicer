/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNRRDStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkMRMLMultiVolumeStorageNode.h"
#include "vtkMRMLMultiVolumeNode.h"

#include "vtkObjectFactory.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeStorageNode::vtkMRMLMultiVolumeStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeStorageNode::~vtkMRMLMultiVolumeStorageNode()
{
}

//----------------------------------------------------------------------------
bool vtkMRMLMultiVolumeStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return this->vtkMRMLNRRDStorageNode::CanReadInReferenceNode(refNode) || 
    refNode->IsA("vtkMRMLMultiVolumeNode");
}
