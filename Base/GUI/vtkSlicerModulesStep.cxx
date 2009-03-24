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
#include "vtkKWMessageDialog.h"

#include "vtkSlicerModulesConfigurationStep.h"

#include "vtkHTTPHandler.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesConfigurationStep.h"
#include "vtkSlicerModulesWizardDialog.h"

#include <itksys/SystemTools.hxx>

#include <vtksys/ios/sstream>

//----------------------------------------------------------------------------
static std::string ltrim(const std::string& str)
{
  std::string s(str);
  if (s.size())
    {
    std::string::size_type pos = s.find_first_not_of(" \n\t");
    if (pos != std::string::npos)
      {
      s = s.substr(pos);
      }
    else
      {
      s = "";
      }
    }
  return s;
}

//----------------------------------------------------------------------------
static std::string rtrim(const std::string& str)
{
  std::string s(str);
  if (s.size())
    {
    std::string::size_type pos = s.find_last_not_of(" \n\t");
    if (pos != std::string::npos)
      {
      s = s.substr(0, pos + 1);
      }
    else
      {
      s = "";
      }
    }
  return s;
}

//----------------------------------------------------------------------------
static std::string trim(const std::string& str)
{
  return rtrim(ltrim(str));
}

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerModulesStep );
vtkCxxRevisionMacro(vtkSlicerModulesStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkSlicerModulesStep::vtkSlicerModulesStep()
{
  this->SetName("Extension Management Wizard");
  this->Frame1 = NULL;
  this->Frame2 = NULL;
  this->Frame3 = NULL;
  this->Frame4 = NULL;
  this->HeaderText = NULL;
  this->SelectAllButton = NULL;
  this->SelectNoneButton = NULL;
  this->ModulesMultiColumnList = NULL;
  this->DownloadButton = NULL;
  this->UninstallButton = NULL;
  this->StopButton = NULL;
  this->WizardDialog = NULL;

  this->Messages["READY"] = "Select extensions, then click uninstall to remove them from\nyour version of 3D Slicer, or click download to retrieve them.";
  this->Messages["DOWNLOAD"] = "Download in progress... Clicking the cancel button will stop\nthe process after the current extension operation is finished.";
  this->Messages["FINISHED"] = "Continue selecting extensions for download or removal,\nor click finish to complete the operation.";
}

//----------------------------------------------------------------------------
vtkSlicerModulesStep::~vtkSlicerModulesStep()
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
  if (this->StopButton)
    {
    this->StopButton->Delete();
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

  this->Script("pack %s %s %s %s -side top -anchor w",
               this->Frame1->GetWidgetName(),
               this->Frame2->GetWidgetName(),
               this->Frame3->GetWidgetName(),
               this->Frame4->GetWidgetName());

  if (!this->HeaderText)
    {
    this->HeaderText = vtkKWLabel::New();
    }
  if (!this->HeaderText->IsCreated())
    {
    this->HeaderText->SetParent( this->Frame1 );
    this->HeaderText->Create();
    this->HeaderText->SetText(this->Messages["READY"].c_str());
    }

  if (!this->SelectAllButton)
    {
    this->SelectAllButton = vtkKWPushButton::New();
    }
  if (!this->SelectAllButton->IsCreated())
    {
    this->SelectAllButton->SetParent( this->Frame2 );
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
    this->SelectNoneButton->SetParent( this->Frame2 );
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

    this->ModulesMultiColumnList->SetParent( this->Frame3 );
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
    this->ModulesMultiColumnList->SetColumnFormatCommandToEmptyOutput(col_index);

    col_index = this->ModulesMultiColumnList->AddColumn("Name");
    col_index = this->ModulesMultiColumnList->AddColumn("Category");
    col_index = this->ModulesMultiColumnList->AddColumn("Description");
    col_index = this->ModulesMultiColumnList->AddColumn("HomePage");
    col_index = this->ModulesMultiColumnList->AddColumn("Binary URL");

    }

  if (!this->DownloadButton)
    {
    this->DownloadButton = vtkKWPushButton::New();
    }
  if (!this->DownloadButton->IsCreated())
    {
    this->DownloadButton->SetParent( this->Frame4 );
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
    this->UninstallButton->SetParent( this->Frame4 );
    this->UninstallButton->Create();
    this->UninstallButton->SetText("Uninstall");
    this->UninstallButton->SetCommand(this, "Uninstall");
    }

  this->Script("pack %s -side top -pady 2 -anchor center", 
               this->HeaderText->GetWidgetName());

  this->Script("pack %s %s -side left -anchor w -pady 2", 
               this->SelectAllButton->GetWidgetName(),
               this->SelectNoneButton->GetWidgetName());

  this->Script("pack %s -side left", 
               this->ModulesMultiColumnList->GetWidgetName());

  this->Script("pack %s %s -side left -anchor w -pady 2", 
               this->DownloadButton->GetWidgetName(),
               this->UninstallButton->GetWidgetName());

  this->Update();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::UpdateModulesFromRepository(vtkSlicerApplication *app)
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
    
  this->Modules.clear();

  this->Modules = this->ParseManifest(HTML);

  delete[] HTML;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Update()
{
  vtkSlicerApplication *app =
    dynamic_cast<vtkSlicerApplication*> (this->GetApplication());

  if (app)
    {
      vtkSlicerModulesWizardDialog *wizard_dlg = this->GetWizardDialog();
      
      vtkSlicerModulesConfigurationStep *conf_step = wizard_dlg->GetModulesConfigurationStep();

      if (vtkSlicerModulesConfigurationStep::ActionInstall == conf_step->GetSelectedAction())
        {
        this->UpdateModulesFromRepository(app);
        if (this->DownloadButton)
          {
          this->DownloadButton->EnabledOn();
          }
        if (this->UninstallButton)
          {
          this->UninstallButton->EnabledOff();
          }
        }
      else if (vtkSlicerModulesConfigurationStep::ActionUninstall == conf_step->GetSelectedAction())
        {
        if (this->DownloadButton)
          {
          this->DownloadButton->EnabledOff();
          }
        if (this->UninstallButton)
          {
          this->UninstallButton->EnabledOn();
          }
        }
      else if (vtkSlicerModulesConfigurationStep::ActionEither == conf_step->GetSelectedAction())
        {
        this->UpdateModulesFromRepository(app);
        if (this->DownloadButton)
          {
          this->DownloadButton->EnabledOn();
          }
        if (this->UninstallButton)
          {
          this->UninstallButton->EnabledOn();
          }
        }

    const char* cachedir = app->GetModuleCachePath();

    if (this->ModulesMultiColumnList)
      {
      this->ModulesMultiColumnList->DeleteAllRows();

      // Insert each extension entry discovered on the repository
      for (unsigned int i = 0; i < this->Modules.size(); i++)
        {
        ManifestEntry *extension = this->Modules[i];            
        this->InsertExtension(i, extension, cachedir);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::InsertExtension(int Index,
                                           ManifestEntry* Entry,
                                           const std::string& CacheDir)
{
  this->ModulesMultiColumnList->InsertCellText(Index, 2, Entry->Name.c_str());
  this->ModulesMultiColumnList->InsertCellText(Index, 3, Entry->Category.c_str());

  std::string summary = Entry->Description.substr(0, 5);
  summary += "...";

  this->ModulesMultiColumnList->InsertCellText(Index, 4, summary.c_str());

  this->ModulesMultiColumnList->InsertCellText(Index, 5, Entry->Homepage.c_str());
  this->ModulesMultiColumnList->InsertCellText(Index, 6, Entry->URL.c_str());
            
  this->ModulesMultiColumnList->SetCellWindowCommandToCheckButton(Index, 0);
      
  std::string extdir(CacheDir + std::string("/") + Entry->Name);

  if (itksys::SystemTools::FileExists(extdir.c_str()))
    {
    this->SetStatus(Index, vtkSlicerModulesStep::StatusFoundOnDisk);
    }
  else
    {
    this->SetStatus(Index, vtkSlicerModulesStep::StatusNotFoundOnDisk);
    }
}

//----------------------------------------------------------------------------
int vtkSlicerModulesStep::IsActionValid()
{
  return this->ActionTaken != vtkSlicerModulesStep::ActionIsEmpty ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectAll()
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (StatusFoundOnDisk != this->GetStatus(row))
      {
      this->ModulesMultiColumnList->SetCellText(row, 0, "1");
      }
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

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
        {
          this->SetStatus(row, vtkSlicerModulesStep::StatusWait);
          if (this->DownloadInstallExtension(this->ModulesMultiColumnList->GetCellText(row, 2),
                                             this->ModulesMultiColumnList->GetCellText(row, 6)))
            {
            this->SetStatus(row, vtkSlicerModulesStep::StatusSuccess);
            }
          else
            {
            this->SetStatus(row, vtkSlicerModulesStep::StatusError);
            }
        }
    }

  this->HeaderText->SetText(this->Messages["FINISHED"].c_str());

  this->ActionTaken = vtkSlicerModulesStep::ActionIsDownloadInstall;
  
  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOn();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Uninstall()
{
  vtkKWMessageDialog *dlg = vtkKWMessageDialog::New();

  dlg->SetApplication( this->GetApplication() );
  dlg->SetMasterWindow( this->GetWizardDialog() );
  dlg->SetText("You are about to uninstall the selected modules, OK?");
  dlg->SetStyleToOkCancel();

  dlg->Delete();

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (1 == this->ModulesMultiColumnList->GetCellTextAsInt(row, 0))
      {
      this->SetStatus(row, vtkSlicerModulesStep::StatusWait);
      if (this->UninstallExtension(this->ModulesMultiColumnList->GetCellText(row, 2)))
        {
        this->SetStatus(row, vtkSlicerModulesStep::StatusSuccess);
        }
      else
        {
        this->SetStatus(row, vtkSlicerModulesStep::StatusError);
        }
      }
    }


  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOff();
}

//----------------------------------------------------------------------------
int vtkSlicerModulesStep::GetStatus(int row_index)
{
  int result = StatusUnknown;

  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  if (row_index < nrows)
    {
    result = this->ModulesMultiColumnList->GetCellTextAsInt(row_index, 1);
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SetStatus(int row_index, int status)
{
  int nrows = this->ModulesMultiColumnList->GetNumberOfRows();
  if (row_index < nrows)
    {
    this->ModulesMultiColumnList->SetCellTextAsInt(row_index, 1, status);

    vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication()); 
    
    switch (status)
      {
      case vtkSlicerModulesStep::StatusSuccess: {
        vtkKWIcon *done = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDoneIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, done);
        } break;
      case vtkSlicerModulesStep::StatusWait: {
        vtkKWIcon *wait = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWaitIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, wait);
        } break;
      case vtkSlicerModulesStep::StatusCancelled: {
        vtkKWIcon *cancelled = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCancelledIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, cancelled);
        } break;
      case vtkSlicerModulesStep::StatusError: {
        vtkKWIcon *error = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerErrorIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, error);
        } break;
      case vtkSlicerModulesStep::StatusFoundOnDisk: {
        vtkKWIcon *foundondisk = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFoundOnDiskIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, foundondisk);
        } break;
      case vtkSlicerModulesStep::StatusNotFoundOnDisk: {
        vtkKWIcon *notfoundondisk = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerNotFoundOnDiskIcon();
        this->ModulesMultiColumnList->SetCellImageToIcon(row_index, 1, notfoundondisk);
        } break;
      case vtkSlicerModulesStep::StatusUnknown: default:
        // ??
        break;
      }
    }

  this->Script("update idletasks");
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

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());

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

  std::string first_key(".zip\">");
  std::string second_key(".s3ext\">");

  std::string::size_type zip = txt.find(first_key, 0);
  std::string::size_type atag = txt.find("</a>", zip);
  std::string::size_type dash = txt.find("-", zip);
  std::string::size_type ext = txt.find(second_key, dash);
  std::string::size_type atag2 = txt.find("</a>", ext);

  ManifestEntry* entry;

  // :NOTE: 20081003 tgl: Put in a sanity check of 10,000 to
  // prevent an infinite loop.  Get Out The Vote 2008!

  std::string s3ext;
  int count = 0;
  while (zip != std::string::npos && count < 10000)
    {
    entry = new ManifestEntry;

    if (std::string::npos != atag2)
      {
      entry->URL = baseURL;
      entry->URL += "/";
      entry->URL += txt.substr(zip + first_key.size(), atag - (zip + first_key.size()));
      entry->Name = txt.substr(zip + first_key.size(), dash - (zip + first_key.size()));
      
      s3ext = baseURL;
      s3ext += "/";
      s3ext += txt.substr(ext + second_key.size(), atag2 - (ext + second_key.size()));

      this->DownloadParseS3ext(s3ext, entry);

      zip = txt.find(first_key, zip + 1);
      dash = txt.find("-", zip);
      atag = txt.find("</a>", zip);
      ext = txt.find(second_key, dash);
      atag2 = txt.find("</a>", dash);

      result.push_back(entry);
      }
      
    count++;
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadParseS3ext(const std::string& s3ext,
                                              ManifestEntry* entry)
{
  vtkHTTPHandler *handler = vtkHTTPHandler::New();

  if (0 != handler->CanHandleURI(s3ext.c_str()))
    {
    std::string::size_type pos = s3ext.rfind("/");
    std::string s3extname = s3ext.substr(pos + 1);
      
    vtkSlicerApplication *app = dynamic_cast<vtkSlicerApplication*> (this->GetApplication());
      
    std::string tmpfile(std::string(app->GetTemporaryDirectory()) + std::string("/") + s3extname);
      
    handler->StageFileRead(s3ext.c_str(), tmpfile.c_str());

    std::ifstream ifs(tmpfile.c_str());
    std::string line;
    
    while (std::getline(ifs, line))
      {
        if (line.find("homepage") == 0)
          {
          entry->Homepage = trim(line.substr(9));
          }
        else if (line.find("category") == 0)
          {
          entry->Category = trim(line.substr(9));
          }
        else if (line.find("status") == 0)
          {
          entry->ExtensionStatus = trim(line.substr(7));
          }
        else if (line.find("description") == 0)
          {
          entry->Description = trim(line.substr(12));
          }
      }

    ifs.close();
    }
  
  handler->Delete();
  
}

//----------------------------------------------------------------------------
static bool UnzipPackage(const std::string& zipfile, 
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

    std::string cachedir = app->GetModuleCachePath();
    if (cachedir.empty())
      {
        cachedir = Slicer3_INSTALL_MODULES_LIB_DIR;
      }

    std::string libdir(cachedir + std::string("/") + ExtensionName);

    std::string tmpdir(std::string(app->GetTemporaryDirectory()) + std::string("/extension"));

    if (UnzipPackage(tmpfile, libdir, tmpdir))
      {
      result = true;
      std::string paths = app->GetModulePaths();

#if WIN32
      const char delim = ';';
#else
      const char delim = ':';
#endif
      if (!paths.empty())
        {
        paths += delim;
        }

      paths += libdir;
      app->SetModulePaths(paths.c_str());
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

    // :BUG: 20090108 tgl: Not guaranteed that the install of the
    // module will be under Slicer3_INSTALL_MODULES_LIB_DIR if
    // ModuelCachePath is empty.

    std::string cachedir = app->GetModuleCachePath();
    if (cachedir.empty())
      {
        cachedir = Slicer3_INSTALL_MODULES_LIB_DIR;
      }

    std::string libdir(cachedir + std::string("/") + ExtensionName);
    
    if (itksys::SystemTools::FileExists(libdir.c_str()))
      {
      itksys::SystemTools::RemoveADirectory(libdir.c_str());
      }

    if (!itksys::SystemTools::FileExists(libdir.c_str()))
      {
      result = true;

      std::string paths = app->GetModulePaths();

#if WIN32
      std::string delim = ";;";
#else
      std::string delim = "::";
#endif
      
      std::string::size_type pos = paths.find(libdir.c_str());
      if (std::string::npos != pos)
        {
        paths.erase(pos, libdir.size());
        }

      pos = paths.find(delim);
      
      if (std::string::npos != pos)
        {
        paths.erase(pos, 2);
        }    

      app->SetModulePaths(paths.c_str());

      }

    }

  return result;
}
