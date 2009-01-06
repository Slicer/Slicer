#ifndef __vtkSlicerModulesConfigurationStep_h
#define __vtkSlicerModulesConfigurationStep_h

#include "vtkKWWizardStep.h"

#include <string>

class vtkKWComboBoxWithLabel;
class vtkKWLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWPushButton;
class vtkKWRadioButtonSet;
class vtkKWStateMachineInput;
class vtkSlicerModulesWizardDialog;

class vtkSlicerModulesConfigurationStep : public vtkKWWizardStep
{
public:
  static vtkSlicerModulesConfigurationStep* New();
  vtkTypeRevisionMacro(vtkSlicerModulesConfigurationStep,vtkKWWizardStep);

  // Description:
  // Show/hide/update the UI, and validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();
  virtual void Update();

  // Description:
  // Connect to selected repository 0 == success
  virtual int IsRepositoryValid();

  // Description:
  // Get selected action
  //BTX
  enum 
  {
    ActionInstall = 0,
    ActionUninstall,
    ActionEither,
    ActionUnknown
  };
  //ETX
  virtual int GetSelectedAction();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

  // Description:
  // Get the input to branch when repository has failed or is empty.
  vtkGetObjectMacro(RepositoryValidationFailed, vtkKWStateMachineInput);

  // Description:
  // Callbacks
  virtual int ActionRadioButtonSetChangedCallback();

  // Description:
  // Get the repository used to query for extensions.
  //BTX
  std::string GetSelectedRepositoryURL() { return this->SelectedRepositoryURL; };
  //ETX

protected:
  vtkSlicerModulesConfigurationStep();
  ~vtkSlicerModulesConfigurationStep();

  vtkSlicerModulesWizardDialog *WizardDialog;
  vtkKWLabel *HeaderIcon;
  vtkKWLabel *HeaderText;
  vtkKWRadioButtonSet *ActionRadioButtonSet;
  vtkKWLoadSaveButtonWithLabel *CacheDirectoryButton;
  vtkKWPushButton *TrashButton;
  vtkKWComboBoxWithLabel *SearchLocationBox;
  vtkKWStateMachineInput *RepositoryValidationFailed;

  //BTX
  std::string SelectedRepositoryURL;
  //ETX

private:
  vtkSlicerModulesConfigurationStep(const vtkSlicerModulesConfigurationStep&); // Not implemented.
  void operator=(const vtkSlicerModulesConfigurationStep&); // Not implemented.

};

#endif
