/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableManagerFactory.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/
///  vtkMRMLDisplayableManagerFactory - superclass for slicer logic classes
/// 
/// Superclass for all slicer logic classes (application, views, slices).
/// There must be a corresponding vtkSlicerGUI subclass corresponding 
/// to each logic class that handles all GUI interaction (no GUI code
/// goes in the logic class).

#ifndef __vtkMRMLDisplayableManagerFactory_h
#define __vtkMRMLDisplayableManagerFactory_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

#include "qSlicerBaseQTGUIExport.h"

#include "vtkMRMLAbstractDisplayableManager.h"

class Q_SLICER_BASE_QTGUI_EXPORT vtkMRMLDisplayableManagerFactory : public vtkObject 
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

  vtkMRMLScene *MRMLScene;
  vtkRenderer *Renderer;
  vtkRenderWindowInteractor *Interactor;
};

#endif

