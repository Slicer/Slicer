/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
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

class VTK_PROSTATENAV_EXPORT vtkProstateNavCalibrationStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavCalibrationStep *New();
  vtkTypeRevisionMacro(vtkProstateNavCalibrationStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

  void ShowZFrameModel();
  void HideZFrameModel();

  const char* AddZFrameModel(const char* nodeName); // returns Node ID
  const char* AddZFrameTransform(const char* nodeName);

  void PerformZFrameCalibration(const char* filename);

  
protected:
  vtkProstateNavCalibrationStep();
  ~vtkProstateNavCalibrationStep();

  vtkKWFrame       *SelectImageFrame;
  vtkKWLoadSaveButtonWithLabel *SelectImageButton;
  vtkKWPushButton  *CalibrateButton;
  vtkKWCheckButton *ShowZFrameCheckButton;
  vtkKWFrame       *ZFrameSettingFrame;

  //vtkMRMLModelNode* ZFrameModelNode;
  //BTX
  std::string ZFrameModelNodeID;
  std::string ZFrameTransformNodeID;
  //ETX
  

private:
  vtkProstateNavCalibrationStep(const vtkProstateNavCalibrationStep&);
  void operator=(const vtkProstateNavCalibrationStep&);
};

#endif
