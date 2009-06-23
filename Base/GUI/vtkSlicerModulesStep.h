#ifndef __vtkSlicerModulesStep_h
#define __vtkSlicerModulesStep_h

#include "vtkStringArray.h"

#include "vtkKWWizardStep.h"

#include <string>
#include <vector>
#include <map>

class vtkSlicerApplication;
class vtkSlicerModulesWizardDialog;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWFrame;

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
  virtual void Update();

  // Description:
  // Check if a download/install or uninstall has been selected
  //BTX
  enum
  {
    ActionIsDownloadInstall = 0,
    ActionIsUninstall,
    ActionIsEmpty
  };
  //ETX
  virtual int IsActionValid();

  // Description:
  // Set/Get the wizard widget this step should install its UI in.
  vtkGetObjectMacro(WizardDialog, vtkSlicerModulesWizardDialog);
  virtual void SetWizardDialog(vtkSlicerModulesWizardDialog*);

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
  
  // Description:
  // Callback for when the homepage is clicked
  void OpenHomePageInBrowserCallback();

  // Description:
  // Command for the description cell
  void DescriptionCommand(const char *notused,
                       int row_index,
                       int col_index,
                       const char *widget_name);
  
  // Description:
  // Get/Set status code for module
  //BTX
  enum
  {
    StatusSuccess = 0,
    StatusWait,
    StatusCancelled,
    StatusError,
    StatusFoundOnDisk,
    StatusNotFoundOnDisk,
    StatusUnknown
  };
  //ETX
  virtual int GetStatus(int row_index);
  virtual void SetStatus(int row_index, int status);

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

  vtkKWFrame *Frame1;
  vtkKWFrame *Frame2;
  vtkKWFrame *Frame3;
  vtkKWFrame *Frame4;

  vtkKWLabel *HeaderText;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *SelectNoneButton;

  vtkKWMultiColumnListWithScrollbars *ModulesMultiColumnList;
  int HomePageColIndex;

  vtkKWPushButton *DownloadButton;
  vtkKWPushButton *UninstallButton;
  vtkKWPushButton *StopButton;

  vtkSlicerModulesWizardDialog *WizardDialog;

  // Description:
  // Helper method to turn a module manifest into a selectable list.
  //BTX
  std::vector<ManifestEntry*> ParseManifest(const std::string&);
  //ETX

  // Description:
  // Helper method to add greatest revision of named extensions to the vector.
  //BTX
  void AddEntry(std::vector<ManifestEntry*> &entries, ManifestEntry *entry);
  //ETX

  // Description:
  // Helper method to download .s3ext file and parse
  //BTX
  void DownloadParseS3ext(const std::string &s3ext, ManifestEntry *entry);
  //ETX
      
  // Description:
  // Helper method for dowlonad and install
  //BTX
  bool DownloadInstallExtension(const std::string &ExtensionName,
                                const std::string &ExtensionBinaryURL);
  //ETX

  // Description:
  // Helper method for dowlonad and install
  //BTX
  bool UninstallExtension(const std::string &ExtensionName);
  //ETX

  // Description:
  // Helper method for adding extensions to the column list
  //BTX
  void InsertExtension(int Index,
                       ManifestEntry *Entry,
                       const std::string &InstallDir);
  //ETX

  // Description:
  // Helper method to read from repository and parse for modules
  //BTX
  void UpdateModulesFromRepository(vtkSlicerApplication *app);
  //ETX

  // Description:
  // Helper method that asks the conf. step what action should be taken.
  //BTX
  int ActionToBeTaken();
  // ETX

  // Description:
  // Keep track if anything was done or not.  Validation succeeds if
  // at least one action was taken.
  //BTX
  int ActionTaken;
  //ETX
};

#endif
