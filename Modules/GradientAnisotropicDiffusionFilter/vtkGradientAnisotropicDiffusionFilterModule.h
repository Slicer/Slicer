/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterModule.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkGradientAnisotropicDiffusionFilterModule_h
#define __vtkGradientAnisotropicDiffusionFilterModule_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceWidgetCollection.h"
#include "vtkMRMLScene.h"

#include "vtkGradientAnisotropicDiffusionFilter.h"
//#include "vtkITKGradientAnisotropicDiffusionImageFilter.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"

class vtkSlicerSliceWidget;
class vtkKWFrame;
class vtkKWPushButton;
class vtkSlicerVolumeSelectGUI;

class VTK_GRADIENTANISOTROPICDIFFUSIONFILTER_EXPORT vtkGradientAnisotropicDiffusionFilterModule : public vtkSlicerComponentGUI
{
  public:
  static vtkGradientAnisotropicDiffusionFilterModule *New();
  vtkTypeMacro(vtkGradientAnisotropicDiffusionFilterModule,vtkSlicerComponentGUI);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void BuildGUI ( );
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( ){};
  virtual void AddLogicObservers ( ){};
  virtual void RemoveLogicObservers ( ){};
  virtual void AddMrmlObservers ( );
  virtual void RemoveMrmlObservers ( ){};
  
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                  void *callData ){};
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                  void *callData );
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData );

  vtkGetObjectMacro (GradientAnisotropicDiffusionFilterNode, vtkMRMLGradientAnisotropicDiffusionFilterNode);
  vtkSetObjectMacro (GradientAnisotropicDiffusionFilterNode, vtkMRMLGradientAnisotropicDiffusionFilterNode);

protected:
  vtkGradientAnisotropicDiffusionFilterModule();
  ~vtkGradientAnisotropicDiffusionFilterModule();
  vtkGradientAnisotropicDiffusionFilterModule(const vtkGradientAnisotropicDiffusionFilterModule&);
  void operator=(const vtkGradientAnisotropicDiffusionFilterModule&);

  vtkMRMLGradientAnisotropicDiffusionFilterNode* GradientAnisotropicDiffusionFilterNode;
  vtkKWScaleWithEntry* ConductanceScale;
  vtkKWScaleWithEntry* TimeStepScale;
  vtkKWScaleWithEntry* NumberOfIterationsScale;
  vtkSlicerVolumeSelectGUI* VolumeSelector;
  vtkKWPushButton* ApplyButton;

};

#endif

