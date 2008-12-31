#ifndef __vtkSlicerModulesConfigurationStep_h
#define __vtkSlicerModulesConfigurationStep_h

#include "vtkKWWizardStep.h"

class vtkSlicerModulesWizardDialog;
class vtkKWRadioButtonSet;
class vtkKWLabelWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWCheckButton;
class vtkKWComboBoxWithLabel;

class vtkSlicerModulesConfigurationStep : public vtkKWWizardStep
{
public:
  static vtkSlicerModulesConfigurationStep* New();
  vtkTypeRevisionMacro(vtkSlicerModulesConfigurationStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

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

protected:
  vtkSlicerModulesConfigurationStep();
  ~vtkSlicerModulesConfigurationStep();

  vtkSlicerModulesWizardDialog *WizardDialog;
  vtkKWLabelWithLabel *Header;
  vtkKWRadioButtonSet *ActionRadioButtonSet;
  vtkKWLoadSaveButtonWithLabel *CacheDirectoryButton;
  vtkKWCheckButton *TrashButton;
  vtkKWComboBoxWithLabel *SearchLocationBox;

private:
  vtkSlicerModulesConfigurationStep(const vtkSlicerModulesConfigurationStep&); // Not implemented.
  void operator=(const vtkSlicerModulesConfigurationStep&); // Not implemented.

};

#endif
