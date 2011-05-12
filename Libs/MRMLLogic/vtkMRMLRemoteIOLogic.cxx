/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLRemoteIOLogic.cxx,v $
  Date:      $Date: 2011-04-06 17:26:19 -0400 (Wed, 06 Apr 2011) $
  Version:   $Revision: 16232 $

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLRemoteIOLogic.h"

// MRML includes

// VTK sys includes
#include <vtksys/SystemTools.hxx>

// RemoteIO includes
#include "vtkMRMLApplicationLogic.h"
//#include "vtkDataIOManagerLogic.h"
#include "vtkHTTPHandler.h"
#include "vtkSRBHandler.h"
#include "vtkXNATHandler.h"
#include "vtkHIDHandler.h"
#include "vtkXNDHandler.h"
//#include "vtkSlicerXNATPermissionPrompterWidget.h"

#include "vtkMRMLRemoteIOLogic.h"

vtkCxxRevisionMacro(vtkMRMLRemoteIOLogic, "$Revision: 16232 $");
vtkStandardNewMacro(vtkMRMLRemoteIOLogic);

//----------------------------------------------------------------------------
vtkMRMLRemoteIOLogic::vtkMRMLRemoteIOLogic()
{
}

//----------------------------------------------------------------------------
vtkMRMLRemoteIOLogic::~vtkMRMLRemoteIOLogic()
{
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

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("Cannot add DataIOHandlers -- scene not set");
    }

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

  vtkSRBHandler *srbHandler = vtkSRBHandler::New();
  srbHandler->SetPrefix ( "srb://" );
  srbHandler->SetName ( "SRBHandler" );
  this->GetMRMLScene()->AddURIHandler(srbHandler);
  srbHandler->Delete();

  vtkXNATHandler *xnatHandler = vtkXNATHandler::New();
  // TODO: deal with permission prompting at the application level
  //vtkSlicerXNATPermissionPrompterWidget *xnatPermissionPrompter = vtkSlicerXNATPermissionPrompterWidget::New();
#if 0
  if (app)
    {
    //xnatPermissionPrompter->SetApplication ( app );
    }
#endif
  //xnatPermissionPrompter->SetPromptTitle ("Permission Prompt");
  xnatHandler->SetPrefix ( "xnat://" );
  xnatHandler->SetName ( "XNATHandler" );
  xnatHandler->SetRequiresPermission (0);
  //xnatHandler->SetRequiresPermission (1);
  //xnatHandler->SetPermissionPrompter ( xnatPermissionPrompter );
  this->GetMRMLScene()->AddURIHandler(xnatHandler);
  //xnatPermissionPrompter->Delete();
  xnatHandler->Delete();

  vtkHIDHandler *hidHandler = vtkHIDHandler::New();
  hidHandler->SetPrefix ( "hid://" );
  hidHandler->SetName ( "HIDHandler" );
  this->GetMRMLScene()->AddURIHandler( hidHandler);
  hidHandler->Delete();

  vtkXNDHandler *xndHandler = vtkXNDHandler::New();
  xndHandler->SetPrefix ( "xnd://" );
  xndHandler->SetName ( "XNDHandler" );
  this->GetMRMLScene()->AddURIHandler( xndHandler);
  xndHandler->Delete();

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
  this->GetMRMLScene()->SetURIHandlerCollection(NULL);
  this->GetMRMLScene()->SetUserTagTable( NULL );
}

