/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeSelectGUI.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumeSelectGUI - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerVolumeSelectGUI_h
#define __vtkSlicerVolumeSelectGUI_h

#include "vtkCallbackCommand.h"

#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumeSelectGUI : public vtkKWMenuButtonWithSpinButtonsWithLabel
{
public:
  static vtkSlicerVolumeSelectGUI* New();
  vtkTypeRevisionMacro(vtkSlicerVolumeSelectGUI,vtkKWMenuButtonWithSpinButtonsWithLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // this is the scene to observe
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  void SetMRMLScene(vtkMRMLScene *MRMLScene);

  void SelectVolume();
  vtkMRMLVolumeNode *GetSelected();

  // Description:
  // reflect the state of the mrml scene in the menu
  void UpdateMenu();

protected:
  vtkSlicerVolumeSelectGUI();
  ~vtkSlicerVolumeSelectGUI();

  vtkMRMLScene       *MRMLScene;

private:

  vtkCallbackCommand *MRMLCallbackCommand;

  vtkSlicerVolumeSelectGUI(const vtkSlicerVolumeSelectGUI&); // Not implemented
  void operator=(const vtkSlicerVolumeSelectGUI&); // Not Implemented
};

#endif

