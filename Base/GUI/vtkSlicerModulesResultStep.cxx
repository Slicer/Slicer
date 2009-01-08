#include "vtkSlicerModulesResultStep.h"
#include "vtkSlicerModulesStep.h"

#include "vtkStringArray.h"

#include "vtkObjectFactory.h" 

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"

#include "vtkHTTPHandler.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesWizardDialog.h"


#include <vtksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesResultStep );
vtkCxxRevisionMacro(vtkSlicerModulesResultStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerModulesResultStep::vtkSlicerModulesResultStep()
{
  this->SetName("Extension Managament Wizard");
  this->WizardDialog = NULL;
  this->HeaderText = NULL;
  this->RestartButton = NULL;
  this->LaterButton = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModulesResultStep::~vtkSlicerModulesResultStep()
{
  if (this->HeaderText)
    {
    this->HeaderText->Delete();
    }
  if (this->RestartButton)
    {
    this->RestartButton->Delete();
    }
  if (this->LaterButton)
    {
    this->LaterButton->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkSlicerModulesWizardDialog *wizard_dialog = 
    dynamic_cast<vtkSlicerModulesWizardDialog*> (this->GetWizardDialog());

  vtkKWWizardWidget *wizard_widget = wizard_dialog->GetWizardWidget();

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    } 
  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( wizard_widget->GetClientArea() );
    this->HeaderText->Create();
    this->HeaderText->SetText("Choose to restart 3D Slicer now to incorporate these\nextensions changes immediately. Restarting later will make\nthe changes next time the software starts up.");
    }

  if (!this->RestartButton)
    {
    this->RestartButton = vtkKWPushButton::New();
    }
  if (!this->RestartButton->IsCreated())
    {
    this->RestartButton->SetParent( wizard_widget->GetClientArea() );
    this->RestartButton->Create();
    this->RestartButton->SetText("Restart 3D Slicer now");
    this->RestartButton->SetCommand(this, "RestartButtonCallback");
    }

  if (!this->LaterButton)
    {
    this->LaterButton = vtkKWPushButton::New();
    }
  if (!this->LaterButton->IsCreated())
    {
    this->LaterButton->SetParent( wizard_widget->GetClientArea() );
    this->LaterButton->Create();
    this->LaterButton->SetText("Restart later");
    this->LaterButton->SetCommand(this, "LaterButtonCallback");
    }

  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->HeaderText->GetWidgetName());

  this->Script("pack %s %s -side top -expand y -fill none -anchor center", 
               this->RestartButton->GetWidgetName(),
               this->LaterButton->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::RestartButtonCallback()
{
  this->GetWizardDialog()->GetWidget()->GetOKButton()->Select();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::LaterButtonCallback()
{
  this->GetWizardDialog()->GetWidget()->GetCancelButton()->Select();
}
