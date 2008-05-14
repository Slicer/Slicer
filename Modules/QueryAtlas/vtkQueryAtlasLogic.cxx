/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkQueryAtlasLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkQueryAtlasLogic.h"


vtkCxxRevisionMacro(vtkQueryAtlasLogic, "$Revision: 1.9.12.1 $");

vtkQueryAtlasLogic* vtkQueryAtlasLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkQueryAtlasLogic");
  if(ret)
    {
      return (vtkQueryAtlasLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkQueryAtlasLogic;
}


//----------------------------------------------------------------------------
vtkQueryAtlasLogic::vtkQueryAtlasLogic()
{
  this->QueryAtlasNode = NULL;
}

//----------------------------------------------------------------------------
vtkQueryAtlasLogic::~vtkQueryAtlasLogic()
{
  this->SetQueryAtlasNode(NULL);
}


//----------------------------------------------------------------------------
void vtkQueryAtlasLogic::PrintSelf(ostream& os, vtkIndent indent)
{

}

//----------------------------------------------------------------------------
void vtkQueryAtlasLogic::ProcessMrmlEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData )
{
  // nothing to do here....
}



//----------------------------------------------------------------------------
vtkIntArray* vtkQueryAtlasLogic::NewObservableEvents()
{
  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue ( vtkMRMLScene::SceneCloseEvent );
  events->InsertNextValue ( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue ( vtkMRMLScene::NodeRemovedEvent );
  return events;
}
