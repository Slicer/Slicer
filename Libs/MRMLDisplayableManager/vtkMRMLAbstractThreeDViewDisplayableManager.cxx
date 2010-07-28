/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractThreeDViewDisplayableManager.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractThreeDViewDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
// vtkMRMLAbstractThreeDViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::vtkMRMLAbstractThreeDViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::~vtkMRMLAbstractThreeDViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLViewNode * vtkMRMLAbstractThreeDViewDisplayableManager::GetMRMLViewNode()
{
  return vtkMRMLViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
}

