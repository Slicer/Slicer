/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavCalibrationStep_h
#define __vtkProstateNavCalibrationStep_h

#include "vtkProstateNavStep.h"

#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkKWMatrixWidgetWithLabel.h"

#include "vtkSlicerNodeSelectorWidget.h"

class VTK_PROSTATENAV_EXPORT vtkProstateNavCalibrationStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavCalibrationStep *New();
  vtkTypeRevisionMacro(vtkProstateNavCalibrationStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers();

  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

  void ShowZFrameModel(bool show);
  void ShowWorkspaceModel(bool show);

  // Description:
  // If a file name is specified, the function will import an image from the file
  // to the MRML scene and call Z-Frame calibration code.
  void PerformZFrameCalibration(const char* filename);
  void PerformZFrameCalibration(vtkMRMLScalarVolumeNode* node, int s_index, int e_index);
  
protected:
  vtkProstateNavCalibrationStep();
  ~vtkProstateNavCalibrationStep();

  vtkKWFrame       *SelectImageFrame;
  //vtkKWLoadSaveButtonWithLabel *SelectImageButton;
  vtkSlicerNodeSelectorWidget *ZFrameImageSelectorWidget;
  vtkKWMatrixWidgetWithLabel* SliceRangeMatrix;
  vtkKWPushButton  *CalibrateButton;
  vtkKWCheckButton *ShowZFrameCheckButton;
  vtkKWCheckButton *ShowWorkspaceCheckButton;

  vtkKWFrame       *ZFrameSettingFrame;

  //vtkMRMLModelNode* ZFrameModelNode;
  std::string ZFrameModelNodeID;
  std::string ZFrameTransformNodeID;
  

private:
  vtkProstateNavCalibrationStep(const vtkProstateNavCalibrationStep&);
  void operator=(const vtkProstateNavCalibrationStep&);
};

#endif
