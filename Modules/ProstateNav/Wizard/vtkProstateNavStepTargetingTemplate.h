/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavStepTargetingTemplate_h
#define __vtkProstateNavStepTargetingTemplate_h

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

class VTK_PROSTATENAV_EXPORT vtkProstateNavStepTargetingTemplate : public vtkProstateNavStep
{
public:
  static vtkProstateNavStepTargetingTemplate *New();
  vtkTypeRevisionMacro(vtkProstateNavStepTargetingTemplate,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void UpdateGUI();

  void AddMRMLObservers();
  void RemoveMRMLObservers();

  void OnMultiColumnListUpdate(int row, int col, char * str);
  void OnMultiColumnListSelectionChanged();
  void UpdateTargetListGUI();

protected:
  vtkProstateNavStepTargetingTemplate();
  ~vtkProstateNavStepTargetingTemplate();


  void ShowTargetPlanningFrame();
  void ShowTargetListFrame();
  void ShowTargetControlFrame();

  void ShowCoverage(bool show);
  void ShowNeedle(bool show);
  void ShowTemplate(bool show);

  void EnableAddTargetsOnClickButton(bool enable);

  unsigned int PopulateListWithTargetDetails(unsigned int targetDescIndex);

  void AddGUIObservers();
  void RemoveGUIObservers();  
  
  // Description:
  // The column orders in the list box
  enum
    {
    TargetNumberColumn = 0,
    NeedleTypeColumn = 1,
    RASLocationColumn = 2,
    ReachableColumn = 3,
    RotationColumn = 4,
    NeedleAngleColumn = 5,    
    DepthColumn = 6,
    NumberOfColumns = 7,
    };
    

  bool ProcessingCallback;

  vtkKWFrame *MainFrame;
  
  // TargetPlanning
  vtkKWFrame *TargetPlanningFrame;
  //vtkKWPushButton* LoadTargetingVolumeButton;
  vtkSlicerNodeSelectorWidget* VolumeSelectorWidget;
  vtkSlicerNodeSelectorWidget* TargetListSelectorWidget;
  vtkKWFrame *LoadVolumeDialogFrame;
  vtkKWCheckButton *ShowCoverageButton;
  vtkKWCheckButton *AddTargetsOnClickButton;
  vtkKWCheckButton *ShowNeedleButton;
  vtkKWCheckButton *ShowTemplateButton;
  vtkKWMenuButtonWithLabel *NeedleTypeMenuList;
  vtkMRMLFiducialListNode *TargetPlanListNode;
  vtkKWFrame *OptionFrame;

  // TargetList frame
  vtkKWFrame *TargetListFrame;
  vtkKWMultiColumnListWithScrollbars* TargetList;
  vtkKWPushButton *DeleteButton;

  // TargetControl frame
  vtkKWFrame *TargetControlFrame;
  vtkKWMatrixWidgetWithLabel* NeedlePositionMatrix;
  vtkKWMatrixWidgetWithLabel* NeedleOrientationMatrix;
  vtkKWPushButton *MoveButton;
  vtkKWPushButton *StopButton;

  vtkKWText *Message;

private:
  vtkProstateNavStepTargetingTemplate(const vtkProstateNavStepTargetingTemplate&);
  void operator=(const vtkProstateNavStepTargetingTemplate&);
};

#endif
