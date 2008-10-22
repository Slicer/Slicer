#include "vtkSlicerModulesWizardDialog.h"

#include "vtkObjectFactory.h"

#include "vtkSlicerRepositoryStep.h"
#include "vtkSlicerModulesStep.h"
#include "vtkSlicerProgressStep.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWIcon.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesWizardDialog );
vtkCxxRevisionMacro(vtkSlicerModulesWizardDialog, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkSlicerModulesWizardDialog::vtkSlicerModulesWizardDialog()
{
  this->RepositoryStep    = NULL;
  this->ModulesStep    = NULL;
  this->ProgressStep    = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesWizardDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  vtkKWWizardWorkflow *wizard_workflow = this->GetWizardWorkflow();
  vtkKWWizardWidget *wizard_widget = this->GetWizardWidget();

  wizard_widget->GetTitleIconLabel()->SetImageToPredefinedIcon(
    vtkKWIcon::IconCalculator);

  // Add Repository step

  this->RepositoryStep = vtkSlicerRepositoryStep::New();
  this->RepositoryStep->SetWizardDialog(this);
  wizard_workflow->AddStep(this->RepositoryStep);
  this->RepositoryStep->Delete();

  // Add Modules step

  this->ModulesStep = vtkSlicerModulesStep::New();
  this->ModulesStep->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->ModulesStep);
  this->ModulesStep->Delete();

  // Add Progress step

  this->ProgressStep = vtkSlicerProgressStep::New();
  this->ProgressStep->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->ProgressStep);
  this->ProgressStep->Delete();

  // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->ProgressStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->RepositoryStep);
}
