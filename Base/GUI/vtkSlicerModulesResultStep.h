#ifndef __vtkSlicerModulesResultStep_h
#define __vtkSlicerModulesResultStep_h

#include <string>
#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWFrame;

class vtkSlicerModulesResultStep : public vtkKWWizardStep
{
public:
  static vtkSlicerModulesResultStep* New();
  vtkTypeRevisionMacro(vtkSlicerModulesResultStep,vtkKWWizardStep);

  /// 
  /// Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  /// 
  /// Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

  /// 
  /// callbacks
  virtual void RestartButtonCallback();
  virtual void LaterButtonCallback();

protected:
  vtkSlicerModulesResultStep();
  ~vtkSlicerModulesResultStep();

  vtkKWFrame *Frame1;
  vtkKWFrame *Frame2;
  vtkKWLabel *HeaderText;
  vtkKWPushButton *RestartButton;
  vtkKWPushButton *LaterButton;
  vtkSlicerModulesWizardDialog *WizardDialog;

private:
  vtkSlicerModulesResultStep(const vtkSlicerModulesResultStep&);   /// Not implemented.
  void operator=(const vtkSlicerModulesResultStep&);  /// Not implemented.

};

#endif
