/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractSliceViewDisplayableManager.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

// MRML includes
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractSliceViewDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractSliceViewDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
// vtkMRMLAbstractSliceViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::vtkMRMLAbstractSliceViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::~vtkMRMLAbstractSliceViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(
    vtkObject* caller)
{
  assert(vtkMRMLSliceNode::SafeDownCast(caller));
  this->OnMRMLSliceNodeModifiedEvent();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode * vtkMRMLAbstractSliceViewDisplayableManager::GetMRMLSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->GetMRMLDisplayableNode());
}

