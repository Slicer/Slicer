/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterGUI.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkGradientAnisotropicDiffusionFilterGUI_h
#define __vtkGradientAnisotropicDiffusionFilterGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkMRMLScene.h"
#include "vtkGradientAnisotropicDiffusionFilterLogic.h"


class vtkSlicerSliceWidget;
class vtkKWFrame;
class vtkKWScaleWithEntry;
class vtkKWPushButton;
class vtkSlicerNodeSelectorWidget;

class VTK_GRADIENTANISOTROPICDIFFUSIONFILTER_EXPORT vtkGradientAnisotropicDiffusionFilterGUI : public vtkSlicerModuleGUI
{
  public:
  static vtkGradientAnisotropicDiffusionFilterGUI *New();
  vtkTypeMacro(vtkGradientAnisotropicDiffusionFilterGUI,vtkSlicerModuleGUI);
  void PrintSelf(ostream& os, vtkIndent indent);

   // Description: Get/Set MRML node
  vtkGetObjectMacro (Logic, vtkGradientAnisotropicDiffusionFilterLogic);
  vtkSetObjectMacro (Logic, vtkGradientAnisotropicDiffusionFilterLogic);
  
  // Description: Get/Set MRML node
  vtkGetObjectMacro (GradientAnisotropicDiffusionFilterNode, vtkMRMLGradientAnisotropicDiffusionFilterNode);
  vtkSetObjectMacro (GradientAnisotropicDiffusionFilterNode, vtkMRMLGradientAnisotropicDiffusionFilterNode);

  virtual void BuildGUI ( );

  virtual void AddGUIObservers ( );
  
  virtual void RemoveGUIObservers ( );
  
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                  void *callData ){};
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                  void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, 
                                  void *callData);
  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter ( ){};
  virtual void Exit ( ){};

protected:
  vtkGradientAnisotropicDiffusionFilterGUI();
  ~vtkGradientAnisotropicDiffusionFilterGUI();
  vtkGradientAnisotropicDiffusionFilterGUI(const vtkGradientAnisotropicDiffusionFilterGUI&);
  void operator=(const vtkGradientAnisotropicDiffusionFilterGUI&);

  void UpdateGUI();
  void UpdateMRML();
  
  vtkKWScaleWithEntry* ConductanceScale;
  vtkKWScaleWithEntry* TimeStepScale;
  vtkKWScaleWithEntry* NumberOfIterationsScale;
  vtkSlicerNodeSelectorWidget* VolumeSelector;
  vtkSlicerNodeSelectorWidget* OutVolumeSelector;
  vtkSlicerNodeSelectorWidget* GADNodeSelector;
  vtkKWPushButton* ApplyButton;
  
  vtkGradientAnisotropicDiffusionFilterLogic *Logic;
  vtkMRMLGradientAnisotropicDiffusionFilterNode* GradientAnisotropicDiffusionFilterNode;

};

#endif

