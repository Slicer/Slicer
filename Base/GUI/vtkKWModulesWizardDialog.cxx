#include "vtkKWModulesWizardDialog.h"

#include "vtkObjectFactory.h"

#include "vtkKWRepositoryStep.h"
#include "vtkKWModulesStep.h"
#include "vtkKWProgressStep.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWIcon.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWModulesWizardDialog );
vtkCxxRevisionMacro(vtkKWModulesWizardDialog, "$Revision$");

//----------------------------------------------------------------------------
vtkKWModulesWizardDialog::vtkKWModulesWizardDialog()
{
  this->RepositoryStep    = NULL;
  this->ModulesStep    = NULL;
  this->ProgressStep    = NULL;
}

//----------------------------------------------------------------------------
void vtkKWModulesWizardDialog::CreateWidget()
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

  // Add Operator step

  this->OperatorStep = vtkKWMyOperatorStep::New();
  this->OperatorStep->SetWizardDialog(this);
  wizard_workflow->AddStep(this->OperatorStep);
  this->OperatorStep->Delete();

  // Add Operand 1 step (addition, division)

  this->Operand1Step = vtkKWMyOperand1Step::New();
  this->Operand1Step->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->Operand1Step);
  this->Operand1Step->Delete();

  // Add Operand 2 step (addition, division)

  this->Operand2Step = vtkKWMyOperand2Step::New();
  this->Operand2Step->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->Operand2Step);
  this->Operand2Step->Delete();

  // Add Result step (addition, division, square root) (aka Finish step)

  this->ResultStep = vtkKWMyResultStep::New();
  this->ResultStep->SetWizardDialog(this);
  wizard_workflow->AddNextStep(this->ResultStep);
  this->ResultStep->Delete();

  // ...manually connect the operand 1 step to result if only one operand
  // is needed

  wizard_workflow->AddInput(
    this->Operand1Step->GetOperand1ValidationSucceededForOneOperandInput());
  wizard_workflow->CreateNextTransition(
    this->Operand1Step,
    this->Operand1Step->GetOperand1ValidationSucceededForOneOperandInput(),
    this->ResultStep);
  wizard_workflow->CreateBackTransition(
    this->Operand1Step, this->ResultStep);

  // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->ResultStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->OperatorStep);
}
