/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: QtApp.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME QtApp - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __QtApp_h
#define __QtApp_h

//BTX

class QtSlicerNodeSelectorWidget;

#include "vtkMRMLScene.h"

class VTK_SLICER_BASE_GUI_QT_EXPORT QtApp
{
public:

  QtApp();
  virtual ~QtApp();
    
  vtkMRMLScene* GetMRMLScene() 
    {
      return this->MRMLScene;
    };
  
  void SetMRMLScene(vtkMRMLScene *scene)
    {
      this->MRMLScene = scene;
    };

  void BuildGUI();
  
protected:

  vtkMRMLScene               *MRMLScene;
  QtSlicerNodeSelectorWidget *qNodeSelector;

  QtApp(const QtApp&); // Not implemented
  void operator=(const QtApp&); // Not Implemented
};
//ETX
#endif

