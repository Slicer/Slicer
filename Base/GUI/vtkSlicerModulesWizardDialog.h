#ifndef __vtkSlicerModulesWizardDialog_h
#define __vtkSlicerModulesWizardDialog_h

#include "vtkKWWizardDialog.h"

#include <string>

class vtkSlicerRepositoryStep;
class vtkSlicerModulesStep;
class vtkSlicerProgressStep;

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
  vtkGetObjectMacro(RepositoryStep, vtkSlicerRepositoryStep);
  vtkGetObjectMacro(ModulesStep, vtkSlicerModulesStep);
  vtkGetObjectMacro(ProgressStep, vtkSlicerProgressStep);

protected:
  vtkSlicerModulesWizardDialog();
  ~vtkSlicerModulesWizardDialog() {};

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Steps
  vtkSlicerRepositoryStep *RepositoryStep;
  vtkSlicerModulesStep *ModulesStep;
  vtkSlicerProgressStep *ProgressStep;

private:
  vtkSlicerModulesWizardDialog(const vtkSlicerModulesWizardDialog&); // Not implemented.
  void operator=(const vtkSlicerModulesWizardDialog&); // Not implemented.
};

#endif
