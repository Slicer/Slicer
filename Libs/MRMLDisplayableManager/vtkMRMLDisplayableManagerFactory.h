/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerFactory.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Factory where displayable manager classe should be registered with
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 

#ifndef __vtkMRMLDisplayableManagerFactory_h
#define __vtkMRMLDisplayableManagerFactory_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderWindowInteractor;
class vtkRenderer;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerFactory : public vtkObject 
{
public:

  static vtkMRMLDisplayableManagerFactory *New();
  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerFactory,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Set Renderer and Interactor
  /// No-op if already initialized. 
  /// \sa IsInitialized
  void Initialize(vtkRenderer* newRenderer);
  
  ///
  /// Return True if Factory has already been initialized
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
  /// Register Displayable Manager
  void RegisterDisplayableManager(vtkMRMLAbstractDisplayableManager *displayableManager);

  ///
  /// UnRegister Displayable Manager
  void UnRegisterDisplayableManager(vtkMRMLAbstractDisplayableManager *displayableManager);

  ///
  /// Return a DisplayManager given its class name
  vtkMRMLAbstractDisplayableManager* GetDisplayableManagerByClassName(const char* className);

  ///
  /// Set / Get MRML ViewNode
  vtkMRMLViewNode* GetMRMLViewNode();
  void SetMRMLViewNode(vtkMRMLViewNode* newMRMLViewNode);

protected:

  vtkMRMLDisplayableManagerFactory();
  virtual ~vtkMRMLDisplayableManagerFactory();

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

private:

  vtkMRMLDisplayableManagerFactory(const vtkMRMLDisplayableManagerFactory&);
  void operator=(const vtkMRMLDisplayableManagerFactory&);

};

#endif

