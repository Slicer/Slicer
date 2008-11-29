#include "vtkSlicerRepositoryStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWCheckButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButton.h"
#include "vtkKWStateMachineInput.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModulesWizardDialog.h"

#include "vtkHTTPHandler.h"

#include <itksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerRepositoryStep );
vtkCxxRevisionMacro(vtkSlicerRepositoryStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkSlicerRepositoryStep::vtkSlicerRepositoryStep()
{
  this->SetName("Repository");
  this->SetDescription("Select a Slicer3 Loadable Module repository.");
  this->WizardDialog = NULL;
  this->RepositoryRadioButtonSet = NULL;
  this->RepositoryCheckButton = NULL;

  this->RepositoryValidationFailed = vtkKWStateMachineInput::New();
  this->RepositoryValidationFailed->SetName("failed");
}

//----------------------------------------------------------------------------
vtkSlicerRepositoryStep::~vtkSlicerRepositoryStep()
{
  if (this->RepositoryRadioButtonSet)
    {
    this->RepositoryRadioButtonSet->Delete();
    }
  if (this->RepositoryCheckButton)
    {
    this->RepositoryCheckButton->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerRepositoryStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerRepositoryStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  // Create radio buttons for each of the mathematical operator we support

  if (!this->RepositoryRadioButtonSet)
    {
    this->RepositoryRadioButtonSet = vtkKWRadioButtonSet::New();
    }
  if (!this->RepositoryRadioButtonSet->IsCreated())
    {
    this->RepositoryRadioButtonSet->SetParent(wizard_widget->GetClientArea());
    this->RepositoryRadioButtonSet->Create();

    vtkKWRadioButton *radiob;

    radiob = this->RepositoryRadioButtonSet->AddWidget(
      vtkSlicerRepositoryStep::RepositoryNAMICSandbox);
    radiob->SetText("NAMIC Sandbox");
    radiob->SetCommand(wizard_widget, "Update");

    radiob = this->RepositoryRadioButtonSet->AddWidget(
      vtkSlicerRepositoryStep::RepositoryNITRC);
    radiob->SetText("NITRC");
    radiob->SetCommand(wizard_widget, "Update");
 
    radiob = this->RepositoryRadioButtonSet->AddWidget(
      vtkSlicerRepositoryStep::RepositorySlicer);
    radiob->SetText("Slicer.org");
    radiob->SetCommand(wizard_widget, "Update");

    this->RepositoryRadioButtonSet->GetWidget(
      vtkSlicerRepositoryStep::RepositoryNAMICSandbox)->Select();
    }
 
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->RepositoryRadioButtonSet->GetWidgetName());

  // checkbox to download possible sources, too

  if (!this->RepositoryCheckButton)
    {
    this->RepositoryCheckButton = vtkKWCheckButton::New();
    }
  if (!this->RepositoryCheckButton->IsCreated())
    {
    this->RepositoryCheckButton->SetParent(wizard_widget->GetClientArea());
    this->RepositoryCheckButton->Create();

    this->RepositoryCheckButton->SetText("Just sources?");
    
    }

  this->Script("pack %s -side top -anchor s -expand n -padx 2 -pady 2", 
               this->RepositoryCheckButton->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkSlicerRepositoryStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
int vtkSlicerRepositoryStep::GetSelectedRepository()
{
  if (this->RepositoryRadioButtonSet)
    {
    if (this->RepositoryRadioButtonSet->GetWidget(
          vtkSlicerRepositoryStep::RepositoryNITRC)->GetSelectedState())
      {
      return vtkSlicerRepositoryStep::RepositoryNITRC;
      }
    if (this->RepositoryRadioButtonSet->GetWidget(
          vtkSlicerRepositoryStep::RepositorySlicer)->GetSelectedState())
      {
      return vtkSlicerRepositoryStep::RepositorySlicer;
      }
    if (this->RepositoryRadioButtonSet->GetWidget(
          vtkSlicerRepositoryStep::RepositoryNAMICSandbox)->GetSelectedState())
      {
      return vtkSlicerRepositoryStep::RepositoryNAMICSandbox;
      }
    }

  return vtkSlicerRepositoryStep::RepositoryUnknown;
}

//----------------------------------------------------------------------------
int vtkSlicerRepositoryStep::IsRepositoryValid()
{
  int result = vtkSlicerRepositoryStep::RepositoryError;
  
  std::string url("http://svn.slicer.org/Slicer3/trunk/Modules/"); 

  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  const char* tmp = app->GetTemporaryDirectory();
  std::string tmpfile(tmp);
  tmpfile += "/manifest.txt";

  if (itksys::SystemTools::FileExists(tmpfile.c_str()))
    {
    itksys::SystemTools::RemoveFile(tmpfile.c_str());
    }
 
  if (vtkSlicerRepositoryStep::RepositoryNITRC == this->GetSelectedRepository())
    {
    url += "nitrc-manifest";
    }
  else if (vtkSlicerRepositoryStep::RepositorySlicer == this->GetSelectedRepository())
    {
    url += "slicer.org-manifest";
    }
  else if (vtkSlicerRepositoryStep::RepositoryNAMICSandbox == this->GetSelectedRepository())
    {
    url += "namicsandbox-manifest";
    }
  else
    {
    result = vtkSlicerRepositoryStep::RepositoryUnknown;
    }

  if (result != vtkSlicerRepositoryStep::RepositoryUnknown)
    {
    if (this->RepositoryCheckButton->GetSelectedState())
      {
      url += ".source.txt";
      }
    else
      {
      // :NOTE: 20081021 tgl: Is there a better way?

#if defined(_WIN32) || defined(__WIN32__)
      url += ".win32.txt";
#else
# if defined(linux) || defined(__linux)
      url += ".linux.txt";
# else
#  ifdef __APPLE__
      url += ".darwin.txt";
#  else
#   if defined(sun) || defined(__sun)
#    if defined(__SVR4) || defined(__svr4__)
      url += ".solaris.txt";
#    else
      url += ".sunos.txt";
#    endif
#   else
      url += ".txt";
#   endif
#  endif
# endif
#endif

    }

    vtkHTTPHandler *handler = vtkHTTPHandler::New();

    if (0 != handler->CanHandleURI(url.c_str()))
      {
      handler->StageFileRead(url.c_str(), tmpfile.c_str());
      }
 
    handler->Delete();

    if (itksys::SystemTools::FileExists(tmpfile.c_str()))
      {
      result = vtkSlicerRepositoryStep::RepositoryIsValid;
      }
    else
      {
      result = vtkSlicerRepositoryStep::RepositoryConnectionError;
      }

    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerRepositoryStep::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  int valid = this->IsRepositoryValid();
  if (valid == vtkSlicerRepositoryStep::RepositoryConnectionError || 
      valid == vtkSlicerRepositoryStep::RepositoryError)
    {
      wizard_widget->SetErrorText("Could not connect to specified repository.");
      wizard_workflow->PushInput(this->GetRepositoryValidationFailed());
    }
  else
    {
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
    }

  wizard_workflow->ProcessInputs();
}
