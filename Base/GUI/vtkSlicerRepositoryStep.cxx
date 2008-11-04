#include "vtkSlicerRepositoryStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButton.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModulesWizardDialog.h"

#include "vtkHTTPHandler.h"

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
}

//----------------------------------------------------------------------------
vtkSlicerRepositoryStep::~vtkSlicerRepositoryStep()
{
  if (this->RepositoryRadioButtonSet)
    {
    this->RepositoryRadioButtonSet->Delete();
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
      vtkSlicerRepositoryStep::RepositoryNITRC);
    radiob->SetText("NITRC");
    radiob->SetCommand(wizard_widget, "Update");

    this->RepositoryRadioButtonSet->GetWidget(
      vtkSlicerRepositoryStep::RepositoryNITRC)->Select();
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->RepositoryRadioButtonSet->GetWidgetName());
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
    }

  return vtkSlicerRepositoryStep::RepositoryUnknown;
}

//----------------------------------------------------------------------------
int vtkSlicerRepositoryStep::IsRepositoryValid()
{
  int result = vtkSlicerRepositoryStep::RepositoryError;
  
  vtkHTTPHandler *handler = vtkHTTPHandler::New();

  if (vtkSlicerRepositoryStep::RepositoryNITRC == this->GetSelectedRepository())
    {


      std::string url("http://svn.slicer.org/Slicer3/trunk/Modules/"); 

      // :NOTE: 20081021 tgl: Is there a better way?

#if defined(_WIN32) || defined(__WIN32__)
      url += "nitrc-manifest.win32.txt";
#else
# if defined(linux) || defined(__linux)
      url += "nitrc-manifest.linux.txt";
# else
#  ifdef __MACOSX__
      url += "nitrc-manifest.darwin.txt";
#  else
#   if defined(sun) || defined(__sun)
#    if defined(__SVR4) || defined(__svr4__)
      url += "nitrc-manifest.solaris.txt";
#    else
      url += "nitrc-manifest.sunos.txt";
#    endif
#   else
      url += "nitrc-manifest.txt";
#   endif
#  endif
# endif
#endif

      if (0 == handler->CanHandleURI(url.c_str()))
        {
          result = vtkSlicerRepositoryStep::RepositoryConnectionError;
        }
      else
        {
          vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

          const char* tmp = app->GetTemporaryDirectory();
          std::string tmpfile(tmp);
          tmpfile += "/manifest.txt";

          handler->StageFileRead(url.c_str(), tmpfile.c_str());
        }
    }
  else
    {
      result = vtkSlicerRepositoryStep::RepositoryUnknown;
    }

  handler->Delete();

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerRepositoryStep::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  int valid = this->IsRepositoryValid();
  if (valid == vtkSlicerRepositoryStep::RepositoryConnectionError)
    {
      wizard_widget->SetErrorText("Could not connect to specified repository.");
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }
  else
    {

      wizard_workflow->PushInput(
        vtkKWWizardStep::GetValidationSucceededInput());

    }


  wizard_workflow->ProcessInputs();
}
