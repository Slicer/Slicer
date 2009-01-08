#ifndef __vtkSlicerModulesResultStep_h
#define __vtkSlicerModulesResultStep_h

#include <string>
#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWPushButton;
class vtkKWLabel;

class vtkSlicerModulesResultStep : public vtkKWWizardStep
{
public:
  static vtkSlicerModulesResultStep* New();
  vtkTypeRevisionMacro(vtkSlicerModulesResultStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

  // Description:
  // callbacks
  virtual void RestartButtonCallback();
  virtual void LaterButtonCallback();

protected:
  vtkSlicerModulesResultStep();
  ~vtkSlicerModulesResultStep();

  vtkKWLabel *HeaderText;
  vtkKWPushButton *RestartButton;
  vtkKWPushButton *LaterButton;
  vtkSlicerModulesWizardDialog *WizardDialog;

private:
  vtkSlicerModulesResultStep(const vtkSlicerModulesResultStep&);   // Not implemented.
  void operator=(const vtkSlicerModulesResultStep&);  // Not implemented.

};

#endif
