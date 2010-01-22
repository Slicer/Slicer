#ifndef __vtkKWModulesWizardDialog_h
#define __vtkKWModulesWizardDialog_h

#include "vtkKWWizardDialog.h"

class vtkKWRepositoryStep;
class vtkKWModulesStep;
class vtkKWProgresstep;

class vtkKWModulesWizardDialog : public vtkKWWizardDialog
{
public:
  static vtkKWModulesWizardDialog* New();
  vtkTypeRevisionMacro(vtkKWModulesWizardDialog,vtkKWWizardDialog);

  /// 
  /// Access to the steps.
  vtkGetObjectMacro(RepositoryStep, vtkKWMyRepositoryStep);
  vtkGetObjectMacro(ModulesStep, vtkKWMyModulesStep);
  vtkGetObjectMacro(ProgressStep, vtkKWMyProgressStep);

protected:
  vtkKWModulesWizardDialog();
  ~vtkKWModulesWizardDialog() {};

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  /// 
  /// Steps
  vtkKWMyRepositoryStep *RepositoryStep;
  vtkKWMyModulesStep *ModulesStep;
  vtkKWMyProgressStep *ProgressStep;

private:
  vtkKWModulesWizardDialog(const vtkKWModulesWizardDialog&);   /// Not implemented.
  void operator=(const vtkKWModulesWizardDialog&);  /// Not implemented.
};

#endif
