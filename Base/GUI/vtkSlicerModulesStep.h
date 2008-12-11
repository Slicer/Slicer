#ifndef __vtkSlicerModulesStep_h
#define __vtkSlicerModulesStep_h

#include "vtkStringArray.h"

#include "vtkKWWizardStep.h"

#include <string>
#include <vector>

class vtkSlicerModulesWizardDialog;
class vtkKWMultiColumnList;
class vtkKWCheckButton;
//BTX
class ManifestEntry;
//ETX

class vtkSlicerModulesStep : public vtkKWWizardStep
{
public:
  static vtkSlicerModulesStep* New();
  vtkTypeRevisionMacro(vtkSlicerModulesStep,vtkKWWizardStep);

  // Description:
  // Show/hide the UI, validate the step.
  virtual void ShowUserInterface();
  virtual void HideUserInterface();
  virtual void Validate();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

  /// Get a list of modules to install
  //BTX
  std::vector<ManifestEntry*> GetSelectedModules();
  //ETX

protected:
  vtkSlicerModulesStep();
  ~vtkSlicerModulesStep();

  vtkKWMultiColumnList *ModulesMultiColumnList;
  vtkSlicerModulesWizardDialog *WizardDialog;
  //BTX
  std::vector<ManifestEntry*> Modules;
  //ETX

private:
  vtkSlicerModulesStep(const vtkSlicerModulesStep&); // Not implemented.
  void operator=(const vtkSlicerModulesStep&); // Not implemented.

  // Description:
  // Helper method to turn a module manifest into a selectable list.
  //BTX
  std::vector<ManifestEntry*> ParseManifest(const std::string&);
  //ETX
  
  vtkKWCheckButton *DownloadButton;
  vtkKWCheckButton *UninstallButton;

};

#endif
