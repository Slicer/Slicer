#include "vtkSlicerModulesResultStep.h"
#include "vtkSlicerModulesStep.h"

#include "vtkStringArray.h"

#include "vtkObjectFactory.h" 

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWText.h"

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
  this->SetName("Progress");
  this->SetDescription("Specify loadable module path.");
  this->WizardDialog = NULL;
  this->ProgressGauge = NULL;
  this->Text = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModulesResultStep::~vtkSlicerModulesResultStep()
{
  if (this->ProgressGauge)
    {
    this->ProgressGauge->Delete();
    }
  if (this->Text)
    {
    this->Text->Delete();
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

  std::vector<ManifestEntry*> modules = wizard_dialog->GetModulesStep()->GetSelectedModules();
  
  std::stringstream messg;
  bool none = true;

  for (unsigned int i=0; i<modules.size(); i++)
    {
      if (modules[i]->Version.compare("source") == 0) {
        this->InstallSource(modules[i]->URL);
        messg << "Source install: " << modules[i]->Name << std::endl;
        none = false;
      } else {
        this->Install(modules[i]->URL);
        messg << "Binary install: " << modules[i]->Name << " " << modules[i]->URL << std::endl;
        none = false;
      }
    }

  if (none)
    {
      messg << "No modules installed" << std::endl;
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
  if (!this->Text)
    {
    this->Text = vtkKWText::New();
    }
  
  if (!this->Text->IsCreated())
    {
    this->Text->SetParent(wizard_widget->GetClientArea());
    this->Text->SetEnabled(0);
    this->Text->Create();
    }

  this->Text->SetText(messg.str().c_str());

  this->Script("pack %s -side top -expand y -fill none -anchor center", 
               this->Text->GetWidgetName());
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
void vtkSlicerModulesResultStep::Install(const std::string& url)
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

//----------------------------------------------------------------------------
void vtkSlicerModulesResultStep::InstallSource(const std::string& url)
{
  vtksys_stl::string slicerHome;
  vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome);
  
  std::string slicerModulesSource = slicerHome;
  slicerModulesSource += "/../Slicer3/Modules/loadablemodules.cmake";

  std::ofstream ofs(slicerModulesSource.c_str(), std::ios::app);

  ofs << "slicer_parse_module_url(\"" << url << "\")" << std::endl;

  ofs.close();
}
