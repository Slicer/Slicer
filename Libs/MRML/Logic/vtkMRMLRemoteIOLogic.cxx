/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLRemoteIOLogic.cxx,v $
  Date:      $Date: 2011-04-06 17:26:19 -0400 (Wed, 06 Apr 2011) $
  Version:   $Revision: 16232 $

=========================================================================auto=*/

// MRML includes
#include <vtkCacheManager.h>
#include <vtkDataIOManager.h>
#include <vtkMRMLScene.h>
#include <vtkTagTable.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

// RemoteIO includes
#include <vtkHTTPHandler.h>

#include "vtkMRMLRemoteIOLogic.h"

vtkStandardNewMacro(vtkMRMLRemoteIOLogic);

vtkCxxSetObjectMacro(vtkMRMLRemoteIOLogic, CacheManager, vtkCacheManager);
vtkCxxSetObjectMacro(vtkMRMLRemoteIOLogic, DataIOManager, vtkDataIOManager);

//----------------------------------------------------------------------------
vtkMRMLRemoteIOLogic::vtkMRMLRemoteIOLogic()
{
  this->CacheManager = vtkCacheManager::New();
  this->DataIOManager = vtkDataIOManager::New();
  this->DataIOManager->SetCacheManager(this->CacheManager);
}

//----------------------------------------------------------------------------
vtkMRMLRemoteIOLogic::~vtkMRMLRemoteIOLogic()
{
  if (this->DataIOManager)
    {
    this->DataIOManager->SetCacheManager(nullptr);
    this->SetDataIOManager(nullptr);
    }
  if (this->CacheManager)
    {
    this->SetCacheManager(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLRemoteIOLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

}

//-----------------------------------------------------------------------------
void vtkMRMLRemoteIOLogic::AddDataIOToScene()
{
  // TODO more of the cache and DataIOManager code
  // from qSlicerCoreApplication::setMRMLScene(vtkMRMLScene* newMRMLScene)
  // should be moved to here so they can be used outside of the
  // context of a qSlicer based application
  // Update 2015/03/20: split qSlicerCoreApplication::setMRMLScene so that
  // a user can call vtkSlicerApplicationLogic::SetMRMLSceneDataIO to trigger
  /// this method on a independent scene with separate remote io logic and data
  /// io manager logic
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("Cannot add DataIOHandlers -- scene not set");
    return;
    }

  // hook our cache and dataIO managers into the MRML scene
  this->CacheManager->SetMRMLScene(this->GetMRMLScene());
  this->GetMRMLScene()->SetCacheManager(this->CacheManager);
  this->GetMRMLScene()->SetDataIOManager(this->DataIOManager);

  vtkCollection *uriHandlerCollection = vtkCollection::New();
  // add some new handlers
  this->GetMRMLScene()->SetURIHandlerCollection( uriHandlerCollection );
  uriHandlerCollection->Delete();

#if !defined(REMOTEIO_DEBUG)
  // register all existing uri handlers (add to collection)
  vtkHTTPHandler *httpHandler = vtkHTTPHandler::New();
  httpHandler->SetPrefix ( "http://" );
  httpHandler->SetName ( "HTTPHandler");
  this->GetMRMLScene()->AddURIHandler(httpHandler);
  httpHandler->Delete();

  //add something to hold user tags
  vtkTagTable *userTagTable = vtkTagTable::New();
  this->GetMRMLScene()->SetUserTagTable( userTagTable );
  userTagTable->Delete();
#endif
}

void vtkMRMLRemoteIOLogic::RemoveDataIOFromScene()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("Cannot remove DataIOHandlers -- scene not set");
    }
  this->GetMRMLScene()->SetURIHandlerCollection(nullptr);
  this->GetMRMLScene()->SetUserTagTable( nullptr );
}

