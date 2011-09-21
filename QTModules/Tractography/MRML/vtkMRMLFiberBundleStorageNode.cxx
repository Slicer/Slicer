/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"
#include "vtkMRMLFiberBundleStorageNode.h"



//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleStorageNode);

//----------------------------------------------------------------------------
int vtkMRMLFiberBundleStorageNode::SupportedFileType(const char *fileName)
{
  return this->Superclass::SupportedFileType(fileName);
  //return 0;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleStorageNode::InitializeSupportedWriteFileTypes()
{
  this->Superclass::InitializeSupportedWriteFileTypes();
}
