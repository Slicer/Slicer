#include "vtkSlicerModulesStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"

#include "vtkHTTPHandler.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesWizardDialog.h"
#include "vtkSlicerModulesConfigurationStep.h"

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesStep );
vtkCxxRevisionMacro(vtkSlicerModulesStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerModulesStep::vtkSlicerModulesStep()
{
  this->SetName("Extension");
  this->SetDescription("Select a Slicer3 Extension.");
  this->HeaderText = NULL;
  this->SelectAllButton = NULL;
  this->SelectNoneButton = NULL;
  this->ModulesMultiColumnList = NULL;
  this->DownloadButton = NULL;
  this->UninstallButton = NULL;
  this->WizardDialog = NULL;

  this->Messages["READY"] = "Select extensions, then click uninstall to remove them from\nyour version of 3D Slicer, or click download to retrieve them.";
  this->Messages["DOWNLOAD"] = "Download in progress... Clicking the cancel button will stop\nthe process after the current extension operation is finished.";
  this->Messages["FINISHED"] = "Continue selecting extensions for download or removal,\nor click finish to complete the operation.";
}

//----------------------------------------------------------------------------
vtkSlicerModulesStep::~vtkSlicerModulesStep()
{
  if (this->HeaderText)
    {
    this->HeaderText->Delete();
    }
  if (this->SelectAllButton)
    {
    this->SelectAllButton->Delete();
    }
  if (this->SelectNoneButton)
    {
    this->SelectNoneButton->Delete();
    }
  if (this->ModulesMultiColumnList)
    {
    this->ModulesMultiColumnList->Delete();
    }
  if (this->DownloadButton)
    {
    this->DownloadButton->Delete();
    }
  if (this->UninstallButton)
    {
    this->UninstallButton->Delete();
    }

  this->SetWizardDialog(NULL);

  std::vector<ManifestEntry*>::iterator iter = this->Modules.begin();
  while (iter != this->Modules.end())
    {
      delete (*iter);
      iter++;
    }
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

  // left most column checkbox to select download
  // radio button set along top: select all, deselect all

  // Show list of available modules for downloaded

  // Script("Slicer3OpenLink %s") see QueryAtlas multi-column

  // along bottom buttons to:
  //-- download now
  //-- uninstall (delete?)

  // status bar or progress along bottom

  // disable next until download or uninstall is selected (once? once
  // after selection change?)

  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    }
  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( wizard_widget->GetClientArea() );
    this->HeaderText->Create();
    this->HeaderText->SetText(this->Messages["READY"].c_str());
    }

  if (!this->SelectAllButton)
    {
    this->SelectAllButton = vtkKWPushButton::New();
    }
  if (!this->SelectAllButton->IsCreated())
    {
    this->SelectAllButton->SetParent( wizard_widget->GetClientArea() );
    this->SelectAllButton->Create();

    this->SelectAllButton->SetText("Select All");
    this->SelectAllButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectAllIcon());
    this->SelectAllButton->SetCommand(this, "SelectAll");
    }

  if (!this->SelectNoneButton)
    {
    this->SelectNoneButton = vtkKWPushButton::New();
    }
  if (!this->SelectNoneButton->IsCreated())
    {
    this->SelectNoneButton->SetParent( wizard_widget->GetClientArea() );
    this->SelectNoneButton->Create();
    
    this->SelectNoneButton->SetText("Select None");
    this->SelectNoneButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectNoneIcon());
    this->SelectNoneButton->SetCommand(this, "SelectNone");
    }

  if (!this->ModulesMultiColumnList)
    {
    this->ModulesMultiColumnList = vtkKWMultiColumnList::New();
    }
  if (!this->ModulesMultiColumnList->IsCreated())
    {

    const char* tmp = app->GetTemporaryDirectory();
    std::string tmpfile(tmp);
    tmpfile += "/manifest.html";

    std::ifstream ifs(tmpfile.c_str());

    char *HTML = 0;

    if (!ifs.fail())
      {
        ifs.seekg(0, std::ios::end);
        size_t len = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        HTML = new char[len+1];
        ifs.read(HTML, len);
        HTML[len] = '\n';
      }


    ifs.close();

    std::vector<ManifestEntry*>::iterator iter = this->Modules.begin();
    while (iter != this->Modules.end())
      {
        delete (*iter);
        iter++;
      }
    
    this->Modules = this->ParseManifest(HTML);

    delete[] HTML;

    this->ModulesMultiColumnList->SetParent(wizard_widget->GetClientArea());
    this->ModulesMultiColumnList->Create();
    this->ModulesMultiColumnList->SetBalloonHelpString(
      "A list of available extensions.");
    this->ModulesMultiColumnList->SetWidth(0);
    this->ModulesMultiColumnList->SetSelectionTypeToCell();

    int col_index;

    col_index = this->ModulesMultiColumnList->AddColumn("Select");
    this->ModulesMultiColumnList->SetColumnEditWindowToCheckButton(col_index);
    this->ModulesMultiColumnList->ColumnEditableOn(col_index);
    
    col_index = this->ModulesMultiColumnList->AddColumn("Status");
    col_index = this->ModulesMultiColumnList->AddColumn("Name");
    col_index = this->ModulesMultiColumnList->AddColumn("Category");
    col_index = this->ModulesMultiColumnList->AddColumn("Description");
    col_index = this->ModulesMultiColumnList->AddColumn("HomePage");
    col_index = this->ModulesMultiColumnList->AddColumn("Binary URL");

    // Insert each extension entry
    for (unsigned int i = 0; i < this->Modules.size(); i++)
      {
      ManifestEntry *extension = this->Modules[i];

      this->ModulesMultiColumnList->InsertCellText(i, 2, extension->Name.c_str());
      this->ModulesMultiColumnList->InsertCellText(i, 6, extension->URL.c_str());

      this->ModulesMultiColumnList->SetCellWindowCommandToCheckButton(i, 0);

      this->ModulesMultiColumnList->SetCellImageToIcon(i, 1, app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerNotFoundOnDiskIcon());
      }
    }

  if (!this->DownloadButton)
    {
    this->DownloadButton = vtkKWPushButton::New();
    }
  if (!this->DownloadButton->IsCreated())
    {
    this->DownloadButton->SetParent( wizard_widget->GetClientArea() );
    this->DownloadButton->Create();
    this->DownloadButton->SetText("Download & Install");
    this->DownloadButton->SetCommand(this, "DownloadInstall");
    }

  if (!this->UninstallButton)
    {
    this->UninstallButton = vtkKWPushButton::New();
    }
  if (!this->UninstallButton->IsCreated())
    {
    this->UninstallButton->SetParent( wizard_widget->GetClientArea() );
    this->UninstallButton->Create();
    this->UninstallButton->SetText("Uninstall");
    this->UninstallButton->SetCommand(this, "Uninstall");
    }

  this->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2", 
               this->HeaderText->GetWidgetName());

  this->Script("pack %s %s -side top -anchor nw -expand y -padx 2 -pady 2", 
               this->SelectAllButton->GetWidgetName(),
               this->SelectNoneButton->GetWidgetName());

  this->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2", 
               this->ModulesMultiColumnList->GetWidgetName());

  this->Script("pack %s %s -side top -anchor nw -expand y -padx 2 -pady 2", 
               this->DownloadButton->GetWidgetName(),
               this->UninstallButton->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectAll()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      this->ModulesMultiColumnList->SetCellTextAsInt(row, 0, 1);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectNone()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      this->ModulesMultiColumnList->SetCellTextAsInt(row, 0, 0);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadInstall()
{

  this->HeaderText->SetText(this->Messages["DOWNLOAD"].c_str());

  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
  vtkKWIcon *done = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDoneIcon();
  vtkKWIcon *wait = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWaitIcon();

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
        {
          this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, wait);
          this->DownloadInstallExtension(this->ModulesMultiColumnList->GetCellText(row, 6));
          this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, done);
        }
    }

  this->HeaderText->SetText(this->Messages["FINISHED"].c_str());
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Uninstall()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
        {
          this->UninstallExtension(this->ModulesMultiColumnList->GetCellText(row, 3));
        }
    }
}

std::vector<ManifestEntry*> vtkSlicerModulesStep::GetSelectedModules()
{
  std::vector<ManifestEntry*> Selected;

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      std::string action(this->ModulesMultiColumnList->GetCellText(row, 3));
      if (action.compare("Install") == 0)
        {
          std::string url(this->ModulesMultiColumnList->GetCellText(row, 4));
          Selected.push_back( new ManifestEntry(*this->Modules[row]) );
        }

    }

  return Selected;
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
std::vector<ManifestEntry*> vtkSlicerModulesStep::ParseManifest(const std::string& txt)
{
  std::vector<ManifestEntry*> result;

  if (txt.empty()) {
    return result;
  }

  vtkSlicerModulesWizardDialog *wizard_dlg = this->GetWizardDialog();

  std::string baseURL = wizard_dlg->GetModulesConfigurationStep()->GetSelectedRepositoryURL();

  std::string key(".zip\">");

  std::string::size_type zip = txt.find(key, 0);
  std::string::size_type atag = txt.find("</a>", zip);
  std::string::size_type dash = txt.find("-", zip);
 
  ManifestEntry* entry;

  // :NOTE: 20081003 tgl: Put in a sanity check of 10,000 to
  // prevent an infinite loop.  Get Out The Vote 2008!

  int count = 0;
  while (zip != std::string::npos && count < 10000)
    {
    entry = new ManifestEntry;

    if (std::string::npos != dash)
      {
      entry->URL = baseURL;
      entry->URL += "/";
      entry->URL += txt.substr(zip + key.size(), atag - (zip + key.size()));
      entry->Name = txt.substr(zip + key.size(), dash - (zip + key.size()));
      
      zip = txt.find(key, zip + 1);
      dash = txt.find("-", zip);

      result.push_back(entry);
      }
      
    count++;
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadInstallExtension(const std::string& ExtensionBinaryURL)
{
  vtkHTTPHandler *handler = vtkHTTPHandler::New();

  if (0 != handler->CanHandleURI(ExtensionBinaryURL.c_str()))
    {
      std::string::size_type pos = ExtensionBinaryURL.rfind("/");
      std::string zipname = ExtensionBinaryURL.substr(pos + 1);
      
      vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
      
      const char* tmp = app->GetTemporaryDirectory();
      std::string tmpfile(tmp);
      tmpfile += "/";
      tmpfile += zipname;
      
      handler->StageFileRead(ExtensionBinaryURL.c_str(), tmpfile.c_str());
      
      vtksys_stl::string slicerHome;
      vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome);

      std::string file(slicerHome + std::string("/") + Slicer3_INSTALL_MODULES_LIB_DIR + std::string("/") + zipname);

      std::string tmpdir(tmp);
      tmpdr += "/";
      tmpdir += "extension";

      app->Script("$::Loader::Mount %s %s", tmpfile.c_str(), tmpdir.c_str());
    }

  handler->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::UninstallExtension(const std::string& ExtensionName)
{
}
