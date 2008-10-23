#ifndef __vtkSlicerProgressStep_h
#define __vtkSlicerProgressStep_h

#include <string>
#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWProgressGauge;

class vtkSlicerProgressStep : public vtkKWWizardStep
{
public:
  static vtkSlicerProgressStep* New();
  vtkTypeRevisionMacro(vtkSlicerProgressStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

protected:
  vtkSlicerProgressStep();
  ~vtkSlicerProgressStep();

  vtkKWProgressGauge *ProgressGauge;
  vtkSlicerModulesWizardDialog *WizardDialog;

private:
  vtkSlicerProgressStep(const vtkSlicerProgressStep&);   // Not implemented.
  void operator=(const vtkSlicerProgressStep&);  // Not implemented.

  //BTX
  void Install(const std::string& url);
  //ETX
};

#endif
