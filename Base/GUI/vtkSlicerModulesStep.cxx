#include "vtkSlicerModulesStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMultiColumnList.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModulesWizardDialog.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesStep );
vtkCxxRevisionMacro(vtkSlicerModulesStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerModulesStep::vtkSlicerModulesStep()
{
  this->SetName("Modules");
  this->SetDescription("Select a Slicer3 Loadable Module.");
  this->WizardDialog = NULL;
  this->ModulesMultiColumnList = NULL;
  
  this->SelectedModules = vtkStringArray::New();
}

//----------------------------------------------------------------------------
vtkSlicerModulesStep::~vtkSlicerModulesStep()
{
  if (this->ModulesMultiColumnList)
    {
    this->ModulesMultiColumnList->Delete();
    }
  this->SetWizardDialog(NULL);

  this->SelectedModules->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SetWizardDialog(vtkSlicerModulesWizardDialog *arg)
{
  this->WizardDialog = arg;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = 
    this->GetWizardDialog()->GetWizardWidget();

  // Show list of available modules for downloaded

  if (!this->ModulesMultiColumnList)
    {
    this->ModulesMultiColumnList = vtkKWMultiColumnList::New();
    }
  if (!this->ModulesMultiColumnList->IsCreated())
    {


    vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
    const char* tmp = app->GetTemporaryDirectory();
    std::string tmpfile(tmp);
    tmpfile += "/manifest.txt";

    std::ifstream ifs(tmpfile.c_str());

    char *TXT = 0;

    if (!ifs.fail())
      {
        ifs.seekg(0, std::ios::end);
        size_t len = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        TXT = new char[len+1];
        ifs.read(TXT, len);
        TXT[len] = '\n';
      }


    ifs.close();

    std::vector<ManifestEntry> modules = this->ParseManifest(TXT);

    delete[] TXT;

    this->ModulesMultiColumnList->SetParent(wizard_widget->GetClientArea());
    this->ModulesMultiColumnList->Create();
    this->ModulesMultiColumnList->SetBalloonHelpString(
      "A list of available modules.");
    this->ModulesMultiColumnList->SetWidth(0);
    this->ModulesMultiColumnList->SetSelectionTypeToCell();

    int col_index;

    col_index = this->ModulesMultiColumnList->AddColumn("Name");

    col_index = this->ModulesMultiColumnList->AddColumn("Installed");
    col_index = this->ModulesMultiColumnList->AddColumn("Available");
    this->ModulesMultiColumnList->SetColumnAlignmentToCenter(col_index);

    col_index = this->ModulesMultiColumnList->AddColumn("Action");
    this->ModulesMultiColumnList->SetColumnWidth(col_index, 10);
    this->ModulesMultiColumnList->SetColumnEditWindowToSpinBox(col_index);
    this->ModulesMultiColumnList->SetColumnFormatCommandToEmptyOutput(col_index);
    this->ModulesMultiColumnList->ColumnEditableOn(col_index);

    col_index = this->ModulesMultiColumnList->AddColumn("URL");

    // Insert each module entry
    
    const char* values[] = {"Skip", "Install"};
    
    for (unsigned int i = 0; i < modules.size(); i++)
      {
        ManifestEntry module = modules[i];
        this->ModulesMultiColumnList->InsertCellText(i, 0, module.Name.c_str());
        this->ModulesMultiColumnList->InsertCellText(i, 2, module.Version.c_str());

        this->ModulesMultiColumnList->SetCellWindowCommandToComboBoxWithValues(i, 3, 2, values);
        this->ModulesMultiColumnList->SetCellText(i, 3, "Skip");
        this->ModulesMultiColumnList->InsertCellText(i, 4, module.URL.c_str());
      }

    }
  
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ModulesMultiColumnList->GetWidgetName());
}


//----------------------------------------------------------------------------
vtkStringArray* vtkSlicerModulesStep::GetSelectedModules()
{
  this->SelectedModules->Reset();

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      std::string action(this->ModulesMultiColumnList->GetCellText(row, 3));
      if (action.compare("Install") == 0)
        {
          std::string url(this->ModulesMultiColumnList->GetCellText(row, 4));
          this->SelectedModules->InsertNextValue( url.c_str() );
        }

    }

  return this->SelectedModules;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->GetWizardDialog()->GetWizardWidget()->ClearPage();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetWizardDialog()->GetWizardWidget()->GetWizardWorkflow();

  // This step always validates

  wizard_workflow->PushInput(
    vtkKWWizardStep::GetValidationSucceededInput());

  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
std::vector<vtkSlicerModulesStep::ManifestEntry> vtkSlicerModulesStep::ParseManifest(const std::string& txt)
{
  std::vector<ManifestEntry> result;

  if (txt.empty()) {
    return result;
  }

  std::string::size_type prev = 0;
  std::string::size_type space = txt.find(" ", prev);
  std::string::size_type newline = txt.find("\n", prev);

  ManifestEntry entry;

  // :NOTE: 20081003 tgl: Put in a sanity check of 10,000 to
  // prevent an infinite loop.  Get Out The Vote 2008!

  int count = 0;
  while (space != std::string::npos && count < 10000)
    {
      entry.URL = txt.substr(prev, space - prev);

      prev = space;
      space = txt.find(" ", space + 1);

      entry.Version = txt.substr(prev, space - prev);
      entry.Name = txt.substr(space, newline - space);

      prev = newline + 1;
      space = txt.find(" ", newline + 1);
      newline = txt.find("\n", newline + 1);

      result.push_back(entry);
      
      count++;
    }

  return result;
}
