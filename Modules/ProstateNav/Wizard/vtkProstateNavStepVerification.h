/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavStepVerification_h
#define __vtkProstateNavStepVerification_h

#include "vtkProstateNavStep.h"

class vtkKWEntry;
class vtkKWEntrySet;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWMatrixWidgetWithLabel;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWMultiColumnList;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkKWText;
class vtkImageData;
class vtkMRMLScalarVolumeNode;
class vtkMRMLSelectionNode;
class vtkMRMLFiducialListNode;
class vtkKWCheckButton;

class VTK_PROSTATENAV_EXPORT vtkProstateNavStepVerification : public vtkProstateNavStep
{
public:
  static vtkProstateNavStepVerification *New();
  vtkTypeRevisionMacro(vtkProstateNavStepVerification,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void TearDownGUI();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void UpdateGUI();

  void OnMultiColumnListSelectionChanged();
  void UpdateTargetListGUI();

protected:
  vtkProstateNavStepVerification();
  ~vtkProstateNavStepVerification();

  void ShowVolumeSelectionFrame();
  void ShowTargetListFrame();
  void ShowVerificationControlFrame();

  unsigned int PopulateListWithTargetDetails(unsigned int targetDescIndex);
  void UpdateVerificationResultsForCurrentTarget();
  void DisplayVerificationResultsForCurrentTarget();

  void AddGUIObservers();
  void RemoveGUIObservers();  

  void AddMRMLObservers();
  void RemoveMRMLObservers();

  void SetVerificationPointListNode(vtkMRMLFiducialListNode *node);

  void StartVerification();
  void StopVerification();  

  bool ProcessingCallback;

  vtkKWFrame *MainFrame;
  
  // TargetPlanning
  vtkKWFrame *VolumeSelectionFrame;
  vtkKWPushButton *LoadVerificationVolumeButton;
  vtkSlicerNodeSelectorWidget* VolumeSelectorWidget;
  vtkKWFrame *LoadVolumeDialogFrame;

  // TargetList frame
  vtkKWFrame *TargetListFrame;
  vtkKWMultiColumnListWithScrollbars* TargetList;

  // TargetControl frame
  vtkKWFrame *VerificationControlFrame;
  vtkKWPushButton *VerifyButton;
  vtkKWPushButton *ClearButton;

  vtkKWText *Message;

  // Description:
  // VerificationPointListNode is used for displaying two fiducial points that defines a needle trajectory
  vtkMRMLFiducialListNode* VerificationPointListNode;

  // If TargetIndexUnderVerification<0 it means that there no target is under verification.
  int TargetIndexUnderVerification;
  
  // If MonitorFiducialNodes is true, then after changing/adding fiducials the verification results are update.
  // It is normally true, except when we want to reload old verification results.
  bool MonitorFiducialNodes; 

private:
  vtkProstateNavStepVerification(const vtkProstateNavStepVerification&);
  void operator=(const vtkProstateNavStepVerification&);
};

#endif
