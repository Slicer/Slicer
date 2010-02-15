#include "vtkSlicerModulesResultStep.h"
#include "vtkSlicerModulesStep.h"

// vtkSlicer includes
#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesWizardDialog.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"

// RemoteIO includes
#include "vtkHTTPHandler.h"

// VTK includes
#include "vtkStringArray.h"
#include "vtkObjectFactory.h" 

#include <vtksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesResultStep );
vtkCxxRevisionMacro(vtkSlicerModulesResultStep, "$Revision$");

//----------------------------------------------------------------------------
vtkSlicerModulesResultStep::vtkSlicerModulesResultStep()
{
  this->SetName("Extension Managament Wizard");
  this->WizardDialog = NULL;
  this->Frame1 = NULL;
  this->Frame2 = NULL;
  this->HeaderText = NULL;
  this->RestartButton = NULL;
  this->LaterButton = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModulesResultStep::~vtkSlicerModulesResultStep()
{
  if (this->Frame1)
    {
    this->Frame1->Delete();
    }
  if (this->Frame2)
    {
    this->Frame2->Delete();
    }
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

  if (!this->Frame1)
    {
    this->Frame1 = vtkKWFrame::New();
    }
  if (!this->Frame1->IsCreated())
    {
    this->Frame1->SetParent( wizard_widget->GetClientArea() );
    this->Frame1->Create();
    }

  if (!this->Frame2)
    {
    this->Frame2 = vtkKWFrame::New();
    }
  if (!this->Frame2->IsCreated())
    {
    this->Frame2->SetParent( wizard_widget->GetClientArea() );
    this->Frame2->Create();
    }

  this->Script("pack %s %s -side top -pady 5",
               this->Frame1->GetWidgetName(),
               this->Frame2->GetWidgetName());

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    } 
  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( this->Frame1 );
    this->HeaderText->Create();
    this->HeaderText->SetText("Choose to restart 3D Slicer now to incorporate these\nextensions changes immediately. Restarting later will make\nthe changes next time the software starts up.");
    }

  if (!this->RestartButton)
    {
    this->RestartButton = vtkKWPushButton::New();
    }
  if (!this->RestartButton->IsCreated())
    {
    this->RestartButton->SetParent( this->Frame2 );
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
    this->LaterButton->SetParent( this->Frame2 );
    this->LaterButton->Create();
    this->LaterButton->SetText("Restart later");
    this->LaterButton->SetCommand(this, "LaterButtonCallback");
    }

  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->HeaderText->GetWidgetName());

  this->Script("pack %s %s -side left -expand y -fill none -padx 10 -anchor center", 
               this->RestartButton->GetWidgetName(),
               this->LaterButton->GetWidgetName());

  this->GetWizardDialog()->GetWizardWidget()->BackButtonVisibilityOff();
  this->GetWizardDialog()->GetWizardWidget()->NextButtonVisibilityOff();
  this->GetWizardDialog()->GetWizardWidget()->OKButtonVisibilityOff();
  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOff();

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
  this->GetWizardDialog()->OK();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::LaterButtonCallback()
{
  this->GetWizardDialog()->Cancel();
}
