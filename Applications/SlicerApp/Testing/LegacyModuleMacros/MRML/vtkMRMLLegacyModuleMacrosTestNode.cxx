/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#include "vtkMRMLLegacyModuleMacrosTestNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLegacyModuleMacrosTestNode);

//----------------------------------------------------------------------------
void vtkMRMLLegacyModuleMacrosTestNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
