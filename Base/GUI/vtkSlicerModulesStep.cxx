#include "vtkSlicerModulesStep.h"

// vtkSlicer includes
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerModulesConfigurationStep.h"
#include "vtkSlicerModulesWizardDialog.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWFrame.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWText.h"
#include "vtkKWTextWithLabel.h"

#include "vtkSlicerModulesConfigurationStep.h"

// RemoteIO includes
#include "vtkHTTPHandler.h"

// VTK includes
#include "vtkObjectFactory.h"

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
vtkCxxRevisionMacro(vtkSlicerModulesStep, "$Revision$");

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
  this->HomePageColIndex = 0;
  this->DownloadButton = NULL;
  this->UninstallButton = NULL;
  this->StopButton = NULL;
  this->WizardDialog = NULL;

  this->Messages["READY"] = "Select extensions, then click uninstall to remove them from\nyour version of 3D Slicer, or click download to retrieve them.";
  this->Messages["DOWNLOAD"] = "Download in progress... Clicking the cancel button will stop\nthe process after the current extension operation is finished.";
  this->Messages["FINISHED"] = "Continue selecting extensions for download or removal,\nor click finish to complete the operation.";

  this->ActionTaken = vtkSlicerModulesStep::ActionIsEmpty;

  strcpy(this->InstallPath, "");
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

  this->ClearModules();
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

  vtkKWWizardWidget *wizard_widget = this->GetWizardDialog()->GetWizardWidget();
  
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

  this->Script("pack %s %s -side top -anchor w",
               this->Frame1->GetWidgetName(),
               this->Frame2->GetWidgetName());
  this->Script("pack %s -side top -anchor w -expand true -fill both",
               this->Frame3->GetWidgetName());
  this->Script("pack %s -side top -anchor w",
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
    this->SelectAllButton->SetBorderWidth( 0 );
    this->SelectAllButton->SetReliefToFlat();
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
    this->SelectNoneButton->SetBorderWidth( 0 );
    this->SelectNoneButton->SetReliefToFlat();
    this->SelectNoneButton->SetImageToIcon(app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDeselectAllIcon());
    this->SelectNoneButton->SetCommand(this, "SelectNone");
    }

  if (!this->ModulesMultiColumnList)
    {
    this->ModulesMultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
    }
  if (!this->ModulesMultiColumnList->IsCreated())
    {

    this->ModulesMultiColumnList->SetParent( this->Frame3 );
    this->ModulesMultiColumnList->Create();
    this->ModulesMultiColumnList->SetWidth(0);
    this->ModulesMultiColumnList->SetHeight(7);

    vtkKWMultiColumnList *the_list = this->ModulesMultiColumnList->GetWidget();

    the_list->SetSelectionTypeToCell();

    int col_index;

    col_index = the_list->AddColumn("Select");
    the_list->SetColumnEditWindowToCheckButton(col_index);
    the_list->SetColumnFormatCommandToEmptyOutput(col_index);
    the_list->SetColumnAlignmentToCenter(col_index);

    col_index = the_list->AddColumn("Status");
    the_list->SetColumnFormatCommandToEmptyOutput(col_index);
    the_list->SetColumnAlignmentToCenter(col_index);

    col_index = the_list->AddColumn("Name");
    col_index = the_list->AddColumn("Category");

    col_index = the_list->AddColumn("Description");
    the_list->SetColumnFormatCommandToEmptyOutput(col_index);
    the_list->SetColumnWidth(col_index, 12);

    this->HomePageColIndex = col_index = the_list->AddColumn("HomePage");
    the_list->SetColumnWidth(col_index, 0);
    the_list->SetColumnFormatCommandToEmptyOutput(col_index);
    the_list->SetSelectionCommand( this, "OpenHomePageInBrowserCallback" );
    the_list->SetColumnAlignmentToCenter(col_index);

    col_index = the_list->AddColumn("Binary URL");

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

  this->Script("pack %s %s -side left -anchor w -pady 2 -padx 2", 
               this->SelectAllButton->GetWidgetName(),
               this->SelectNoneButton->GetWidgetName());

  this->Script("pack %s -side left -anchor sw -expand true -fill both", 
               this->ModulesMultiColumnList->GetWidgetName());

  this->Script("pack %s %s -side left -anchor sw -pady 2", 
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

  std::vector<ManifestEntry*> modules = this->ParseManifest(HTML);
  this->Modules.insert(this->Modules.begin(), modules.begin(), modules.end());

  delete[] HTML;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::UpdateModulesFromDisk(vtkSlicerApplication *app)
{
  std::vector<ManifestEntry*> modules;

  std::string paths = app->GetModulePaths();

  std::cout << "UPDATE paths: " << paths << std::endl;

#if WIN32
  const char delim = ';';
  const char dir = '\\';
#else
  const char delim = ':';
  const char dir =  '/';
#endif

  ManifestEntry* entry;

  std::string::size_type npos = paths.find(delim);

    std::cout << "npos: " << npos << std::endl;

    std::string::size_type dpos = paths.rfind(dir, npos);

    std::cout << "dpos: " << dpos << std::endl;


  while (std::string::npos != npos)
    {
    entry = new ManifestEntry;

    entry->Name = paths.substr(dpos + 1, npos - dpos - 1);
    
    std::cout << "name: " << entry->Name << std::endl;

    this->AddEntry(modules, entry);

    npos = paths.find(delim, npos + 1);
    dpos = paths.rfind(dir, npos);


    std::cout << "npos: " << npos << std::endl;
    std::cout << "dpos: " << dpos << std::endl;
    }

  
  if (paths.length() > 0)
    {
    npos = paths.length() - 1;
    dpos = paths.rfind(dir, npos);

    std::cout << "npos: " << npos << std::endl;
    std::cout << "dpos: " << dpos << std::endl;

    entry = new ManifestEntry;

    entry->Name = paths.substr(dpos + 1, npos - dpos);
  
    std::cout << "name: " << entry->Name << std::endl;
  
    this->AddEntry(modules, entry);
    }


  this->Modules.insert(this->Modules.begin(), modules.begin(), modules.end());
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::Update()
{
  vtkSlicerApplication *app =
    vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    int action = this->ActionToBeTaken();

    if (vtkSlicerModulesConfigurationStep::ActionInstall == action)
      {
      this->ClearModules();
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
    else if (vtkSlicerModulesConfigurationStep::ActionUninstall == action)
      {
      this->ClearModules();
      this->UpdateModulesFromDisk(app);
      if (this->DownloadButton)
        {
        this->DownloadButton->EnabledOff();
        }
      if (this->UninstallButton)
        {
        this->UninstallButton->EnabledOn();
        }
      }
    else if (vtkSlicerModulesConfigurationStep::ActionEither == action)
      {
      this->ClearModules();
      this->UpdateModulesFromRepository(app);
      this->UpdateModulesFromDisk(app);
      if (this->DownloadButton)
        {
        this->DownloadButton->EnabledOn();
        }
      if (this->UninstallButton)
        {
        this->UninstallButton->EnabledOn();
        }
      }

    if (this->ModulesMultiColumnList)
      {
      this->ModulesMultiColumnList->GetWidget()->DeleteAllRows();

      // Insert each extension entry discovered on the repository
      for (unsigned int i = 0; i < this->Modules.size(); i++)
        {
        this->InsertExtension(i, this->Modules[i], this->GetInstallPath());
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::InsertExtension(int Index,
                                           ManifestEntry* Entry,
                                           const std::string& InstallDir)
{
  vtkKWMultiColumnList *the_list = this->ModulesMultiColumnList->GetWidget();

  the_list->InsertCellText(Index, 2, Entry->Name.c_str());
  the_list->InsertCellText(Index, 3, Entry->Category.c_str());

  the_list->SetCellText(Index, 4, Entry->Description.c_str());
  the_list->SetCellWindowCommand(Index, 4, this, "DescriptionCommand");



  the_list->SetCellText(Index, this->HomePageColIndex, Entry->Homepage.c_str());

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication()); 
  
  if (app)
    {
    vtkKWIcon *www = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWWWIcon();
    the_list->SetCellImageToIcon(Index, this->HomePageColIndex, www);
    }

  the_list->SetCellEditable(Index, this->HomePageColIndex, 0);



  the_list->InsertCellText(Index, 6, Entry->URL.c_str());
            
  the_list->SetCellWindowCommandToCheckButton(Index, 0);
      
  std::string extdir(InstallDir + std::string("/") + Entry->Name);

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
  int action = this->ActionToBeTaken();
  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (vtkSlicerModulesConfigurationStep::ActionInstall == action)
        {
        if (StatusFoundOnDisk != this->GetStatus(row) &&
            StatusSuccess != this->GetStatus(row))
          {
          this->ModulesMultiColumnList->GetWidget()->SetCellText(row, 0, "1");
          }
        }
      else if (vtkSlicerModulesConfigurationStep::ActionUninstall == action)
        {
        if (StatusFoundOnDisk == this->GetStatus(row) ||
            StatusSuccess == this->GetStatus(row))
          {
          this->ModulesMultiColumnList->GetWidget()->SetCellText(row, 0, "1");
          }
        }
      else
        {
        this->ModulesMultiColumnList->GetWidget()->SetCellText(row, 0, "1");
        }
    }

  this->ModulesMultiColumnList->GetWidget()->RefreshAllRowsWithWindowCommand(0);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SelectNone()
{
  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    this->ModulesMultiColumnList->GetWidget()->SetCellText(row, 0, "0");
    }

  this->ModulesMultiColumnList->GetWidget()->RefreshAllRowsWithWindowCommand(0);
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadInstall()
{
  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOff();

  this->HeaderText->SetText(this->Messages["DOWNLOAD"].c_str());

  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
      if (1 == this->ModulesMultiColumnList->GetWidget()->GetCellTextAsInt(row, 0))
        {
          this->SetStatus(row, vtkSlicerModulesStep::StatusWait);
          if (this->DownloadInstallExtension(this->ModulesMultiColumnList->GetWidget()->GetCellText(row, 2),
                                             this->ModulesMultiColumnList->GetWidget()->GetCellText(row, 6)))
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

  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (1 == this->ModulesMultiColumnList->GetWidget()->GetCellTextAsInt(row, 0))
      {
      this->SetStatus(row, vtkSlicerModulesStep::StatusWait);
      if (this->UninstallExtension(this->ModulesMultiColumnList->GetWidget()->GetCellText(row, 2)))
        {
        this->SetStatus(row, vtkSlicerModulesStep::StatusSuccess);
        }
      else
        {
        this->SetStatus(row, vtkSlicerModulesStep::StatusError);
        }
      }
    }

  this->ActionTaken = vtkSlicerModulesStep::ActionIsUninstall;

  this->GetWizardDialog()->GetWizardWidget()->CancelButtonVisibilityOff();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::OpenHomePageInBrowserCallback()
{
  vtkKWMultiColumnList *the_list = this->ModulesMultiColumnList->GetWidget();

  int num_rows = the_list->GetNumberOfRows();
  int col;
  
  for (int loop = 0; loop < num_rows; loop++)
    {
      col = the_list->IsCellSelected( loop, this->HomePageColIndex );
    if (col)
      {
      const char *uri = the_list->GetCellText( loop, this->HomePageColIndex );
      if (0 != uri)
        {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        if (app)
          {
          app->OpenLink(uri);
          } 
        }
      the_list->DeselectCell( loop, this->HomePageColIndex );
      break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DescriptionCommand(const char *vtkNotUsed(notused),
                                              int row_index,
                                              int col_index,
                                              const char *widget_name)
{
  vtkKWMultiColumnList *the_list = this->ModulesMultiColumnList->GetWidget();

  vtkKWLabel *child = 
    vtkKWLabel::SafeDownCast(the_list->GetCellWindowAsFrame(row_index, col_index));

 
  if (!child)
    {
    child = vtkKWLabel::New();
    child->SetWidgetName(widget_name);
    child->SetParent(the_list);
    child->Create();
    child->Delete();
    }

  child->SetBackgroundColor(the_list->GetCellCurrentBackgroundColor(row_index, col_index));

  std::string description = the_list->GetCellText(row_index, col_index);

  if (!description.empty())
    {
    std::string summary = description.substr(0, 12);
    summary += "...";

    //child->SetText(summary.c_str());
    child->SetText(description.c_str());
    child->SetBalloonHelpString(description.c_str());
    }
}

//----------------------------------------------------------------------------
int vtkSlicerModulesStep::GetStatus(int row_index)
{
  int result = StatusUnknown;

  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  if (row_index < nrows)
    {
    result = this->ModulesMultiColumnList->GetWidget()->GetCellTextAsInt(row_index, 1);
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::SetStatus(int row_index, int status)
{
  int nrows = this->ModulesMultiColumnList->GetWidget()->GetNumberOfRows();
  if (row_index < nrows)
    {
    this->ModulesMultiColumnList->GetWidget()->SetCellTextAsInt(row_index, 1, status);

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication()); 
    
    switch (status)
      {
      case vtkSlicerModulesStep::StatusSuccess: {
        vtkKWIcon *done = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDoneIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, done);
        } break;
      case vtkSlicerModulesStep::StatusWait: {
        vtkKWIcon *wait = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerWaitIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, wait);
        } break;
      case vtkSlicerModulesStep::StatusCancelled: {
        vtkKWIcon *cancelled = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCancelledIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, cancelled);
        } break;
      case vtkSlicerModulesStep::StatusError: {
        vtkKWIcon *error = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerErrorIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, error);
        } break;
      case vtkSlicerModulesStep::StatusFoundOnDisk: {
        vtkKWIcon *foundondisk = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFoundOnDiskIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, foundondisk);
        } break;
      case vtkSlicerModulesStep::StatusNotFoundOnDisk: {
        vtkKWIcon *notfoundondisk = app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerNotFoundOnDiskIcon();
        this->ModulesMultiColumnList->GetWidget()->SetCellImageToIcon(row_index, 1, notfoundondisk);
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
  vtkKWWizardWidget *wizard_widget = this->GetWizardDialog()->GetWizardWidget();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetWizardDialog()->GetWizardWidget()->GetWizardWorkflow();

  int valid = this->IsActionValid();
  if (valid != 0)
    {
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
    }
  else
    {
    int action = this->ActionToBeTaken();
    if (vtkSlicerModulesConfigurationStep::ActionInstall == action)
      {
      wizard_widget->SetErrorText("No action taken, choose \"Download & Install\".");
      }
    else if (vtkSlicerModulesConfigurationStep::ActionUninstall == action)
      {
      wizard_widget->SetErrorText("No action taken, choose \"Uninstall\".");
      }
    else
      {
      wizard_widget->SetErrorText("No action taken, choose \"Download & Install\" or \"Uninstall\".");
      }

    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    }

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

  std::string zip_key(".zip\">");
  std::string ext_key(".s3ext\">");
  std::string atag_key("</a>");
  std::string svn_key("-svn");
  std::string cvs_key("-cvs");

  std::string::size_type zip = txt.find(zip_key, 0);
  std::string::size_type atag = txt.find(atag_key, zip);
  std::string::size_type dash = txt.find(svn_key, zip);

  bool cvs = false;
  if (std::string::npos == dash || dash > atag)
    {
    cvs = true;
    dash = txt.find(cvs_key, zip);
    }

  std::string::size_type dash2;
  if (cvs)
    {
    dash2 = (dash + 3 + 10);
    }
  else
    {
    dash2 = txt.find("-", dash + 1);
    }

  std::string::size_type ext = txt.find(ext_key, dash2);
  std::string::size_type atag2 = txt.find(atag_key, ext);

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
      entry->URL += txt.substr(zip + zip_key.size(), atag - (zip + zip_key.size()));
      
      if (cvs)
        {
        entry->Name = txt.substr(zip + zip_key.size(), dash - (zip + zip_key.size()));
        }
      else
        {
        entry->Name = txt.substr(zip + zip_key.size(), dash - (zip + zip_key.size()));
        }

      if (cvs)
        {
        // :NOTE: 20090519 tgl: CVS controlled extensions use an ISO date.
        entry->Revision = txt.substr(dash + cvs_key.size(), 10);
        }
      else
        {
        entry->Revision = txt.substr(dash + svn_key.size(), dash2 - (dash + svn_key.size()));
        }

      s3ext = baseURL;
      s3ext += "/";
      s3ext += txt.substr(ext + ext_key.size(), atag2 - (ext + ext_key.size()));

      this->DownloadParseS3ext(s3ext, entry);

      zip = txt.find(zip_key, zip + 1);

      dash = txt.find(svn_key, zip);
      atag = txt.find(atag_key, zip);

      cvs = false;
      if (std::string::npos == dash || dash > atag)
        {
        cvs = true;
        dash = txt.find(cvs_key, zip);
        }

      dash2 = txt.find("-", dash + 1);
      ext = txt.find(ext_key, dash );
      atag2 = txt.find(atag_key, ext);

      this->AddEntry(result, entry);
      }
      
    count++;
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::AddEntry(std::vector<ManifestEntry*> &entries,
                                    ManifestEntry *entry)
{
  std::vector<ManifestEntry*>::iterator iter;
  for (iter = entries.begin(); iter != entries.end(); iter++)
    {
    if ((*iter)->Name == entry->Name)
      {
      break;
      }
    }

  if (iter == entries.end())
    {
    entries.push_back(entry);
    }
  else
    {
    if (entry->Revision > (*iter)->Revision)
      {
      delete (*iter);
      (*iter) = entry;
      }
    else
      {
      delete entry;
      entry = 0;
      }
    }
  
}// AddEntry

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::DownloadParseS3ext(const std::string& s3ext,
                                              ManifestEntry* entry)
{
  vtkHTTPHandler *handler = vtkHTTPHandler::New();
  handler->SetForbidReuse(1);

  if (0 != handler->CanHandleURI(s3ext.c_str()))
    {
    std::string::size_type pos = s3ext.rfind("/");
    std::string s3extname = s3ext.substr(pos + 1);
      
    vtkSlicerApplication *app =
      vtkSlicerApplication::SafeDownCast(this->GetApplication());
      
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
                         const std::string& vtkNotUsed(tmpdir))
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
    cmd << unzip << " -o \"" << zipfile << "\" -d " << target;
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
  handler->SetForbidReuse(1);

  if (0 != handler->CanHandleURI(ExtensionBinaryURL.c_str()))
    {
    std::string::size_type pos = ExtensionBinaryURL.rfind("/");
    std::string zipname = ExtensionBinaryURL.substr(pos + 1);
      
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
      
    std::string tmpfile(std::string(app->GetTemporaryDirectory()) + std::string("/") + zipname);
      
    handler->StageFileRead(ExtensionBinaryURL.c_str(), tmpfile.c_str());

    std::string installdir = this->GetInstallPath();

    std::string libdir(installdir + std::string("/") + ExtensionName);

    std::string searchdir(std::string(this->GetInstallPath(true)) + std::string("/") + ExtensionName);

    std::string tmpdir(std::string(app->GetTemporaryDirectory()) + std::string("/extension"));

    if (UnzipPackage(tmpfile, libdir, tmpdir))
      {
      result = true;
      }
    else
      {
      app->Script("package require vfs::zip");
      app->Script("if { ![file exists \"%s\"] } {file mkdir \"%s\"}", libdir.c_str(), libdir.c_str());
      app->Script("set ::ZIPFD [vfs::zip::Mount \"%s\" /zipfile]", tmpfile.c_str());
      app->Script("set ::ZIPFILES [glob /zipfile/*]");
      app->Script("foreach direntry $::ZIPFILES {file copy -force $direntry \"%s\"}", 
        libdir.c_str());
      app->Script("vfs::zip::Unmount $::ZIPFD /zipfile");
      result = true;
      }

    std::cout << "search " << searchdir << std::endl;
      app->AppendPotentialModulePath(searchdir.c_str(), true);
    }

  handler->Delete();

  return result;
}

//----------------------------------------------------------------------------
bool vtkSlicerModulesStep::UninstallExtension(const std::string& ExtensionName)
{
  bool result = false;

  vtkSlicerApplication *app =
    vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {

    // :BUG: 20090108 tgl: Not guaranteed that the install of the
    // module will be under this directory if the user has made edits
    // to the location over time.

    std::string installdir = this->GetInstallPath();

    std::string libdir(installdir + std::string("/") + ExtensionName);
    
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

//----------------------------------------------------------------------------
int vtkSlicerModulesStep::ActionToBeTaken()
{
  vtkSlicerModulesWizardDialog *wizard_dlg = this->GetWizardDialog();
      
  vtkSlicerModulesConfigurationStep *conf_step = wizard_dlg->GetModulesConfigurationStep();

  return conf_step->GetSelectedAction();
}

//----------------------------------------------------------------------------
void vtkSlicerModulesStep::ClearModules()
{
  std::vector<ManifestEntry*>::iterator iter = this->Modules.begin();
  while (iter != this->Modules.end())
    {
    delete (*iter);
    iter++;
    }
  this->Modules.clear();
}

//----------------------------------------------------------------------------
const char* vtkSlicerModulesStep::GetInstallPath(bool WithToken)
{
  vtkSlicerApplication *app =
    vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    std::string installdir = app->GetExtensionsInstallPath();
    installdir += "/";
    if (WithToken)
      {
      installdir += "@SVN@";
      }
    else
      {
      installdir += app->GetSvnRevision();
      }

    strcpy(this->InstallPath, installdir.c_str());
    }

  if (!WithToken)
    {
    if (!itksys::SystemTools::FileExists(this->InstallPath))
      {
      itksys::SystemTools::MakeDirectory(this->InstallPath);
      }
    }
  
  return this->InstallPath;
}
