#ifndef __vtkSlicerRepositoryStep_h
#define __vtkSlicerRepositoryStep_h

#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWRadioButtonSet;
class vtkKWStateMachineInput;

class vtkSlicerRepositoryStep : public vtkKWWizardStep
{
public:
  static vtkSlicerRepositoryStep* New();
  vtkTypeRevisionMacro(vtkSlicerRepositoryStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Get selected repository
  //BTX
  enum 
  {
    RepositoryNITRC = 0,
    RepositoryUnknown
  };
  //ETX
  virtual int GetSelectedRepository();

  // Description:
  // Check if the repository is valid
  //BTX
  enum 
  {
    RepositoryIsValid = 0,
    RepositoryConnectionError,
    RepositoryError
  };
  //ETX
  virtual int IsRepositoryValid();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

  // Description:
  // Get the input to branch when repository has failed or is empty.
  vtkGetObjectMacro(RepositoryValidationFailed, vtkKWStateMachineInput);

protected:
  vtkSlicerRepositoryStep();
  ~vtkSlicerRepositoryStep();

  vtkKWRadioButtonSet *RepositoryRadioButtonSet;
  vtkSlicerModulesWizardDialog *WizardDialog;
  vtkKWStateMachineInput *RepositoryValidationFailed;

private:
  vtkSlicerRepositoryStep(const vtkSlicerRepositoryStep&);   // Not implemented.
  void operator=(const vtkSlicerRepositoryStep&);  // Not implemented.
};

#endif
