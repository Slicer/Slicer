#include "vtkSlicerModulesConfigurationStep.h"

#include "vtkSlicerConfigure.h"

// vtkSlicer includes
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModulesWizardDialog.h"
#include "vtkSlicerFoundationIcons.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWComboBox.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWPushButton.h"
#include "vtkKWStateMachineInput.h"
#include "vtkKWFrame.h"

// RemoteIO includes
#include "vtkHTTPHandler.h"

// VTK includes
#include "vtkObjectFactory.h"

#include <itksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesConfigurationStep );
vtkCxxRevisionMacro(vtkSlicerModulesConfigurationStep, "$Revision$");

//----------------------------------------------------------------------------
vtkSlicerModulesConfigurationStep::vtkSlicerModulesConfigurationStep()
{
  this->SetName("Extensions Management Wizard");
  this->WizardDialog = NULL;

  this->Frame1 = NULL;
  this->Frame2 = NULL;
  this->Frame3 = NULL;
  this->Frame4 = NULL;

  this->HeaderIcon = NULL;
  this->HeaderText = NULL;
  this->ActionRadioButtonSet = NULL;
  this->CacheDirectoryButton = NULL;
  this->TrashButton = NULL;
  this->SearchLocationLabel = NULL;
  this->SearchLocationBox = NULL;

  this->RepositoryValidationFailed = vtkKWStateMachineInput::New();
  this->RepositoryValidationFailed->SetName("failed");
}

//----------------------------------------------------------------------------
vtkSlicerModulesConfigurationStep::~vtkSlicerModulesConfigurationStep()
{
  if (this->Frame1)
    {
    this->Frame1->Delete();
    }
  if (this->Frame2)
    {
    this->Frame2->Delete();
    }
  if (this->Frame3)
    {
    this->Frame3->Delete();
    }
  if (this->Frame4)
    {
    this->Frame4->Delete();
    }
  if (this->HeaderIcon)
    {
    this->HeaderIcon->Delete();
    }
  if (this->HeaderText)
    {
    this->HeaderText->Delete();
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
  if (this->SearchLocationLabel)
    {
    this->SearchLocationLabel->Delete();
    }
  if (this->SearchLocationBox)
    {
    this->SearchLocationBox->Delete();
    }

  this->SetWizardDialog(NULL);

  this->RepositoryValidationFailed->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());

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
  if (!this->Frame3)
    {
    this->Frame3 = vtkKWFrame::New();
    }
  if (!this->Frame3->IsCreated())
    {
    this->Frame3->SetParent( wizard_widget->GetClientArea() );
    this->Frame3->Create();
    }

  if (!this->Frame4)
    {
    this->Frame4 = vtkKWFrame::New();
    }
  if (!this->Frame4->IsCreated())
    {
    this->Frame4->SetParent( wizard_widget->GetClientArea() );
    this->Frame4->Create();
    }

  this->Script("pack %s -side top -fill x -anchor w -padx 5 -pady 5",
               this->Frame1->GetWidgetName() );
  this->Script("pack %s -side top -anchor w -padx 30 -pady 5",
               this->Frame2->GetWidgetName() );
  this->Script("pack %s -side top -anchor w -padx 30 -pady 5",
               this->Frame3->GetWidgetName() );
  this->Script("pack %s -side top -fill x -anchor w -expand y -padx 5 -pady 5",
               this->Frame4->GetWidgetName());

  if (!this->HeaderIcon)
    {
    this->HeaderIcon = vtkKWLabel::New();
    }

  if (!this->HeaderIcon->IsCreated())
    {
    this->HeaderIcon->SetParent( this->Frame1 );
    this->HeaderIcon->Create();
    this->HeaderIcon->SetImageToPredefinedIcon(vtkKWIcon::IconConnection);
    }

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    }

  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( this->Frame1 );
    this->HeaderText->Create();
    this->HeaderText->SetText("This wizard lets you search for extensions to add to 3D Slicer,\ndownload and install them, and uninstall existing extensions.\nYou will need a network connection to access remote extension\nrepositories.");
    }

  if (!this->ActionRadioButtonSet)
    {
    this->ActionRadioButtonSet = vtkKWRadioButtonSet::New();
    }
  if (!this->ActionRadioButtonSet->IsCreated())
    {
    this->ActionRadioButtonSet->SetParent( this->Frame2 );
    this->ActionRadioButtonSet->Create();

    vtkKWRadioButton *radiob;

    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionInstall);
    radiob->SetText("Find & Install");
    radiob->SetCommand(this, "ActionRadioButtonSetChangedCallback");

    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionUninstall);
    radiob->SetText("Uninstall");
    radiob->SetCommand(this, "ActionRadioButtonSetChangedCallback");
 
    radiob = this->ActionRadioButtonSet->AddWidget(
      vtkSlicerModulesConfigurationStep::ActionEither);
    radiob->SetText("Either");
    radiob->SetCommand(this, "ActionRadioButtonSetChangedCallback");

    this->ActionRadioButtonSet->PackHorizontallyOn();
  }

  if (!this->CacheDirectoryButton)
    {
    this->CacheDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
    }
  if (!this->CacheDirectoryButton->IsCreated())
    {
    this->CacheDirectoryButton->SetParent( this->Frame3 );
    this->CacheDirectoryButton->Create();
    this->CacheDirectoryButton->SetLabelText("Change extensions install path:");
    this->CacheDirectoryButton->SetLabelWidth(34);
    this->CacheDirectoryButton->GetLabel()->SetAnchorToEast();
    this->CacheDirectoryButton->GetWidget()->TrimPathFromFileNameOff();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->ChooseDirectoryOn();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->SaveDialogOff();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->SetTitle("Select a directory");
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->SetBalloonHelpString(
      "Select a directory to be used as an install directory for Extensions.");

    this->CacheDirectoryButton->GetWidget()->SetCommand(this, "CacheDirectoryCallback");
    }

  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( this->Frame3 );
  l->Create();
  l->SetText ( "Delete zip files from temp. dir. (optional):" );
  l->SetWidth ( 34 );
  l->SetAnchorToEast();

  if (!this->TrashButton)
    {
    this->TrashButton = vtkKWPushButton::New();
    }
  if (!this->TrashButton->IsCreated())
    {
    this->TrashButton->SetParent( this->Frame3 );
    this->TrashButton->Create();
    this->TrashButton->SetCommand(this, "EmptyCacheDirectoryCommand");

    if (app)
      {
      this->TrashButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDeleteIcon());        
      }
    else
      {
      this->TrashButton->SetImageToPredefinedIcon(vtkKWIcon::IconTrashcan);
      }

    this->TrashButton->SetBorderWidth(0);
    this->TrashButton->SetReliefToFlat();
    }

  if (!this->SearchLocationLabel)
    {
      this->SearchLocationLabel = vtkKWLabel::New();
    }
  if (!this->SearchLocationLabel->IsCreated())
    {
    this->SearchLocationLabel->SetParent( this->Frame4 );
    this->SearchLocationLabel->Create();
    this->SearchLocationLabel->SetText("Where to search:");
    this->SearchLocationLabel->SetWidth(17);
    this->SearchLocationLabel->SetAnchorToEast();
    }

  if (!this->SearchLocationBox)
    {
    this->SearchLocationBox = vtkKWComboBox::New();
    }
  if (!this->SearchLocationBox->IsCreated())
    {
    this->SearchLocationBox->SetParent( this->Frame4 );
    this->SearchLocationBox->Create();
    this->SearchLocationBox->SetCommand(this, "SearchLocationCallback");
    }
 
  this->Script("pack %s %s -side left -anchor w -padx 5", 
               this->HeaderIcon->GetWidgetName(),
               this->HeaderText->GetWidgetName());

  this->Script("pack %s -side top -padx 2 -anchor w", 
               this->ActionRadioButtonSet->GetWidgetName());

  this->Script("pack %s -side top -padx 2 -pady 25 -anchor w",
               this->CacheDirectoryButton->GetWidgetName());

  this->Script("pack %s %s -side left -padx 2 -pady 2 -anchor w",
               l->GetWidgetName(),
               this->TrashButton->GetWidgetName());

  this->Script("pack %s %s -side left -anchor w -padx 5 -pady 25 -fill x -expand y",
               this->SearchLocationLabel->GetWidgetName(),
               this->SearchLocationBox->GetWidgetName());

  l->Delete();
  this->Update();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::Update()
{
  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  if (app)
    {
      if (this->CacheDirectoryButton)
      {
      this->CacheDirectoryButton->GetWidget()->TrimPathFromFileNameOff();
      this->CacheDirectoryButton->GetWidget()->SetText(app->GetExtensionsInstallPath());
      this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetLastPath(app->GetExtensionsInstallPath());
      }
    }

  if (this->SearchLocationBox)
    {
    // :TODO: 20090405 tgl: URL below should be configurable.

    std::string ext_slicer_org("http://ext.slicer.org/ext/");

    ext_slicer_org += app->GetSvnUrl();
    ext_slicer_org += "/";
    ext_slicer_org += app->GetSvnRevision();
    ext_slicer_org += "-";
    ext_slicer_org += app->GetPlatform();
    
    this->GetWizardDialog()->SetSelectedRepositoryURL( ext_slicer_org );
        
    this->SearchLocationBox->SetValue(this->GetWizardDialog()->GetSelectedRepositoryURL().c_str());
    }

  if (this->ActionRadioButtonSet)
    {
    this->ActionRadioButtonSet->GetWidget(vtkSlicerModulesConfigurationStep::ActionInstall)->Select();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkSlicerModulesConfigurationStep::ActionRadioButtonSetChangedCallback()
{
  int result = 0;

  if (vtkSlicerModulesConfigurationStep::ActionUninstall == this->GetSelectedAction())
    {
    if (this->CacheDirectoryButton)
      this->CacheDirectoryButton->EnabledOff();
    if (this->TrashButton)
      this->TrashButton->EnabledOff();
    if (this->SearchLocationBox)
      this->SearchLocationBox->EnabledOff();      
    }
  else
    {
    if (this->CacheDirectoryButton)
      this->CacheDirectoryButton->EnabledOn();
    if (this->TrashButton)
      this->TrashButton->EnabledOn();
    if (this->SearchLocationBox)
      this->SearchLocationBox->EnabledOn();     
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkSlicerModulesConfigurationStep::IsRepositoryValid()
{
  int result = 1;
  
  if (vtkSlicerModulesConfigurationStep::ActionInstall == this->GetSelectedAction() ||
      vtkSlicerModulesConfigurationStep::ActionEither == this->GetSelectedAction())
    {      
    std::string url = this->SearchLocationBox->GetValue();
      
    vtkSlicerApplication *app = 
      vtkSlicerApplication::SafeDownCast(this->GetApplication());

    const char* tmp = app->GetTemporaryDirectory();
    std::string tmpfile(tmp);
    tmpfile += "/manifest.html";

    if (itksys::SystemTools::FileExists(tmpfile.c_str()))
      {
      itksys::SystemTools::RemoveFile(tmpfile.c_str());
      }

    vtkHTTPHandler *handler = vtkHTTPHandler::New();
    handler->SetForbidReuse(1);
      
    if (0 != handler->CanHandleURI(url.c_str()))
      {
      handler->StageFileRead(url.c_str(), tmpfile.c_str());
      }
 
    handler->Delete();
      
    if (itksys::SystemTools::FileExists(tmpfile.c_str()) &&
        itksys::SystemTools::FileLength(tmpfile.c_str()) > 0)
      {
      result = 0;
      }

    }
  else
    {
    // :NOTE: 20090728 tgl: Action is uninstall, repository checking
    // is pointless.
    result = 0;
    }
  
  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::Validate()
{
  vtkKWWizardWidget *wizard_widget = this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  int valid = this->IsRepositoryValid();
  if (0 == valid)
    {
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
    }
  else
    {
    wizard_widget->SetErrorText("Could not connect to specified repository, check network connection.");
    wizard_workflow->PushInput(this->GetRepositoryValidationFailed());
    }

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkSlicerModulesConfigurationStep::GetSelectedAction()
{
  if (this->ActionRadioButtonSet)
    {
    if (this->ActionRadioButtonSet->GetWidget(vtkSlicerModulesConfigurationStep::ActionInstall)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionInstall;
      }
    if (this->ActionRadioButtonSet->GetWidget(vtkSlicerModulesConfigurationStep::ActionUninstall)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionUninstall;
      }
    if (this->ActionRadioButtonSet->GetWidget(vtkSlicerModulesConfigurationStep::ActionEither)->GetSelectedState())
      {
      return vtkSlicerModulesConfigurationStep::ActionEither;
      }
    }

  return vtkSlicerModulesConfigurationStep::ActionUnknown;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::CacheDirectoryCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetExtensionsInstallPath(this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::SearchLocationCallback(const char* value)
{
  this->GetWizardDialog()->SetSelectedRepositoryURL(value);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesConfigurationStep::EmptyCacheDirectoryCommand()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {

    }
}

