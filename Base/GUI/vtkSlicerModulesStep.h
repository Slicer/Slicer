#ifndef __vtkSlicerModulesStep_h
#define __vtkSlicerModulesStep_h

#include "vtkStringArray.h"

#include "vtkKWWizardStep.h"

#include <string>
#include <vector>
#include <map>

class vtkSlicerModulesWizardDialog;
class vtkKWMultiColumnList;
class vtkKWPushButton;
class vtkKWLabel;
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

  // Description:
  // Callbacks for extensions configuration actions.
  // Description:
  // Select all extensions from the repository.
  void SelectAll();
  // Description:
  // Select no extensions from the repository.
  void SelectNone();
  // Description:
  // Download and install selected extensions.
  void DownloadInstall();
  // Description:
  // Uninstall selected extensions.
  void Uninstall();

protected:
  vtkSlicerModulesStep();
  ~vtkSlicerModulesStep();

  //BTX
  std::vector<ManifestEntry*> Modules;
  //ETX

  //BTX
  std::map<std::string, std::string> Messages;
  //ETX

private:
  vtkSlicerModulesStep(const vtkSlicerModulesStep&); // Not implemented.
  void operator=(const vtkSlicerModulesStep&); // Not implemented.

  // Description:
  // Helper method to turn a module manifest into a selectable list.
  //BTX
  std::vector<ManifestEntry*> ParseManifest(const std::string&);
  //ETX
  
  vtkKWLabel *HeaderText;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *SelectNoneButton;
  vtkKWMultiColumnList *ModulesMultiColumnList;
  vtkKWPushButton *DownloadButton;
  vtkKWPushButton *UninstallButton;

  vtkSlicerModulesWizardDialog *WizardDialog;

  // Description:
  // Helper method for dowlonad and install
  //BTX
  void DownloadInstallExtension(const std::string& ExtensionBinaryURL);
  //ETX

  // Description:
  // Helper method for dowlonad and install
  //BTX
  void UninstallExtension(const std::string& ExtensionName);
  //ETX

};

#endif
