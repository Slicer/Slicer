/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkEMSegmentParametersSetStep.h,v$
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $

==============================================================auto=*/

#ifndef __vtkEMSegmentParametersSetStep_h
#define __vtkEMSegmentParametersSetStep_h

#include "vtkEMSegmentStep.h"
#include <vector>

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWTopLevel;
//class vtkKWLabel;
class vtkKWPushButton;
class vtkKWEntryWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentParametersSetStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentParametersSetStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentParametersSetStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Update the list of loaded parameter sets.
  virtual void UpdateLoadedParameterSets();

  // Description:
  // Callbacks.
  virtual void SelectedParameterSetChangedCallback(int index) { this->SelectedParameterSetChangedCallback(index,1);}
  virtual void SelectedParameterSetChangedCallback(int index, int flag);
  virtual void SelectedDefaultTaskChangedCallback(int index, bool warningFlag = 1);
  virtual void SelectedPreprocessingChangedCallback(int index, bool warningFlag = 1);

  void RenameApplyCallback(const char* newName);
  void UpdateTasksCallback();
  void HideRenameEntry();

  vtkGetObjectMacro(RenameEntry,vtkKWEntryWithLabel);
  vtkGetObjectMacro(RenameApply, vtkKWPushButton);

  // void Validate() {this->Validate(1);}

protected:
  vtkEMSegmentParametersSetStep();
  ~vtkEMSegmentParametersSetStep();

  vtkKWFrameWithLabel      *ParameterSetFrame;
  vtkKWMenuButtonWithLabel *ParameterSetMenuButton;
  vtkKWMenuButtonWithLabel *PreprocessingMenuButton;

  int RenameIndex;
  vtkKWTopLevel *RenameTopLevel;
  vtkKWEntryWithLabel *RenameEntry;
  vtkKWPushButton *RenameApply;
  vtkKWPushButton *RenameCancel;
  vtkKWPushButton *UpdateTasksButton;


  // Description:
  // Populate the list of loaded parameter sets.
  virtual void PopulateLoadedParameterSets();

private:
  vtkEMSegmentParametersSetStep(const vtkEMSegmentParametersSetStep&);
  void operator=(const vtkEMSegmentParametersSetStep&);

  void PopUpRenameEntry(int index);
  void UpdateTaskListIndex(int index);

  int LoadDefaultData(const char *dataLink, bool warningFlag);
  int LoadDefaultTask(int index, bool warningFlag);

  void DefineDefaultTasksList();
  void AddDefaultTasksToList(const char* FilePath);

  //BTX:
  std::vector<std::string> pssDefaultTasksName;
  std::vector<std::string> pssDefaultTasksFile;
  std::vector<std::string> DefinePreprocessingTasksName;
  std::vector<std::string> DefinePreprocessingTasksFile;
 //ETX:

  int SettingSegmentationMode(int flag);

};

#endif
