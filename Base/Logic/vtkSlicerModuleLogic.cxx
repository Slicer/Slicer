/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModuleLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerModuleLogic.h"

vtkCxxRevisionMacro(vtkSlicerModuleLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::vtkSlicerModuleLogic()
{
  this->ApplicationLogic = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::~vtkSlicerModuleLogic()
{
  this->SetApplicationLogic(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerLogic:             " << this->GetClassName() << "\n";
}

