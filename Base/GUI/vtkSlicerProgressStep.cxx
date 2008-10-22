#include "vtkSlicerProgressStep.h"
#include "vtkSlicerModulesStep.h"

#include "vtkStringArray.h"

#include "vtkObjectFactory.h" 

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWProgressGauge.h"

#include "vtkHTTPHandler.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesWizardDialog.h"


#include <vtksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerProgressStep );
vtkCxxRevisionMacro(vtkSlicerProgressStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerProgressStep::vtkSlicerProgressStep()
{
  this->SetName("Progress");
  this->SetDescription("Specify loadable module path.");
  this->WizardDialog = NULL;
  this->ProgressGauge = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerProgressStep::~vtkSlicerProgressStep()
{
  if (this->ProgressGauge)
    {
    this->ProgressGauge->Delete();
    }
  this->SetWizardDialog(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerProgressStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerProgressStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkSlicerModulesWizardDialog *wizard_dialog = 
    dynamic_cast<vtkSlicerModulesWizardDialog*> (this->GetWizardDialog());

  vtkKWWizardWidget *wizard_widget = 
    wizard_dialog->GetWizardWidget();

  vtkStringArray *modules = wizard_dialog->GetModulesStep()->GetSelectedModules();
  
  for (int i=0; i<modules->GetSize(); i++)
    {
      Install(modules->GetValue(i));
    }

  if (!this->ProgressGauge)
    {
    this->ProgressGauge = vtkKWProgressGauge::New();
    }
  if (!this->ProgressGauge->IsCreated())
    {
    this->ProgressGauge->SetParent(wizard_widget->GetClientArea());
    this->ProgressGauge->Create();
    }
  
  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->ProgressGauge->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkSlicerProgressStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
void vtkSlicerProgressStep::Validate()
{
  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = wizard_widget->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
void vtkSlicerProgressStep::Install(const std::string& url)
{
  
  vtkHTTPHandler *handler = vtkHTTPHandler::New();

  if (0 != handler->CanHandleURI(url.c_str()))
    {
      std::string::size_type pos = url.rfind("/");
      std::string libname = url.substr(pos);
      
      vtksys_stl::string slicerHome;
      vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome);

      std::string file(slicerHome + std::string("/") + Slicer3_INSTALL_MODULES_LIB_DIR + std::string("/") + libname);

      handler->StageFileRead(url.c_str(), file.c_str());
    }

  handler->Delete();
  
}
