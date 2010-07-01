/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractDisplayableManager.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLDisplayableManagerFactory.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
class vtkMRMLAbstractDisplayableManager::vtkInternal
{
public:
  vtkInternal();

  bool                                Initialized;
  bool                                Created;
  int                                 ProcessingMRMLEvent;
  bool                                UpdateFromMRMLRequested;
  vtkRenderer *                       Renderer;
  vtkMRMLViewNode *                   MRMLViewNode;
  vtkMRMLDisplayableManagerFactory *  DisplayableManagerFactory;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkInternal::vtkInternal()
{
  this->Initialized = false;
  this->Created = false;
  this->ProcessingMRMLEvent = 0;
  this->UpdateFromMRMLRequested = false;
  this->Renderer = 0;
  this->MRMLViewNode = 0;
  this->DisplayableManagerFactory = 0;
}

//----------------------------------------------------------------------------
// vtkMRMLAbstractDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkMRMLAbstractDisplayableManager()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::~vtkMRMLAbstractDisplayableManager()
{
  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory * vtkMRMLAbstractDisplayableManager::GetDisplayableManagerFactory()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->DisplayableManagerFactory address "
                << this->Internal->DisplayableManagerFactory );
  return this->Internal->DisplayableManagerFactory;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::CreateIfPossible()
{
  if (!this->IsCreated())
    {
    assert(this->GetMRMLScene());
    assert(this->GetMRMLViewNode());
    this->Create();
    this->ProcessMRMLEvents(this->GetMRMLScene(), vtkMRMLScene::SceneLoadEndEvent, 0);
    this->Internal->Created = true;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::Initialize(vtkMRMLDisplayableManagerFactory * factory,
                                                   vtkRenderer* newRenderer)
{
  // Sanity checks
  if (this->Internal->Initialized)
    {
    return;
    }
  if (!factory)
    {
    return;
    }
  if (!newRenderer)
    {
    return;
    }

  this->Internal->DisplayableManagerFactory = factory;

  this->Internal->Renderer = newRenderer;
  this->Internal->Renderer->Register(this);

  this->Internal->Initialized = true;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "initializing with Factory " << factory << "and Renderer " << newRenderer);

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractDisplayableManager::IsInitialized()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Initialized of "
                << this->Internal->Initialized);
  return this->Internal->Initialized;
}

bool vtkMRMLAbstractDisplayableManager::IsCreated()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Created of "
                << this->Internal->Created);
  return this->Internal->Created;
}

//---------------------------------------------------------------------------
vtkRenderer * vtkMRMLAbstractDisplayableManager::GetRenderer()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer address " << this->Internal->Renderer );
  return this->Internal->Renderer;
}

//---------------------------------------------------------------------------
vtkRenderWindowInteractor * vtkMRMLAbstractDisplayableManager::GetInteractor()
{
  if (!this->Internal->Renderer && !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return 0;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->Renderer->GetRenderWindow()->GetInteractor() address "
                << this->Internal->Renderer->GetRenderWindow()->GetInteractor() );
  return this->Internal->Renderer->GetRenderWindow()->GetInteractor();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode * vtkMRMLAbstractDisplayableManager::GetMRMLViewNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->MRMLViewNode address " << this->Internal->MRMLViewNode );
  return this->Internal->MRMLViewNode;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetAndObserveMRMLViewNode(vtkMRMLViewNode * newMRMLViewNode)
{
  this->SetMRMLScene(newMRMLViewNode ? newMRMLViewNode->GetScene() : 0);
  vtkSetAndObserveMRMLNodeMacro(this->Internal->MRMLViewNode, newMRMLViewNode);
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractDisplayableManager::GetProcessingMRMLEvent()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->ProcessingMRMLEvent of "
                << this->Internal->ProcessingMRMLEvent);
  return this->Internal->ProcessingMRMLEvent;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::SetUpdateFromMRMLRequested(bool requested)
{
  if (this->Internal->UpdateFromMRMLRequested == requested)
    {
    return;
    }

  this->Internal->UpdateFromMRMLRequested = requested;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RequestRender()
{
  // TODO Add a mechanism to check if Rendering is disable

  if (this->Internal->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    }

  this->InvokeEvent(vtkCommand::UpdateEvent);
  this->GetDisplayableManagerFactory()->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::RemoveMRMLObservers()
{
  this->SetAndObserveMRMLViewNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  // Sanity checks
  if (this->Internal->ProcessingMRMLEvent != 0)
    {
    return;
    }

  vtkMRMLScene * scene = vtkMRMLScene::SafeDownCast(caller);
  assert(scene);

  vtkMRMLNode * node = 0;

  this->Internal->ProcessingMRMLEvent = event;

  switch(event)
    {
    case vtkMRMLScene::SceneClosingEvent:
      this->OnMRMLSceneClosingEvent(scene);
      break;
    case vtkMRMLScene::SceneCloseEvent:
      this->OnMRMLSceneCloseEvent(scene);
      break;
    case vtkMRMLScene::SceneLoadStartEvent:
      this->OnMRMLSceneLoadStartEvent(scene);
      break;
    case vtkMRMLScene::SceneLoadEndEvent:
      this->OnMRMLSceneLoadEndEvent(scene);
      break;
    case vtkMRMLScene::SceneRestoredEvent:
      this->OnMRMLSceneRestoredEvent(scene);
      break;
    case vtkMRMLScene::NodeAddedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeAddedEvent(scene, node);
      break;
    case vtkMRMLScene::NodeRemovedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeRemovedEvent(scene, node);
      break;
    }

  this->Internal->ProcessingMRMLEvent = 0;
}

