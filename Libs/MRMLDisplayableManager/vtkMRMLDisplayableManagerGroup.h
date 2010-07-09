/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerGroup.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// DisplayableManagerGroup is a collection of DisplayableManager
/// 
/// It also provides method allowing to either call RenderRequest
/// or SetAndObserveMRMLViewNode on all member of the group.
/// 

#ifndef __vtkMRMLDisplayableManagerGroup_h
#define __vtkMRMLDisplayableManagerGroup_h

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLDisplayableManagerFactory;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLViewNode;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerGroup : public vtkObject 
{
public:

  static vtkMRMLDisplayableManagerGroup *New();
  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerGroup,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Set and observe DisplayableManager factory
  void SetAndObserveDisplayableManagerFactory(vtkMRMLDisplayableManagerFactory * factory);

  ///
  /// Add a DisplayableManager and initialize it if required
  void AddAndInitialize(vtkMRMLAbstractDisplayableManager * displayableManager);

  ///
  /// Return the number of DisplayableManager already added to the group
  int GetDisplayableManagerCount();

  ///
  /// Return a DisplayableManager given its class name
  vtkMRMLAbstractDisplayableManager* GetDisplayableManagerByClassName(const char* className);

  /// Set Renderer and Interactor
  /// No-op if already initialized.
  /// \sa IsInitialized
  void Initialize(vtkRenderer* newRenderer);

  ///
  /// Return True if Group has already been initialized
  bool IsInitialized();

  ///
  /// Convenient method to get the WindowInteractor associated with the Renderer
  vtkRenderWindowInteractor* GetInteractor();

  /// Invoke vtkCommand::UpdateEvent
  /// An observer can then listen for that event and "compress" the different Render requests
  /// to efficiently call RenderWindow->Render()
  /// \sa vtkMRMLAbstractDisplayableManager::RequestRender()
  void RequestRender();

  ///
  /// Get Renderer
  vtkRenderer* GetRenderer();

  ///
  /// Set / Get MRML ViewNode
  vtkMRMLViewNode* GetMRMLViewNode();
  void SetMRMLViewNode(vtkMRMLViewNode* newMRMLViewNode);

protected:

  vtkMRMLDisplayableManagerGroup();
  virtual ~vtkMRMLDisplayableManagerGroup();

  //BTX
  typedef vtkMRMLDisplayableManagerGroup Self;
  static void DoCallback(vtkObject* vtk_obj, unsigned long event,
                         void* client_data, void* call_data);
  /// Trigger upon a DisplayableManager is either registered or unregisterer from
  /// the associated factory
  void onDisplayableManagerFactoryRegisteredEvent(const char* displayableManagerName);
  void onDisplayableManagerFactoryUnRegisteredEvent(const char* displayableManagerName);
  //ETX

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

private:

  vtkMRMLDisplayableManagerGroup(const vtkMRMLDisplayableManagerGroup&);
  void operator=(const vtkMRMLDisplayableManagerGroup&);

};

#endif

