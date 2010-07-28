/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractThreeDViewDisplayableManager.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLThreeDViewDisplayableManagerGroup.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// STD includes
#include <cassert>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractThreeDViewDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
class vtkMRMLAbstractThreeDViewDisplayableManager::vtkInternal
{
public:
  vtkInternal();

  static void DoDeleteCallback(vtkObject* vtk_obj, unsigned long event,
                               void* client_data, void* call_data);

  bool                                Initialized;
  bool                                Created;
  bool                                UpdateFromMRMLRequested;
  vtkRenderer *                       Renderer;
  vtkMRMLViewNode *                   MRMLViewNode;
  vtkMRMLThreeDViewDisplayableManagerGroup *    DisplayableManagerGroup;
  vtkSmartPointer<vtkCallbackCommand> DeleteCallBackCommand;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::vtkInternal::vtkInternal()
{
  this->Initialized = false;
  this->Created = false;
  this->UpdateFromMRMLRequested = false;
  this->Renderer = 0;
  this->MRMLViewNode = 0;
  this->DisplayableManagerGroup = 0;
  this->DeleteCallBackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->DeleteCallBackCommand->SetCallback(
      vtkMRMLAbstractThreeDViewDisplayableManager::vtkInternal::DoDeleteCallback);
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::vtkInternal::DoDeleteCallback(vtkObject* vtk_obj,
                                                                      unsigned long event,
                                                                      void* vtkNotUsed(client_data),
                                                                      void* vtkNotUsed(call_data))
{
  vtkMRMLAbstractThreeDViewDisplayableManager* self =
      vtkMRMLAbstractThreeDViewDisplayableManager::SafeDownCast(vtk_obj);
  assert(self);
  assert(event == vtkCommand::DeleteEvent);

  self->RemoveMRMLObservers();
}

//----------------------------------------------------------------------------
// vtkMRMLAbstractThreeDViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::vtkMRMLAbstractThreeDViewDisplayableManager()
{
  this->Internal = new vtkInternal;
  this->AddObserver(vtkCommand::DeleteEvent, this->Internal->DeleteCallBackCommand);
}

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::~vtkMRMLAbstractThreeDViewDisplayableManager()
{
  if (this->Internal->Renderer)
    {
    this->Internal->Renderer->UnRegister(this);
    }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerGroup * vtkMRMLAbstractThreeDViewDisplayableManager::GetDisplayableManagerGroup()
{
  vtkDebugMacro("returning Internal->DisplayableManagerGroup address "
                << this->Internal->DisplayableManagerGroup );
  return this->Internal->DisplayableManagerGroup;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::CreateIfPossible()
{
  if (!this->GetMRMLViewNode())
    {
    return;
    }
  if (!this->IsCreated())
    {
    assert(this->GetMRMLScene());
    assert(this->GetMRMLViewNode());
    this->Create();
    this->Internal->Created = true;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::Initialize(vtkMRMLThreeDViewDisplayableManagerGroup * group,
                                                   vtkRenderer* newRenderer)
{
  // Sanity checks
  if (this->Internal->Initialized)
    {
    return;
    }
  if (!group)
    {
    return;
    }
  if (!newRenderer)
    {
    return;
    }

  this->Internal->DisplayableManagerGroup = group;

  this->Internal->Renderer = newRenderer;
  this->Internal->Renderer->Register(this);

  this->AdditionnalInitializeStep();

  vtkDebugMacro("initializing with Group " << group << " and Renderer " << newRenderer);

  this->Internal->Initialized = true;

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractThreeDViewDisplayableManager::IsInitialized()
{
  vtkDebugMacro("returning Internal->Initialized of " << this->Internal->Initialized);
  return this->Internal->Initialized;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractThreeDViewDisplayableManager::IsCreated()
{
  vtkDebugMacro("returning Internal->Created of "<< this->Internal->Created);
  return this->Internal->Created;
}

//---------------------------------------------------------------------------
vtkRenderer * vtkMRMLAbstractThreeDViewDisplayableManager::GetRenderer()
{
  vtkDebugMacro("returning Internal->Renderer address " << this->Internal->Renderer );
  return this->Internal->Renderer;
}

//---------------------------------------------------------------------------
vtkRenderWindowInteractor * vtkMRMLAbstractThreeDViewDisplayableManager::GetInteractor()
{
  if (!this->Internal->Renderer || !this->Internal->Renderer->GetRenderWindow())
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Interactor address 0");
    return 0;
    }
  vtkDebugMacro("returning Internal->Renderer->GetRenderWindow()->GetInteractor() address "
                << this->Internal->Renderer->GetRenderWindow()->GetInteractor() );
  return this->Internal->Renderer->GetRenderWindow()->GetInteractor();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode * vtkMRMLAbstractThreeDViewDisplayableManager::GetMRMLViewNode()
{
  vtkDebugMacro("returning Internal->MRMLViewNode address " << this->Internal->MRMLViewNode );
  return this->Internal->MRMLViewNode;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  VTK_CREATE(vtkIntArray, sceneEvents);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneAboutToBeImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::SetAndObserveMRMLViewNode(vtkMRMLViewNode * newMRMLViewNode)
{
  // Observe scene associated with the MRML ViewNode
  vtkMRMLScene * sceneToObserve = 0;
  if (newMRMLViewNode)
    {
    sceneToObserve = newMRMLViewNode->GetScene();
    }
  this->SetMRMLScene(sceneToObserve);

  vtkSetAndObserveMRMLNodeMacro(this->Internal->MRMLViewNode, newMRMLViewNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::SetUpdateFromMRMLRequested(bool requested)
{
  if (this->Internal->UpdateFromMRMLRequested == requested)
    {
    return;
    }

  this->Internal->UpdateFromMRMLRequested = requested;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::RequestRender()
{
  // TODO Add a mechanism to check if Rendering is disable

  if (this->Internal->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    }

  this->InvokeEvent(vtkCommand::UpdateEvent);
  this->GetDisplayableManagerGroup()->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::RemoveMRMLObservers()
{
  this->SetAndObserveMRMLViewNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                          unsigned long event,
                                                          void *callData)
{
  assert(vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene());

  vtkMRMLNode * node = 0;

  switch(event)
    {
    case vtkMRMLScene::SceneAboutToBeClosedEvent:
      this->OnMRMLSceneAboutToBeClosedEvent();
      break;
    case vtkMRMLScene::SceneClosedEvent:
      this->OnMRMLSceneClosedEvent();
      break;
    case vtkMRMLScene::SceneAboutToBeImportedEvent:
      this->OnMRMLSceneAboutToBeImportedEvent();
      break;
    case vtkMRMLScene::SceneImportedEvent:
      this->OnMRMLSceneImportedEvent();
      break;
    case vtkMRMLScene::SceneRestoredEvent:
      this->OnMRMLSceneRestoredEvent();
      break;
    case vtkMRMLScene::NodeAddedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeAddedEvent(node);
      break;
    case vtkMRMLScene::NodeRemovedEvent:
      node = reinterpret_cast<vtkMRMLNode*>(callData);
      assert(node);
      this->OnMRMLSceneNodeRemovedEvent(node);
      break;
    }
}

