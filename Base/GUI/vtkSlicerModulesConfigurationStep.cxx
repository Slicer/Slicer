#include "vtkSlicerModulesConfigurationStep.h"

#include "vtkSlicerConfigure.h"
#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWPushButton.h"
#include "vtkKWStateMachineInput.h"
#include "vtkKWFrame.h"

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
  this->SetName("Extensions Management Wizard");
  this->WizardDialog = NULL;

  this->Frame1 = NULL;
  this->Frame2 = NULL;
  this->Frame3 = NULL;

  this->HeaderIcon = NULL;
  this->HeaderText = NULL;
  this->ActionRadioButtonSet = NULL;
  this->CacheDirectoryButton = NULL;
  this->TrashButton = NULL;
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

  this->Script("pack %s %s %s -side top -pady 5",
               this->Frame1->GetWidgetName(),
               this->Frame2->GetWidgetName(),
               this->Frame3->GetWidgetName());

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
    this->HeaderText->SetText("This wizard lets you search for extensions to add to 3D Slicer,\ndownload and install them, and uninstall existing extensions.");
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

  int label_width = 25;

  if (!this->CacheDirectoryButton)
    {
    this->CacheDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
    }
  if (!this->CacheDirectoryButton->IsCreated())
    {
    this->CacheDirectoryButton->SetParent( this->Frame3 );
    this->CacheDirectoryButton->Create();
    this->CacheDirectoryButton->SetLabelText("Download (cache) directory:");
    this->CacheDirectoryButton->SetLabelWidth(label_width);
    this->CacheDirectoryButton->GetWidget()->SetCommand(this, "CacheDirectoryCallback");
    }

  if (!this->TrashButton)
    {
    this->TrashButton = vtkKWPushButton::New();
    }
  if (!this->TrashButton->IsCreated())
    {
    this->TrashButton->SetParent( this->Frame3 );
    this->TrashButton->Create();
    this->TrashButton->SetImageToPredefinedIcon(vtkKWIcon::IconTrashcan);
    }

  if (!this->SearchLocationBox)
    {
    this->SearchLocationBox = vtkKWComboBoxWithLabel::New();
    }
  if (!this->SearchLocationBox->IsCreated())
    {
    this->SearchLocationBox->SetParent( this->Frame3 );
    this->SearchLocationBox->Create();
    this->SearchLocationBox->SetLabelText("Where to search:");
    this->SearchLocationBox->SetLabelWidth(label_width);
    }
 
  this->Script("pack %s %s -side left", 
               this->HeaderIcon->GetWidgetName(),
               this->HeaderText->GetWidgetName());
  
  this->Script("pack %s -side left -pady 20", 
               this->ActionRadioButtonSet->GetWidgetName());

  this->Script("grid %s -row 0 -column 0 -sticky e -padx 5 -pady 5",
               this->CacheDirectoryButton->GetWidgetName());

  this->Script("grid %s -row 0 -column 1 -sticky e -padx 5 -pady 5",
               this->TrashButton->GetWidgetName());
 
  this->Script("grid %s -row 1 -column 0 -columnspan 2 -sticky e -padx 5 -pady 5",
               this->SearchLocationBox->GetWidgetName());

  this->Script("grid columnconfigure %s 0 -weight 1",
               this->Frame3->GetWidgetName());

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
      this->CacheDirectoryButton->GetWidget()->SetText(app->GetModuleCachePath());
      this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetLastPath(app->GetModuleCachePath());
      }
    }

  if (this->SearchLocationBox)
    {
    std::string txtfile(app->GetBinDir());
    txtfile += "/../";
    txtfile += Slicer3_INSTALL_LIB_DIR;
    txtfile += "/";
    txtfile += "Slicer3Version.txt";

    std::string platform;
    std::string build_date;
    std::string svnurl;
    std::string svnrevision;

    std::cout << "txtfile: " << txtfile << std::endl;

    std::ifstream ifs(txtfile.c_str());

    std::string line;
    while (std::getline(ifs, line, '\n'))
      {
      if (line.find("build ") == 0)
      {
      platform = line.substr(6);
      }
      else if (line.find("buildDate ") == 0)
      {
      build_date = line.substr(10);
      }
      else if (line.find("svnurl ") == 0)
      {
        svnurl = line.substr(7);
      }
      else if (line.find("svnrevision ") == 0)
      {
        svnrevision = line.substr(12);
      }
    }

    ifs.close();

    // :TODO: 20090405 tgl: URL below should be configurable.

    std::string ext_slicer_org("http://ext.slicer.org/ext/");

    if (!svnurl.empty())
      {
      int pos = svnurl.find_last_of("/");
      ext_slicer_org += svnurl.substr(pos);
      ext_slicer_org += "/";
      ext_slicer_org += svnrevision;
      ext_slicer_org += "-";
      ext_slicer_org += platform;
      }

    this->GetWizardDialog()->SetSelectedRepositoryURL( ext_slicer_org );
        
    this->SearchLocationBox->GetWidget()->SetValue(this->GetWizardDialog()->GetSelectedRepositoryURL().c_str());
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
    std::string url = this->SearchLocationBox->GetWidget()->GetValue();
      
    vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

    const char* tmp = app->GetTemporaryDirectory();
    std::string tmpfile(tmp);
    tmpfile += "/manifest.html";

    if (itksys::SystemTools::FileExists(tmpfile.c_str()))
      {
      itksys::SystemTools::RemoveFile(tmpfile.c_str());
      }

    vtkHTTPHandler *handler = vtkHTTPHandler::New();
      
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
    app->SetModuleCachePath(this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

