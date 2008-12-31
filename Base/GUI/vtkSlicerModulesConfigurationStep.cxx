#include "vtkSlicerModulesConfigurationStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
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
  this->SetName("Modules Management Wizard");
  this->WizardDialog = NULL;

  this->Header = NULL;
  this->ActionRadioButtonSet = NULL;
  this->CacheDirectoryButton = NULL;
  this->TrashButton = NULL;
  this->SearchLocationBox = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModulesConfigurationStep::~vtkSlicerModulesConfigurationStep()
{
  if (this->Header)
    {
    this->Header->Delete();
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

  if (!this->Header)
    {
    this->Header = vtkKWLabelWithLabel::New();
    }

  if (this->Header->IsCreated())
    {
    this->Header->SetParent( wizard_widget->GetClientArea() );
    this->Header->Create();
    this->Header->SetLabelText("This wizard lets you search for modules to add to 3D Slicer,\ndownload and install them, and uninstall existing modules.");
    this->Header->GetWidget()->SetImageToPredefinedIcon(vtkKWIcon::IconConnection);
    }

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

    this->ActionRadioButtonSet->PackHorizontallyOn();
  }

  if (!this->CacheDirectoryButton)
    {
    this->CacheDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
    }
  if (!this->CacheDirectoryButton->IsCreated())
    {
    this->CacheDirectoryButton->SetParent( wizard_widget->GetClientArea() );
    this->CacheDirectoryButton->Create();
    this->CacheDirectoryButton->SetLabelText("Download (cache) directory:");
    }

  if (!this->TrashButton)
    {
    this->TrashButton = vtkKWCheckButton::New();
    }
  if (!this->TrashButton->IsCreated())
    {
    this->TrashButton->SetParent( wizard_widget->GetClientArea() );
    this->TrashButton->Create();
    this->TrashButton->SetImageToPredefinedIcon(vtkKWIcon::IconTrashcan);
    }

  if (!this->SearchLocationBox)
    {
    this->SearchLocationBox = vtkKWComboBoxWithLabel::New();
    }
  if (!this->SearchLocationBox->IsCreated())
    {
    this->SearchLocationBox->SetParent( wizard_widget->GetClientArea() );
    this->SearchLocationBox->Create();
    this->SearchLocationBox->SetLabelText("Where to search:");
    this->SearchLocationBox->GetWidget()->SetValue("ext.slicer.org/ext");
    }
 
  this->Script("pack %s -side top -expand y -anchor center",
               this->Header->GetWidgetName());

  this->Script("pack %s %s -side top -expand y -anchor center",
               this->CacheDirectoryButton->GetWidgetName(),
               this->TrashButton->GetWidgetName());
  
  this->Script("pack %s -side top -expand y -anchor center", 
               this->ActionRadioButtonSet->GetWidgetName());

  this->Script("pack %s -side top -expand y -anchor center", 
               this->SearchLocationBox->GetWidgetName());
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

