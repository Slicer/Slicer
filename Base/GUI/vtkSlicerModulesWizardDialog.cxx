#include "vtkSlicerModulesWizardDialog.h"

#include "vtkObjectFactory.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModulesConfigurationStep.h"
#include "vtkSlicerModulesStep.h"
#include "vtkSlicerModulesResultStep.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWIcon.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesWizardDialog );
vtkCxxRevisionMacro(vtkSlicerModulesWizardDialog, "$Revision$");

//----------------------------------------------------------------------------
vtkSlicerModulesWizardDialog::vtkSlicerModulesWizardDialog()
{
  this->ModulesConfigurationStep    = NULL;
  this->ModulesStep    = NULL;
  this->ModulesResultStep    = NULL;

  this->SelectedRepositoryURL = "NULL";
}

//----------------------------------------------------------------------------
void vtkSlicerModulesWizardDialog::OK()
{
  this->Superclass::OK();

  this->Script("exec $::env(Slicer_HOME)/Slicer3 &; $::slicer3::Application SetPromptBeforeExit 0; exit");
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

  this->SetSize(600, 500);

  vtkKWWizardWorkflow *wizard_workflow = this->GetWizardWorkflow();

  // Add Configuration step

  this->ModulesConfigurationStep = vtkSlicerModulesConfigurationStep::New();
  this->ModulesConfigurationStep->SetWizardDialog(this);
  wizard_workflow->AddStep(this->ModulesConfigurationStep);

  // Add Modules step

  this->ModulesStep = vtkSlicerModulesStep::New();
  this->ModulesStep->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->ModulesStep);

  // Add Result step

  this->ModulesResultStep = vtkSlicerModulesResultStep::New();
  this->ModulesResultStep->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->ModulesResultStep);

  // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->ModulesResultStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->ModulesConfigurationStep);

  // Clean up
  this->ModulesConfigurationStep->Delete();
  this->ModulesStep->Delete();
  this->ModulesResultStep->Delete();
}
