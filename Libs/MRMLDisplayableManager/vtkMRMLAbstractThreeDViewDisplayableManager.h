/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractThreeDViewDisplayableManager.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Superclass for displayable manager classes.
/// 
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
/// 


#ifndef __vtkMRMLAbstractThreeDViewDisplayableManager_h
#define __vtkMRMLAbstractThreeDViewDisplayableManager_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLThreeDViewDisplayableManagerGroup;
class vtkMRMLViewNode;
class vtkMRMLScene;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractThreeDViewDisplayableManager : public vtkMRMLAbstractLogic 
{
public:
  
  static vtkMRMLAbstractThreeDViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager, vtkMRMLAbstractLogic);

  ///
  /// Return True if the DisplayableManager has already been initialized
  bool IsInitialized();

  ///
  /// Return True if Create() method has been invoked
  /// \sa CreateIfPossible() Create()
  bool IsCreated();

  ///
  /// Get Renderer
  vtkRenderer* GetRenderer();

  ///
  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  ///
  /// Get MRML ViewNode
  vtkMRMLViewNode * GetMRMLViewNode();

  ///
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

protected:

  vtkMRMLAbstractThreeDViewDisplayableManager();
  virtual ~vtkMRMLAbstractThreeDViewDisplayableManager();

  //BTX
  /// Access to Initialize, SetMRMLViewNode and CreateIfPossible methods
  friend class vtkMRMLThreeDViewDisplayableManagerGroup;
  //ETX

  /// Set Renderer and Interactor
  /// No-op if already initialized.
  /// Called by vtkMRMLThreeDViewDisplayableManagerGroup
  /// \sa IsInitialized
  void Initialize(vtkMRMLThreeDViewDisplayableManagerGroup * group, vtkRenderer* newRenderer);

  /// Called by Initialize();
  /// Sub-class could overload that function and perform additional initialization steps
  /// \note Initialization occurs before the MRMLViewNode is set and observed
  /// \warning That function should only be used directly !
  /// \sa Initialize
  virtual void AdditionnalInitializeStep(){}
  
  /// Called by SetMRMLScene - Used to initialize the Scene
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Set MRML ViewNode
  /// Called by vtkMRMLThreeDViewDisplayableManagerFactory
  void SetAndObserveMRMLViewNode(vtkMRMLViewNode * newMRMLViewNode);

  /// Get associated DisplayableManager group
  vtkMRMLThreeDViewDisplayableManagerGroup * GetDisplayableManagerGroup();

  /// Invoke Create() and set Created flag to True
  /// A no-op if IsCreated() return True
  void CreateIfPossible();

  /// Called after a valid MRML ViewNode is set
  /// \note GetRenderer() and GetMRMLViewNode() will return valid object
  virtual void Create(){}

  /// Remove MRML observers
  virtual void RemoveMRMLObservers();

  /// Specify if UodateFromMRML() should be called
  /// \sa UpdateFromMRML()
  void SetUpdateFromMRMLRequested(bool requested);

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  virtual void UpdateFromMRML(){}

  /// Invoke vtkCommand::UpdateEvent and then call vtkMRMLThreeDViewDisplayableManagerFactory::RequestRender()
  /// which will also invoke vtkCommand::UpdateEvent
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  void RequestRender();

  /// Called after the corresponding MRML event is triggered.
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneAboutToBeClosedEvent(){}
  virtual void OnMRMLSceneClosedEvent(){}
  virtual void OnMRMLSceneAboutToBeImportedEvent(){}
  virtual void OnMRMLSceneImportedEvent(){}
  virtual void OnMRMLSceneRestoredEvent(){}
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* /*node*/){}
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/){}
  
private:

  vtkMRMLAbstractThreeDViewDisplayableManager(const vtkMRMLAbstractThreeDViewDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractThreeDViewDisplayableManager&);                    // Not implemented

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX
};

#endif

