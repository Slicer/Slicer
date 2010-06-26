/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractDisplayableManager.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractDisplayableManager, "$Revision: 13525 $");
                       
//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkMRMLAbstractDisplayableManager()
{
  this->Renderer = 0;
  this->Interactor = 0;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::~vtkMRMLAbstractDisplayableManager()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

