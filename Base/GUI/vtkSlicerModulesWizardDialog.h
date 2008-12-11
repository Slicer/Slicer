#ifndef __vtkSlicerModulesWizardDialog_h
#define __vtkSlicerModulesWizardDialog_h

#include "vtkKWWizardDialog.h"

#include <string>

class vtkSlicerModulesConfigurationStep;
class vtkSlicerModulesStep;
class vtkSlicerModulesResultStep;

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

class vtkSlicerModulesWizardDialog : public vtkKWWizardDialog
{
public:
  static vtkSlicerModulesWizardDialog* New();
  vtkTypeRevisionMacro(vtkSlicerModulesWizardDialog,vtkKWWizardDialog);
  
  // Description:
  // Loadable Modules specific OK will restart Slicer.
  virtual void OK();

  // Description:
  // Access to the steps.
  vtkGetObjectMacro(ModulesConfigurationStep, vtkSlicerModulesConfigurationStep);
  vtkGetObjectMacro(ModulesStep, vtkSlicerModulesStep);
  vtkGetObjectMacro(ModulesResultStep, vtkSlicerModulesResultStep);

protected:
  vtkSlicerModulesWizardDialog();
  ~vtkSlicerModulesWizardDialog() {};

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Steps
  vtkSlicerModulesConfigurationStep *ModulesConfigurationStep;
  vtkSlicerModulesStep *ModulesStep;
  vtkSlicerModulesResultStep *ModulesResultStep;

private:
  vtkSlicerModulesWizardDialog(const vtkSlicerModulesWizardDialog&); // Not implemented.
  void operator=(const vtkSlicerModulesWizardDialog&); // Not implemented.
};

#endif
