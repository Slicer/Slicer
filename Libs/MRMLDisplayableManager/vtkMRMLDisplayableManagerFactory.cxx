/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerFactory.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"

vtkCxxRevisionMacro(vtkMRMLDisplayableManagerFactory, "$Revision: 13859 $");
vtkStandardNewMacro(vtkMRMLDisplayableManagerFactory);

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkMRMLDisplayableManagerFactory()
{
  this->MRMLScene = NULL;
  this->Renderer = NULL;
  this->Interactor = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::~vtkMRMLDisplayableManagerFactory()
{
  for(unsigned int i=0; i<this->DisplayableManagers.size(); i++)
    {
    this->DisplayableManagers[i]->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory:: RegisterDisplayableManager ( vtkMRMLAbstractDisplayableManager *displayableManager )
{
  displayableManager->Register(this);
  displayableManager->SetRenderer(this->GetRenderer());
  displayableManager->SetInteractor(this->GetInteractor());
  displayableManager->SetMRMLScene(this->GetMRMLScene());
  displayableManager->Create();

  this->DisplayableManagers.push_back(displayableManager);
  
}
