#ifndef __vtkSlicerModulesStep_h
#define __vtkSlicerModulesStep_h

#include "vtkStringArray.h"

#include "vtkKWWizardStep.h"

#include <string>
#include <vector>

class vtkSlicerModulesWizardDialog;
class vtkKWMultiColumnList;

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
  vtkStringArray* GetSelectedModules();

protected:
  vtkSlicerModulesStep();
  ~vtkSlicerModulesStep();

  vtkKWMultiColumnList *ModulesMultiColumnList;
  vtkSlicerModulesWizardDialog *WizardDialog;
  vtkStringArray* SelectedModules;

private:
  vtkSlicerModulesStep(const vtkSlicerModulesStep&);   // Not implemented.
  void operator=(const vtkSlicerModulesStep&);  // Not implemented.

  // Description:
  // An object to hold entry information.
  //BTX
  class ManifestEntry
  {
  public:
    std::string Name;
    std::string Version;
    std::string URL;
  };
  //ETX

  // Description:
  // Helper method to turn a module manifest into a selectable list.
  //BTX
  std::vector<ManifestEntry> ParseManifest(const std::string&);
  //ETX

};

#endif
