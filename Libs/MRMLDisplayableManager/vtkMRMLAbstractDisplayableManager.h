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
  
  static vtkMRMLAbstractDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractDisplayableManager, vtkMRMLAbstractLogic);

  vtkGetObjectMacro(Renderer, vtkRenderer);
  vtkSetObjectMacro(Renderer, vtkRenderer);

  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);
  vtkSetObjectMacro(Interactor, vtkRenderWindowInteractor);

  /// Method create should be re-implemented
  virtual void Create(){};


protected:
  vtkMRMLAbstractDisplayableManager();
  virtual ~vtkMRMLAbstractDisplayableManager();
  
  vtkRenderWindowInteractor * Interactor;
  vtkRenderer               * Renderer;
  
private:
  vtkMRMLAbstractDisplayableManager(const vtkMRMLAbstractDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractDisplayableManager&);                    // Not implemented
};

#endif

