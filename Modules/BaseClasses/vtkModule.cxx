/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkModule.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.11 $
=========================================================================auto=*/

#include "vtkModule.h"

//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml scene back into the logic layer for further processing
// - this can also end up calling observers of the logic (i.e. in the GUI)
//
static void MRMLCallback(vtkObject *__mrmlscene, unsigned long eid, void *__clientData, void *callData)
{
  static int inMRMLCallback = 0;

  if (inMRMLCallback)
    {
    cout << "*********MRMLCallback called recursively?" << endl;
    return;
    }
  inMRMLCallback = 1;

  vtkMRMLScene *mrmlscene = static_cast<vtkMRMLScene *>(__mrmlscene); // Not used, since it is ivar

  vtkModule *self = reinterpret_cast<vtkModule *>(__clientData);

  self->ProcessMRMLSceneModified(mrmlscene);

  inMRMLCallback = 0;
}

//------------------------------------------------------------------------------
vtkModule* vtkModule::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModule");
  if(ret)
    {
      return (vtkModule*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return NULL;
}

//------------------------------------------------------------------------------
vtkModule::vtkModule()
{
  this->Description = NULL;
  this->Name = NULL;
  this->MRMLSceneRootDir = NULL;
  this->MRMLScene = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
}

//----------------------------------------------------------------------------
vtkModule::~vtkModule()
{
  this->SetDescription(NULL);
  this->SetName(NULL);
  if (this->MRMLScene) {
    this->MRMLScene->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkModule::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent);
  os << indent << "Name: " <<
    (this->Name ? this->Name : "(none)") << "\n";

  os << indent << "Description: " <<
    (this->Description ? this->Description : "(none)") << "\n";

}

//----------------------------------------------------------------------------
void vtkModule::SetMRMLScene(vtkMRMLScene* MRMLScene)
{
  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete();
    }
  
  this->MRMLScene = MRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}
