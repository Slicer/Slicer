#ifndef __vtkSlicerModulesResultStep_h
#define __vtkSlicerModulesResultStep_h

#include <string>
#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWProgressGauge;
class vtkKWText;

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

protected:
  vtkSlicerModulesResultStep();
  ~vtkSlicerModulesResultStep();

  vtkKWProgressGauge *ProgressGauge;
  vtkKWText *Text;
  vtkSlicerModulesWizardDialog *WizardDialog;

private:
  vtkSlicerModulesResultStep(const vtkSlicerModulesResultStep&);   // Not implemented.
  void operator=(const vtkSlicerModulesResultStep&);  // Not implemented.

  //BTX
  void Install(const std::string& url);
  void InstallSource(const std::string& url);
  //ETX
};

#endif
