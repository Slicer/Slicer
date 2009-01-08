#include "vtkSlicerModulesStep.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWFrame.h"

#include "vtkHTTPHandler.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesWizardDialog.h"

#include <itksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesStep );
vtkCxxRevisionMacro(vtkSlicerModulesStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerModulesStep::vtkSlicerModulesStep()
{
  this->SetName("Extension Management Wizard");
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

  vtkSlicerApplication *app =
    dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  vtkKWFrame *f1 = vtkKWFrame::New();
  vtkKWFrame *f2 = vtkKWFrame::New();
  vtkKWFrame *f3 = vtkKWFrame::New();
  vtkKWFrame *f4 = vtkKWFrame::New();

  f1->SetParent( wizard_widget->GetClientArea() );
  f1->Create();
  f2->SetParent( wizard_widget->GetClientArea() );
  f2->Create();
  f3->SetParent( wizard_widget->GetClientArea() );
  f3->Create();
  f4->SetParent( wizard_widget->GetClientArea() );
  f4->Create();

  this->Script("pack %s %s %s %s -side top",
               f1->GetWidgetName(),
               f2->GetWidgetName(),
               f3->GetWidgetName(),
               f4->GetWidgetName());

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    }
  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( f1 );
    this->HeaderText->Create();
    this->HeaderText->SetText(this->Messages["READY"].c_str());
    }

  if (!this->SelectAllButton)
    {
    this->SelectAllButton = vtkKWPushButton::New();
    }
  if (!this->SelectAllButton->IsCreated())
    {
    this->SelectAllButton->SetParent( f2 );
    this->SelectAllButton->Create();

    this->SelectAllButton->SetText("Select All");
    this->SelectAllButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerSelectAllIcon());
    this->SelectAllButton->SetCommand(this, "SelectAll");
    }

  if (!this->SelectNoneButton)
    {
    this->SelectNoneButton = vtkKWPushButton::New();
    }
  if (!this->SelectNoneButton->IsCreated())
    {
    this->SelectNoneButton->SetParent( f2 );
    this->SelectNoneButton->Create();
    
    this->SelectNoneButton->SetText("Select None");
    this->SelectNoneButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDeselectAllIcon());
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

    this->ModulesMultiColumnList->SetParent( f3 );
    this->ModulesMultiColumnList->Create();
    this->ModulesMultiColumnList->SetBalloonHelpString(
      "A list of available extensions.");
    this->ModulesMultiColumnList->SetWidth(0);
    this->ModulesMultiColumnList->SetHeight(5);
    this->ModulesMultiColumnList->SetSelectionTypeToCell();

    int col_index;

    col_index = this->ModulesMultiColumnList->AddColumn("Select");
    this->ModulesMultiColumnList->SetColumnEditWindowToCheckButton(col_index);
    this->ModulesMultiColumnList->SetColumnFormatCommandToEmptyOutput(col_index);
    
    col_index = this->ModulesMultiColumnList->AddColumn("Status");
    col_index = this->ModulesMultiColumnList->AddColumn("Name");
    col_index = this->ModulesMultiColumnList->AddColumn("Category");
    col_index = this->ModulesMultiColumnList->AddColumn("Description");
    col_index = this->ModulesMultiColumnList->AddColumn("HomePage");
    col_index = this->ModulesMultiColumnList->AddColumn("Binary URL");

    const char* cachedir = app->GetModuleCachePath();
    
    // Insert each extension entry
    for (unsigned int i = 0; i < this->Modules.size(); i++)
      {
      ManifestEntry *extension = this->Modules[i];

      this->ModulesMultiColumnList->InsertCellText(i, 2, extension->Name.c_str());
      this->ModulesMultiColumnList->InsertCellText(i, 6, extension->URL.c_str());

      this->ModulesMultiColumnList->SetCellWindowCommandToCheckButton(i, 0);

      std::string moddir(cachedir + std::string("/") + this->Modules[i]->Name);

      if (itksys::SystemTools::FileExists(moddir.c_str()))
        {
        this->ModulesMultiColumnList->SetCellImageToIcon(i, 1, app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFoundOnDiskIcon());
        }
      else
        {
        this->ModulesMultiColumnList->SetCellImageToIcon(i, 1, app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerNotFoundOnDiskIcon());
        }
      }
    }

  if (!this->DownloadButton)
    {
    this->DownloadButton = vtkKWPushButton::New();
    }
  if (!this->DownloadButton->IsCreated())
    {
    this->DownloadButton->SetParent( f4 );
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
    this->UninstallButton->SetParent( f4 );
    this->UninstallButton->Create();
    this->UninstallButton->SetText("Uninstall");
    this->UninstallButton->SetCommand(this, "Uninstall");
    }

  this->Script("pack %s -side top -pady 10", 
               this->HeaderText->GetWidgetName());

  this->Script("pack %s %s -side left -anchor w -pady 2", 
               this->SelectAllButton->GetWidgetName(),
               this->SelectNoneButton->GetWidgetName());

  this->Script("pack %s -side left", 
               this->ModulesMultiColumnList->GetWidgetName());

  this->Script("pack %s %s -side left -anchor w -pady 2", 
               this->DownloadButton->GetWidgetName(),
               this->UninstallButton->GetWidgetName());

  f1->Delete();
  f2->Delete();
  f3->Delete();
  f4->Delete();

}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectAll()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    this->ModulesMultiColumnList->SetCellText(row, 0, "1");
    }

  this->ModulesMultiColumnList->RefreshAllRowsWithWindowCommand(0);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectNone()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    this->ModulesMultiColumnList->SetCellText(row, 0, "0");
    }

  this->ModulesMultiColumnList->RefreshAllRowsWithWindowCommand(0);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadInstall()
{
  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOff();

  this->HeaderText->SetText(this->Messages["DOWNLOAD"].c_str());

  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
  vtkKWIcon *done = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDoneIcon();
  vtkKWIcon *wait = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWaitIcon();
  vtkKWIcon *error = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerErrorIcon();

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
        {
          this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, wait);
          this->Script("update idletasks");
          if (this->DownloadInstallExtension(this->ModulesMultiColumnList->GetCellText(row, 1),
                                             this->ModulesMultiColumnList->GetCellText(row, 6)))
            {
            this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, done);
            }
          else
            {
            this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, error);
            }
        }
    }

  this->HeaderText->SetText(this->Messages["FINISHED"].c_str());

  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOn();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Uninstall()
{
  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
  vtkKWIcon *done = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDoneIcon();
  vtkKWIcon *wait = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWaitIcon();
  vtkKWIcon *error = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerErrorIcon();

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
      {
      this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, wait);
      this->Script("update idletasks");
      if (this->UninstallExtension(this->ModulesMultiColumnList->GetCellText(row, 3)))
        {
        this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, done);
        }
      else
        {
        this->ModulesMultiColumnList->SetCellImageToIcon(row, 1, error);
        }
      }
    }

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

  std::string baseURL = wizard_dlg->GetSelectedRepositoryURL();

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

static bool
UnzipPackage(const std::string& zipfile, 
             const std::string& target,
             const std::string& tmpdir)
{
  std::string unzip;

  std::vector<std::string> candidates;
  candidates.push_back("c:/cygwin/bin/unzip.exe");
  candidates.push_back("/usr/bin/unzip");
  candidates.push_back("/bin/unzip");
  candidates.push_back("/usr/local/bin/unzip");

  std::vector<std::string>::iterator iter = candidates.begin();
  while (iter != candidates.end())
    {
      if (itksys::SystemTools::FileExists((*iter).c_str()))
      {
      unzip = (*iter);
      }
    iter++;
    }

  bool result = false;

  if (!unzip.empty())
    {
    std::stringstream cmd;
    cmd << unzip << " -o " << zipfile << " -d " << target;
    if (!system(cmd.str().c_str()))
      {
      result = true;
      }
    }

  return result;
}

//----------------------------------------------------------------------------
bool vtkSlicerModulesStep::DownloadInstallExtension(const std::string& ExtensionName,
                                                    const std::string& ExtensionBinaryURL)
{
  this->UninstallExtension(ExtensionName);

  bool result = false;

  vtkHTTPHandler *handler = vtkHTTPHandler::New();

  if (0 != handler->CanHandleURI(ExtensionBinaryURL.c_str()))
    {
    std::string::size_type pos = ExtensionBinaryURL.rfind("/");
    std::string zipname = ExtensionBinaryURL.substr(pos + 1);
      
    vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
      
    std::string tmpfile(std::string(app->GetTemporaryDirectory()) + std::string("/") + zipname);
      
    handler->StageFileRead(ExtensionBinaryURL.c_str(), tmpfile.c_str());

    std::string libdir(std::string(app->GetModuleCachePath()) + std::string("/") + ExtensionName);

    std::string tmpdir(std::string(app->GetTemporaryDirectory()) + std::string("/extension"));

    if (UnzipPackage(tmpfile, libdir, tmpdir))
      {
      result = true;
      }
    else
      {
      app->Script("eval $::_fixed_zip_code; vfs::zip::Mount %s /zipfile; file copy -force /zipfile/* %s; vfs::zip::Unmount %s /zipfile", tmpfile.c_str(), libdir.c_str(), tmpfile.c_str());
      }
    }

  handler->Delete();

  return result;
}

//----------------------------------------------------------------------------
bool vtkSlicerModulesStep::UninstallExtension(const std::string& ExtensionName)
{
  bool result = false;

  vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  if (app)
    {
    std::string libdir(std::string(app->GetModuleCachePath()) + std::string("/") + ExtensionName);
    
    if (itksys::SystemTools::FileExists(libdir.c_str()))
      {
      itksys::SystemTools::RemoveADirectory(libdir.c_str());
      }

    if (!itksys::SystemTools::FileExists(libdir.c_str()))
      {
      result = true;
      }

    }

  return result;
}
