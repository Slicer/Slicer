/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAbstractDisplayableManager.h,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

/// Superclass for displayable manager classes.
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 


#ifndef __vtkMRMLAbstractDisplayableManager_h
#define __vtkMRMLAbstractDisplayableManager_h

// VTK includes
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

// MRML includes
#include "vtkMRMLScene.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractDisplayableManager : public vtkMRMLAbstractLogic 
{
public:
  
  //void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkSetObjectMacro(Renderer, vtkRenderer);

  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);
  vtkSetObjectMacro(Interactor, vtkRenderWindowInteractor);

  virtual void Create() = 0;


protected:
  vtkMRMLAbstractDisplayableManager();
  virtual ~vtkMRMLAbstractDisplayableManager();
  vtkMRMLAbstractDisplayableManager(const vtkMRMLAbstractDisplayableManager&);
  void operator=(const vtkMRMLAbstractDisplayableManager&);

  vtkRenderWindowInteractor * Interactor;
  vtkRenderer               * Renderer;
};

#endif

