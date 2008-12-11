#include "vtkSlicerModulesConfigurationStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModulesWizardDialog.h"

#include "vtkHTTPHandler.h"

#include <itksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesConfigurationStep );
vtkCxxRevisionMacro(vtkSlicerModulesConfigurationStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkSlicerModulesConfigurationStep::vtkSlicerModulesConfigurationStep()
{
  this->SetName("Modules Configuration Wizard");
  this->WizardDialog = NULL;

  this->Icon = NULL;
  this->Label = NULL;
  this->ActionRadioButtonSet = NULL;
  this->CacheDirectoryButton = NULL;
  this->TrashButton = NULL;
  this->SearchLocationBox = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModulesConfigurationStep::~vtkSlicerModulesConfigurationStep()
{
  if (this->Icon)
    {
    this->Icon->Delete();
    }
  if (this->Label)
    {
    this->Label->Delete();
    }
  if (this->ActionRadioButtonSet)
    {
    this->ActionRadioButtonSet->Delete();
    }
  if (this->CacheDirectoryButton)
    {
    this->CacheDirectoryButton->Delete();
    }
  if (this->TrashButton)
    {
    this->TrashButton->Delete();
    }
  if (this->SearchLocationBox)
    {
    this->SearchLocationBox->Delete();
    }

  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  if (this->WizardDialog)
    {
    this->WizardDialog->Delete();
    }
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  if (!this->Icon)
    {
    this->Icon = vtkKWLabel::New();
    }

  if (this->Icon->IsCreated())
    {
      this->Icon->SetImageToPredefinedIcon(vtkKWIcon::IconConnection);
    }
 
  if (!this->Label)
    {
    this->Label = vtkKWLabel::New();
    }
  if (!this->Label->IsCreated())
    {
    this->Label->SetText("This wizard lets you search for modules to add to 3D Slicer, download and install them, and uninstall existing modules.");
    }

  this->Script("pack %s %s -side left -anchor n -padbx 4 -pady 4",
               this->Icon->GetWidgetName(),
               this->Label->GetWidgetName());

  if (!this->ActionRadioButtonSet)
    {
    this->ActionRadioButtonSet = vtkKWRadioButtonSet::New();
    }
  if (!this->ActionRadioButtonSet->IsCreated())
    {
    this->ActionRadioButtonSet->SetParent(wizard_widget->GetClientArea());
    this->ActionRadioButtonSet->Create();

    vtkKWRadioButton *radiob;

    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionInstall);
    radiob->SetText("Install");
    radiob->SetCommand(wizard_widget, "Update");

    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionUninstall);
    radiob->SetText("Uninstall");
    radiob->SetCommand(wizard_widget, "Update");
 
    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionEither);
    radiob->SetText("Either");
    radiob->SetCommand(wizard_widget, "Update");

    this->ActionRadioButtonSet->GetWidget(
      vtkSlicerModulesConfigurationStep::ActionInstall)->Select();
    }

  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->ActionRadioButtonSet->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetWizardDialog()->GetWizardWidget()->GetWizardWorkflow();

  // This step always validates

  wizard_workflow->PushInput(
    vtkKWWizardStep::GetValidationSucceededInput());

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkSlicerModulesConfigurationStep::GetSelectedAction()
{
  if (this->ActionRadioButtonSet)
    {
    if (this->ActionRadioButtonSet->GetWidget(
          vtkSlicerModulesConfigurationStep::ActionInstall)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionInstall;
      }
    if (this->ActionRadioButtonSet->GetWidget(
          vtkSlicerModulesConfigurationStep::ActionUninstall)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionUninstall;
      }
    if (this->ActionRadioButtonSet->GetWidget(
          vtkSlicerModulesConfigurationStep::ActionEither)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionEither;
      }
    }

  return vtkSlicerModulesConfigurationStep::ActionUnknown;
}

