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

// VTK includes
#include "vtkObject.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerFactory : public vtkObject 
{
public:
  /// The Usual vtk class functions
  static vtkMRMLDisplayableManagerFactory *New();
  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerFactory,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Register Displayable Manager
  void RegisterDisplayableManager ( vtkMRMLAbstractDisplayableManager *displayableManager );

  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkSetObjectMacro(Renderer, vtkRenderer);

  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  vtkSetObjectMacro(MRMLScene, vtkMRMLScene);

  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);
  vtkSetObjectMacro(Interactor, vtkRenderWindowInteractor);


protected:
  vtkMRMLDisplayableManagerFactory();
  virtual ~vtkMRMLDisplayableManagerFactory();
  vtkMRMLDisplayableManagerFactory(const vtkMRMLDisplayableManagerFactory&);
  void operator=(const vtkMRMLDisplayableManagerFactory&);

  //BTX
  std::vector<vtkMRMLAbstractDisplayableManager *> DisplayableManagers;
  //ETX

  vtkMRMLScene *               MRMLScene;
  vtkRenderer *                Renderer;
  vtkRenderWindowInteractor *  Interactor;
};

#endif

