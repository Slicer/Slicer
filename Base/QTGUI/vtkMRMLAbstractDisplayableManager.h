/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractDisplayableManager.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/
///  vtkMRMLAbstractDisplayableManager - superclass for slicer logic classes
/// 
/// Superclass for all slicer logic classes (application, views, slices).
/// There must be a corresponding vtkSlicerGUI subclass corresponding 
/// to each logic class that handles all GUI interaction (no GUI code
/// goes in the logic class).

#ifndef __vtkMRMLAbstractDisplayableManager_h
#define __vtkMRMLAbstractDisplayableManager_h

#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "qSlicerBaseQTGUIExport.h"


#include "vtkMRMLScene.h"
#include "vtkMRMLAbstractLogic.h"

class Q_SLICER_BASE_QTGUI_EXPORT vtkMRMLAbstractDisplayableManager : public vtkMRMLAbstractLogic 
{
  public:
  
  /// The Usual vtk class functions
  static vtkMRMLAbstractDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAbstractDisplayableManager,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkSetObjectMacro(Renderer, vtkRenderer);

  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);
  vtkSetObjectMacro(Interactor, vtkRenderWindowInteractor);

  virtual void Create() {};


protected:
  vtkMRMLAbstractDisplayableManager();
  virtual ~vtkMRMLAbstractDisplayableManager();
  vtkMRMLAbstractDisplayableManager(const vtkMRMLAbstractDisplayableManager&);
  void operator=(const vtkMRMLAbstractDisplayableManager&);

  vtkRenderWindowInteractor *Interactor;
  vtkRenderer               *Renderer;
};

#endif

