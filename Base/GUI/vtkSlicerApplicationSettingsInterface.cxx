#include "vtkKWCheckButton.h"
#include "vtkKWDirectoryPresetSelector.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWWidget.h"
#include "vtkObjectFactory.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkSlicerFont.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerToolbarGUI.h"
#include "vtkSlicerViewControlGUI.h"

#include "vtkSlicerConfigure.h" // Slicer3_INSTALL_* 

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerApplicationSettingsInterface );
vtkCxxRevisionMacro(vtkSlicerApplicationSettingsInterface, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkSlicerApplicationSettingsInterface::vtkSlicerApplicationSettingsInterface()
{
  this->SlicerSettingsFrame = NULL;
  this->ConfirmDeleteCheckButton = NULL;
    
  this->ModuleSettingsFrame = NULL;
  this->ModulePathsPresetSelector = NULL;
  this->ModuleCachePathButton = NULL;
  this->HomeModuleEntry = NULL;
  this->TemporaryDirectoryButton = NULL;
  this->BrowserSelectButton = NULL;
  this->ZipSelectButton = NULL;
  this->UnzipSelectButton = NULL;
  this->RmSelectButton = NULL;
  this->LoadModulesCheckButton = NULL;
  this->LoadCommandLineModulesCheckButton = NULL;
  this->EnableDaemonCheckButton = NULL;

  this->FontSettingsFrame = NULL;
  this->FontSizeButtons = NULL;
  this->FontFamilyButtons = NULL;

  this->RemoteCacheSettingsFrame = NULL;
  this->EnableAsynchronousIOCheckButton = NULL;
  this->EnableForceRedownloadCheckButton = NULL;
//  this->EnableRemoteCacheOverwritingCheckButton = NULL;
  this->RemoteCacheDirectoryButton = NULL;
  this->RemoteCacheLimitSpinBox = NULL;
  this->RemoteCacheFreeBufferSizeSpinBox = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationSettingsInterface::~vtkSlicerApplicationSettingsInterface()
{
  if (this->SlicerSettingsFrame)
    {
    this->SlicerSettingsFrame->Delete();
    this->SlicerSettingsFrame = 0;
    }
  if ( this->FontSizeButtons )
    {
    this->FontSizeButtons->SetParent ( NULL );
    this->FontSizeButtons->Delete();
    this->FontSizeButtons = NULL;
    }
  if ( this->FontFamilyButtons )
    {
    this->FontFamilyButtons->SetParent ( NULL );
    this->FontFamilyButtons->Delete();
    this->FontFamilyButtons = NULL;
    }
  if ( this->FontSettingsFrame )
    {
    this->FontSettingsFrame->SetParent ( NULL );
    this->FontSettingsFrame->Delete();
    this->FontSettingsFrame = NULL;
    }
  if (this->ConfirmDeleteCheckButton)
    {
    this->ConfirmDeleteCheckButton->Delete();
    this->ConfirmDeleteCheckButton = NULL;
    }
  
  if (this->ModuleSettingsFrame)
    {
    this->ModuleSettingsFrame->Delete();
    this->ModuleSettingsFrame = 0;
    }

  if (this->ModulePathsPresetSelector)
    {
    this->ModulePathsPresetSelector->Delete();
    this->ModulePathsPresetSelector = 0;
    }

  if (this->ModuleCachePathButton)
    {
    this->ModuleCachePathButton->Delete();
    this->ModuleCachePathButton = 0;
    }
  
  if (this->HomeModuleEntry)
    {
    this->HomeModuleEntry->Delete();
    this->HomeModuleEntry = 0;
    }

  if ( this->BrowserSelectButton )
    {
    this->BrowserSelectButton->Delete();
    this->BrowserSelectButton = 0;
    }

  if ( this->ZipSelectButton )
    {
    this->ZipSelectButton->Delete();
    this->ZipSelectButton = 0;
    }

  if ( this->UnzipSelectButton )
    {
    this->UnzipSelectButton->Delete();
    this->UnzipSelectButton = 0;
    }

  if ( this->RmSelectButton )
    {
    this->RmSelectButton->Delete();
    this->RmSelectButton = 0;
    }
  
  if (this->TemporaryDirectoryButton)
    {
    this->TemporaryDirectoryButton->Delete();
    this->TemporaryDirectoryButton = 0;
    }

  if (this->LoadModulesCheckButton)
    {
    this->LoadModulesCheckButton->Delete();
    this->LoadModulesCheckButton = NULL;
    }

  if (this->LoadCommandLineModulesCheckButton)
    {
    this->LoadCommandLineModulesCheckButton->Delete();
    this->LoadCommandLineModulesCheckButton = NULL;
    }

  if (this->EnableDaemonCheckButton)
    {
    this->EnableDaemonCheckButton->Delete();
    this->EnableDaemonCheckButton = NULL;
    }

  if (this->RemoteCacheSettingsFrame)
    {
    this->RemoteCacheSettingsFrame->Delete();
    this->RemoteCacheSettingsFrame = NULL;
    }

  if (this->EnableAsynchronousIOCheckButton)
    {
    this->EnableAsynchronousIOCheckButton->Delete();
    this->EnableAsynchronousIOCheckButton = NULL;
    }

  if (this->EnableForceRedownloadCheckButton)
    {
    this->EnableForceRedownloadCheckButton->Delete();
    this->EnableForceRedownloadCheckButton = NULL;
    }

//  if (this->EnableRemoteCacheOverwritingCheckButton)
//    {
//    this->EnableRemoteCacheOverwritingCheckButton->Delete();
//    this->EnableRemoteCacheOverwritingCheckButton = NULL;
//    }

  if (this->RemoteCacheDirectoryButton)
    {
    this->RemoteCacheDirectoryButton->Delete();
    this->RemoteCacheDirectoryButton = NULL;
    }

  if (this->RemoteCacheLimitSpinBox)
    {
    this->RemoteCacheLimitSpinBox->Delete();
    this->RemoteCacheLimitSpinBox = NULL;
    }

  if (this->RemoteCacheFreeBufferSizeSpinBox)
    {
    this->RemoteCacheFreeBufferSizeSpinBox->Delete();
    this->RemoteCacheFreeBufferSizeSpinBox = NULL;
    }
    
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::Create()
{
  if (this->IsCreated())
    {
    vtkErrorMacro("The panel is already created.");
    return;
    }

  // Create the superclass instance (and set the application)

  this->Superclass::Create();

  ostrstream tk_cmd;
  vtkKWWidget *page;
  vtkKWFrame *frame;

  int label_width = 25;

  // --------------------------------------------------------------
  // Add a "Preferences" page

  this->AddPage(this->GetName());
  page = this->GetPageWidget(this->GetName());

  // --------------------------------------------------------------
  // Slicer Interface settings : main frame
  if (!this->SlicerSettingsFrame)
    {
    this->SlicerSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->SlicerSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->SlicerSettingsFrame->Create();
  this->SlicerSettingsFrame->SetLabelText("Slicer Settings");

  tk_cmd << "pack " << this->SlicerSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->SlicerSettingsFrame->GetFrame();
  
  // --------------------------------------------------------------
  // Slicer interface settings : Confirm on delete ?

  if (!this->ConfirmDeleteCheckButton)
    {
    this->ConfirmDeleteCheckButton = vtkKWCheckButton::New();
    }
  this->ConfirmDeleteCheckButton->SetParent(frame);
  this->ConfirmDeleteCheckButton->Create();
  this->ConfirmDeleteCheckButton->SetText("Confirm delete");
  this->ConfirmDeleteCheckButton->SetCommand(this, "ConfirmDeleteCallback");
  this->ConfirmDeleteCheckButton->SetBalloonHelpString(
    "A confirmation dialog will be presented to the user on deleting nodes.");

  tk_cmd << "pack " << this->ConfirmDeleteCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Slicer interface settings : Load Daemon?

  if (!this->EnableDaemonCheckButton)
    {
    this->EnableDaemonCheckButton = vtkKWCheckButton::New();
    }
  this->EnableDaemonCheckButton->SetParent(frame);
  this->EnableDaemonCheckButton->Create();
  this->EnableDaemonCheckButton->SetText("Enable Slicer Daemon");
  this->EnableDaemonCheckButton->SetCommand(this, "EnableDaemonCallback");
  this->EnableDaemonCheckButton->SetBalloonHelpString(
    "The Slicer Daemon will be enabled at startup.\nThis feature allows external programs to connect to a network port opened by Slicer.\nA dialog box will appear when the first connection is made giving you the option to allow connections or not.");

  tk_cmd << "pack " << this->EnableDaemonCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Slicer interface settings : Browser Select
  
  if (!this->BrowserSelectButton)
    {
    this->BrowserSelectButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->BrowserSelectButton->SetParent(frame);
  this->BrowserSelectButton->Create();
  this->BrowserSelectButton->SetLabelText("Set Firefox browser:");
  this->BrowserSelectButton->SetLabelWidth(label_width);
  this->BrowserSelectButton->GetWidget()->TrimPathFromFileNameOff();
  this->BrowserSelectButton->GetWidget()
    ->SetCommand(this, "BrowserSelectCallback");
  this->BrowserSelectButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOff();
  this->BrowserSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->BrowserSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Select Firefox web browser");
  this->BrowserSelectButton->SetBalloonHelpString(
    "Select the firefox browser for Slicer modules that use the web.");

  tk_cmd << "pack " << this->BrowserSelectButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Slicer interface settings : Zip Select
  if (!this->ZipSelectButton)
    {
    this->ZipSelectButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->ZipSelectButton->SetParent(frame);
  this->ZipSelectButton->Create();
  this->ZipSelectButton->SetLabelText("Set zip executable:");
  this->ZipSelectButton->SetLabelWidth(label_width);
  this->ZipSelectButton->GetWidget()->TrimPathFromFileNameOff();
  this->ZipSelectButton->GetWidget()
    ->SetCommand(this, "ZipSelectCallback");
  this->ZipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOff();
  this->ZipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->ZipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Set zip executable");
  this->ZipSelectButton->SetBalloonHelpString(
    "Select the zip executable for making archive files.");

  tk_cmd << "pack " << this->ZipSelectButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Slicer interface settings : Unzip Select
  if (!this->UnzipSelectButton)
    {
    this->UnzipSelectButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->UnzipSelectButton->SetParent(frame);
  this->UnzipSelectButton->Create();
  this->UnzipSelectButton->SetLabelText("Set unzip executable:");
  this->UnzipSelectButton->SetLabelWidth(label_width);
  this->UnzipSelectButton->GetWidget()->TrimPathFromFileNameOff();
  this->UnzipSelectButton->GetWidget()
    ->SetCommand(this, "UnzipSelectCallback");
  this->UnzipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOff();
  this->UnzipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->UnzipSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Set unzip executable");
  this->UnzipSelectButton->SetBalloonHelpString(
    "Select the unzip executable for expanding archive files.");

  tk_cmd << "pack " << this->UnzipSelectButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Slicer interface settings : Rm Select
  if (!this->RmSelectButton)
    {
    this->RmSelectButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->RmSelectButton->SetParent(frame);
  this->RmSelectButton->Create();
  this->RmSelectButton->SetLabelText("Set file remove executable:");
  this->RmSelectButton->SetLabelWidth(label_width);
  this->RmSelectButton->GetWidget()->TrimPathFromFileNameOff();
  this->RmSelectButton->GetWidget()
    ->SetCommand(this, "RmSelectCallback");
  this->RmSelectButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOff();
  this->RmSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->RmSelectButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Set rm executable");
  this->RmSelectButton->SetBalloonHelpString(
    "Select the executable for removing files (needs to take -rf argument).");

  tk_cmd << "pack " << this->RmSelectButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;
  
  
  // --------------------------------------------------------------
  // Slicer interface settings : Font settings frame
  if ( !this->FontSettingsFrame )
    {
    this->FontSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->FontSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->FontSettingsFrame->Create();
  this->FontSettingsFrame->SetLabelText("Font Settings");

  tk_cmd << "pack " << this->FontSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " << " -in "
         << page->GetWidgetName() << endl;
  frame = this->FontSettingsFrame->GetFrame();

  // --------------------------------------------------------------
  // Slicer interface settings : Font size?
  
  vtkKWFrameWithScrollbar *scrollframe = vtkKWFrameWithScrollbar::New();
  scrollframe->SetParent ( frame );
  scrollframe->Create();
  scrollframe->VerticalScrollbarVisibilityOn();
  scrollframe->HorizontalScrollbarVisibilityOn();
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 scrollframe->GetWidgetName());  

  vtkKWLabel *fontSizeLabel = vtkKWLabel::New();
  fontSizeLabel->SetParent ( scrollframe->GetFrame());
  fontSizeLabel->Create();
  fontSizeLabel->SetText ("Font size:");

  if ( !this->FontSizeButtons)
    {
    this->FontSizeButtons = vtkKWRadioButtonSet::New();
    }
  this->FontSizeButtons->SetParent (scrollframe->GetFrame());
  this->FontSizeButtons->Create();
  vtkKWRadioButton *button;
  button = this->FontSizeButtons->AddWidget ( 0 );
  button->SetText  ( "Use small font" );
  button->SetValue ( "small" );
  button->SetCommand ( this, "SetFontSizeCallback");
    
  button = this->FontSizeButtons->AddWidget ( 1 );
  button->SetText ("Use medium font" );
  button->SetValue ( "medium" );
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  button = this->FontSizeButtons->AddWidget ( 2 );
  button->SetText ( "Use large font");
  button->SetValue ( "large" );
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  button = this->FontSizeButtons->AddWidget (3 ); 
  button->SetText ( "Use largest font");
  button->SetValue ( "largest");
  button->SetCommand ( this, "SetFontSizeCallback");
  button->SetVariableName ( this->FontSizeButtons->GetWidget(0)->GetVariableName());

  // --------------------------------------------------------------
  // Slicer interface settings : Font family?
  
  vtkKWLabel *fontFamilyLabel = vtkKWLabel::New();
  fontFamilyLabel->SetParent ( scrollframe->GetFrame());
  fontFamilyLabel->Create();
  fontFamilyLabel->SetText ("Font family:");

  if ( !this->FontFamilyButtons)
    {
    this->FontFamilyButtons = vtkKWRadioButtonSet::New();
    }
  this->FontFamilyButtons->SetParent (scrollframe->GetFrame());
  this->FontFamilyButtons->Create();
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkSlicerTheme *theme = app->GetSlicerTheme();
  int numfonts = theme->GetSlicerFonts()->GetNumberOfFontFamilies();
  const char *font;
  for ( int i = 0; i < numfonts; i++ )
    {
    font = theme->GetSlicerFonts()->GetFontFamily(i);
    button = this->FontFamilyButtons->AddWidget ( i );
    button->SetText  (font );
    button->SetValue ( font );
    button->SetVariableName (this->FontFamilyButtons->GetWidget(0)->GetVariableName() );
    button->SetCommand( this, "SetFontFamilyCallback" );
    }
  
  vtkKWLabel *restartLabel = vtkKWLabel::New();
  restartLabel->SetParent ( scrollframe->GetFrame());
  restartLabel->Create();
  restartLabel->SetText ("(for best results, restart Slicer)");


  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 fontFamilyLabel->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -expand n",
                 this->FontFamilyButtons->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",
                 fontSizeLabel->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",                 
                 this->FontSizeButtons->GetWidgetName());
  this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2 -expand n",                 
                 restartLabel->GetWidgetName() );

  fontSizeLabel->Delete();
  fontFamilyLabel->Delete();
  restartLabel->Delete();
  scrollframe->Delete();
  
  // --------------------------------------------------------------
  // Module Interface settings : main frame

  if (!this->ModuleSettingsFrame)
    {
    this->ModuleSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->ModuleSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->ModuleSettingsFrame->Create();
  this->ModuleSettingsFrame->SetLabelText("Module Settings");
    
  tk_cmd << "pack " << this->ModuleSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->ModuleSettingsFrame->GetFrame();


  

  // --------------------------------------------------------------
  // Module settings : Load modules on startup ? (i.e. loadable modules)

  if (!this->LoadModulesCheckButton)
    {
    this->LoadModulesCheckButton = vtkKWCheckButton::New();
    }
  this->LoadModulesCheckButton->SetParent(frame);
  this->LoadModulesCheckButton->Create();
  this->LoadModulesCheckButton->SetText(
    "Load Modules");
  this->LoadModulesCheckButton->SetCommand(
    this, "LoadModulesCallback");
  this->LoadModulesCheckButton->SetBalloonHelpString(
    "Control if modules should be loaded at startup.");

  tk_cmd << "pack " << this->LoadModulesCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Module settings : Load commandline modules on startup ? (CLI plugins)

  if (!this->LoadCommandLineModulesCheckButton)
    {
    this->LoadCommandLineModulesCheckButton = vtkKWCheckButton::New();
    }
  this->LoadCommandLineModulesCheckButton->SetParent(frame);
  this->LoadCommandLineModulesCheckButton->Create();
  this->LoadCommandLineModulesCheckButton->SetText(
    "Load Command-Line Plugins");
  this->LoadCommandLineModulesCheckButton->SetCommand(
    this, "LoadCommandLineModulesCallback");
  this->LoadCommandLineModulesCheckButton->SetBalloonHelpString(
    "Control if command-line plugins (CLI) should be loaded at startup.");

  tk_cmd << "pack " << this->LoadCommandLineModulesCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Module settings : Home Module

  if ( !this->HomeModuleEntry )
    {
    this->HomeModuleEntry = vtkKWEntryWithLabel::New ( );
    }
  this->HomeModuleEntry->SetParent ( frame );
  this->HomeModuleEntry->Create ( );  
  this->HomeModuleEntry->SetLabelText( "Home Module:" );
  this->HomeModuleEntry->SetLabelWidth(label_width);
  this->HomeModuleEntry->GetWidget()->SetCommand ( 
    this, "HomeModuleCallback" );
  this->HomeModuleEntry->SetBalloonHelpString ( 
    "Name of the module displayed at startup and when the 'Home' icon is clicked. Alternatively, you may set this entry using the 'Edit -> Set Home' menu entry." );

  tk_cmd << "pack " << this->HomeModuleEntry->GetWidgetName()
         << "  -side top -anchor w -expand no -fill x -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Module settings : Module CachePath

  if (!this->ModuleCachePathButton)
    {
    this->ModuleCachePathButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->ModuleCachePathButton->SetParent(frame);
  this->ModuleCachePathButton->Create();
  this->ModuleCachePathButton->SetLabelText("Module Cache Path:");
  this->ModuleCachePathButton->SetLabelWidth(label_width);
  this->ModuleCachePathButton->GetWidget()->TrimPathFromFileNameOff();
  this->ModuleCachePathButton->GetWidget()->SetCommand(
    this, "ModuleCachePathCallback");
  this->ModuleCachePathButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->ModuleCachePathButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->ModuleCachePathButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Select a directory for the module cache");
  this->ModuleCachePathButton->SetBalloonHelpString(
    "Cache directory for modules. Leave it empty for default location.");

  tk_cmd << "pack " << this->ModuleCachePathButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Module settings : TemporaryDirectory

  if (!this->TemporaryDirectoryButton)
    {
    this->TemporaryDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->TemporaryDirectoryButton->SetParent(frame);
  this->TemporaryDirectoryButton->Create();
  this->TemporaryDirectoryButton->SetLabelText("Temporary Directory:");
  this->TemporaryDirectoryButton->SetLabelWidth(label_width);
  this->TemporaryDirectoryButton->GetWidget()->TrimPathFromFileNameOff();
  this->TemporaryDirectoryButton->GetWidget()
    ->SetCommand(this, "TemporaryDirectoryCallback");
  this->TemporaryDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->TemporaryDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->TemporaryDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Select a directory for temporary files");
  this->TemporaryDirectoryButton->SetBalloonHelpString(
    "Temporary directory for intermediate files.");

  tk_cmd << "pack " << this->TemporaryDirectoryButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Module settings : User Module Paths

  if (!this->ModulePathsPresetSelector)
    {
    this->ModulePathsPresetSelector = vtkKWDirectoryPresetSelector::New();
    }

  this->ModulePathsPresetSelector->SetParent(frame);
  this->ModulePathsPresetSelector->Create();
  this->ModulePathsPresetSelector->SetPresetAddCommand(
    this, "ModulePathsAddCallback");
  this->ModulePathsPresetSelector->SetPresetHasChangedCommand(
    this, "ModulePathsHasChangedCallback");
  this->ModulePathsPresetSelector->SetPresetRemovedCommand(
    this, "ModulePathsRemovedCallback");

  this->ModulePathsPresetSelector->SetBorderWidth(2);
  this->ModulePathsPresetSelector->SetReliefToGroove();
  this->ModulePathsPresetSelector->SetPadX(2);
  this->ModulePathsPresetSelector->SetPadY(2);
  this->ModulePathsPresetSelector->SetHelpLabelVisibility(1);
  
  this->ModulePathsPresetSelector->SetListHeight(6);
  this->ModulePathsPresetSelector->UniqueDirectoriesOn();

  std::string help_str;
  help_str = help_str + "Click on the \"Add a preset\" button to add one or more new user module paths. A typical Module path ends with " + Slicer3_INSTALL_MODULES_LIB_DIR + ", whereas a Plugins/CLP path ends with " + Slicer3_INSTALL_PLUGINS_LIB_DIR + ".";
  this->ModulePathsPresetSelector->SetHelpLabelText(help_str.c_str());

  tk_cmd << "pack " << this->ModulePathsPresetSelector->GetWidgetName()
         << "  -side top -anchor w -expand no -fill x -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Remote Data Handling settings : main frame

  if (!this->RemoteCacheSettingsFrame)
    {
    this->RemoteCacheSettingsFrame = vtkKWFrameWithLabel::New();
    }
  this->RemoteCacheSettingsFrame->SetParent(this->GetPagesParentWidget());
  this->RemoteCacheSettingsFrame->Create();
  this->RemoteCacheSettingsFrame->SetLabelText("Remote Data Handling Settings");
    
  tk_cmd << "pack " << this->RemoteCacheSettingsFrame->GetWidgetName()
         << " -side top -anchor nw -fill x -padx 2 -pady 2 " 
         << " -in " << page->GetWidgetName() << endl;
  
  frame = this->RemoteCacheSettingsFrame->GetFrame();

  // --------------------------------------------------------------
  // Remote settings : Enable asynchronous IO ?

  if (!this->EnableAsynchronousIOCheckButton)
    {
    this->EnableAsynchronousIOCheckButton = vtkKWCheckButton::New();
    }
  this->EnableAsynchronousIOCheckButton->SetParent(frame);
  this->EnableAsynchronousIOCheckButton->Create();
  this->EnableAsynchronousIOCheckButton->SetText(
    "Enable Asynchronous I/O");
  this->EnableAsynchronousIOCheckButton->SetCommand(
    this, "EnableAsynchronousIOCallback");
  this->EnableAsynchronousIOCheckButton->SetBalloonHelpString(
    "Control if using asychronous IO, if false use blocking IO, if true, downloads and loading are forked off into a separate thread.");

  tk_cmd << "pack " << this->EnableAsynchronousIOCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Remote settings : Enable force redownload?

  if (!this->EnableForceRedownloadCheckButton)
    {
    this->EnableForceRedownloadCheckButton = vtkKWCheckButton::New();
    }
  this->EnableForceRedownloadCheckButton->SetParent(frame);
  this->EnableForceRedownloadCheckButton->Create();
  this->EnableForceRedownloadCheckButton->SetText(
    "Force re-download");
  this->EnableForceRedownloadCheckButton->SetCommand(
    this, "EnableForceRedownloadCallback");
  this->EnableForceRedownloadCheckButton->SetBalloonHelpString(
    "Control if the file is in the cache, if should download it again. If false, check for and use the cached copy.");

  tk_cmd << "pack " << this->EnableForceRedownloadCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

  // --------------------------------------------------------------
  // Remote settings : remote cache over writing?

/*
  if (!this->EnableRemoteCacheOverwritingCheckButton)
    {
    this->EnableRemoteCacheOverwritingCheckButton = vtkKWCheckButton::New();
    }
  this->EnableRemoteCacheOverwritingCheckButton->SetParent(frame);
  this->EnableRemoteCacheOverwritingCheckButton->Create();
  this->EnableRemoteCacheOverwritingCheckButton->SetText(
    "Overwrite cache files");
  this->EnableRemoteCacheOverwritingCheckButton->SetCommand(
    this, "EnableRemoteCacheOverwritingCallback");
  this->EnableRemoteCacheOverwritingCheckButton->SetBalloonHelpString(
    "Control if should overwrite copies in the cache when downloading a remote file. If false, rename the old copy..");

  tk_cmd << "pack " << this->EnableRemoteCacheOverwritingCheckButton->GetWidgetName()
         << "  -side top -anchor w -expand no -fill none" << endl;

*/
  // --------------------------------------------------------------
  // Module settings : Remote Cache Directory

  if (!this->RemoteCacheDirectoryButton)
    {
    this->RemoteCacheDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
    }

  this->RemoteCacheDirectoryButton->SetParent(frame);
  this->RemoteCacheDirectoryButton->Create();
  this->RemoteCacheDirectoryButton->SetLabelText("Cache Directory:");
  this->RemoteCacheDirectoryButton->SetLabelWidth(label_width);
  this->RemoteCacheDirectoryButton->GetWidget()->TrimPathFromFileNameOff();
  this->RemoteCacheDirectoryButton->GetWidget()
    ->SetCommand(this, "RemoteCacheDirectoryCallback");
  this->RemoteCacheDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->RemoteCacheDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->SaveDialogOff();
  this->RemoteCacheDirectoryButton->GetWidget()
    ->GetLoadSaveDialog()->SetTitle("Select a directory for cached files");
  this->RemoteCacheDirectoryButton->SetBalloonHelpString(
    "Remote Cache directory for downloded files.");

  tk_cmd << "pack " << this->RemoteCacheDirectoryButton->GetWidgetName()
         << "  -side top -anchor w -expand no -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Remote settings : Cache limit

  if (!this->RemoteCacheLimitSpinBox)
    {
    this->RemoteCacheLimitSpinBox = vtkKWSpinBoxWithLabel::New();
    }

  this->RemoteCacheLimitSpinBox->SetParent(frame);
  this->RemoteCacheLimitSpinBox->Create();
  this->RemoteCacheLimitSpinBox->SetLabelText("Cache Size Limit:");
  this->RemoteCacheLimitSpinBox->SetLabelWidth(label_width);
  this->RemoteCacheLimitSpinBox->GetWidget()->SetCommand(this, "RemoteCacheLimitCallback");
  this->RemoteCacheLimitSpinBox->GetWidget()->SetRestrictValueToInteger();
  this->RemoteCacheLimitSpinBox->GetWidget()->SetRange(0,1000);
  this->RemoteCacheLimitSpinBox->SetBalloonHelpString("Set the upper limit on the size of the cache directory (Mb).");

  tk_cmd << "pack " << this->RemoteCacheLimitSpinBox->GetWidgetName()
         << "  -side top -anchor w -expand no -fill x -padx 2 -pady 2" << endl;

  // --------------------------------------------------------------
  // Remote settings : Cache free buffer size

  if (!this->RemoteCacheFreeBufferSizeSpinBox)
    {
    this->RemoteCacheFreeBufferSizeSpinBox = vtkKWSpinBoxWithLabel::New();
    }

  this->RemoteCacheFreeBufferSizeSpinBox->SetParent(frame);
  this->RemoteCacheFreeBufferSizeSpinBox->Create();
  this->RemoteCacheFreeBufferSizeSpinBox->SetLabelText("Cache Free Buffer Size:");
  this->RemoteCacheFreeBufferSizeSpinBox->SetLabelWidth(label_width);
  this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()->SetCommand(this, "RemoteCacheFreeBufferSizeCallback");
  this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()->SetRestrictValueToInteger();
  this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()->SetRange(0,900);
  this->RemoteCacheFreeBufferSizeSpinBox->SetBalloonHelpString("Set the amount of space in the cache directory that should remain free (Mb).");

  tk_cmd << "pack " << this->RemoteCacheFreeBufferSizeSpinBox->GetWidgetName()
         << "  -side top -anchor w -expand no -fill x -padx 2 -pady 2" << endl;

  
  // --------------------------------------------------------------
  // Pack 

  tk_cmd << ends;
  this->Script(tk_cmd.str());
  tk_cmd.rdbuf()->freeze(0);

  // Update

  this->Update();
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::SetFontFamilyCallback (  )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    vtkSlicerTheme *theme = app->GetSlicerTheme();
    if ( theme )
      {
      const char *font= this->FontFamilyButtons->GetWidget(0)->GetVariableValue();
      theme->SetFontFamily ( font );
      app->SetApplicationFontFamily ( font );
      app->Script ( "font configure %s -family %s", theme->GetApplicationFont2(), font);
      app->Script ( "font configure %s -family %s", theme->GetApplicationFont1(), font );
      app->Script ( "font configure %s -family %s", theme->GetApplicationFont0(), font );

     vtkSlicerApplicationGUI* appGUI = app->GetApplicationGUI();
      if ( appGUI )
        {
        vtkSlicerToolbarGUI *tGUI = appGUI->GetApplicationToolbar();
        if ( tGUI )
          {
          app->GetApplicationGUI()->GetApplicationToolbar()->ReconfigureGUIFonts();
          }
        vtkSlicerViewControlGUI *vcGUI = appGUI->GetViewControlGUI ();
        if ( vcGUI )
          {
          app->GetApplicationGUI()->GetViewControlGUI()->ReconfigureGUIFonts();
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::SetFontSizeCallback ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {

    vtkSlicerTheme *theme = app->GetSlicerTheme();
    if ( theme )
      {
      vtkSlicerFont *font = theme->GetSlicerFonts ( );
      if ( font )
        {
        const char *v = this->FontSizeButtons->GetWidget(0)->GetVariableValue();
        if ( font->IsValidFontSize ( v ) )
          {
          int f0 = font->GetFontSize0 ( v );
          int f1 = font->GetFontSize1 ( v );
          int f2 = font->GetFontSize2 ( v );
          app->SetApplicationFontSize (v);
          app->Script ( "font configure %s -size %d", theme->GetApplicationFont2(), f2 );
          app->Script ( "font configure %s -size %d", theme->GetApplicationFont1(), f1 );
          app->Script ( "font configure %s -size %d", theme->GetApplicationFont0(), f0 );

          vtkSlicerApplicationGUI* appGUI = app->GetApplicationGUI();
          if ( appGUI )
            {
            vtkSlicerToolbarGUI *tGUI = appGUI->GetApplicationToolbar();
            if ( tGUI )
              {
              app->GetApplicationGUI()->GetApplicationToolbar()->ReconfigureGUIFonts();
              }
            vtkSlicerViewControlGUI *vcGUI = appGUI->GetViewControlGUI ();
            if ( vcGUI )
              {
              app->GetApplicationGUI()->GetViewControlGUI()->ReconfigureGUIFonts();
              }
            }
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ConfirmDeleteCallback(int state)
{  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    app->SetConfirmDelete(state ? "1" : "0");       
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::LoadModulesCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    app->SetLoadModules(state ? 1 : 0);       
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::LoadCommandLineModulesCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    app->SetLoadCommandLineModules(state ? 1 : 0);       
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::EnableDaemonCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {

    app->SetEnableDaemon(state ? 1 : 0);       
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::HomeModuleCallback(char *name)
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app && name )
    {
    app->SetHomeModule ( name );
    }
}

//----------------------------------------------------------------------------
int vtkSlicerApplicationSettingsInterface::ModulePathsAddCallback()
{
  int id = this->ModulePathsPresetSelector->AddDirectoryCallback();
  this->ModulePathsRemovedCallback();
  return id;
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ModulePathsHasChangedCallback(int)
{
  this->ModulePathsRemovedCallback();
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ModulePathsRemovedCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    // Store the setting in the application object
    char *str;

    str = NULL;
    this->ModulePathsPresetSelector->GetPresetDirectoriesToDelimitedString(
      &str, '|');
    app->SetPotentialModulePaths(str);
    delete [] str;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ModuleCachePathCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetModuleCachePath(this->ModuleCachePathButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::BrowserSelectCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetWebBrowser(this->BrowserSelectButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::ZipSelectCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetZip(this->ZipSelectButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::UnzipSelectCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetUnzip(this->UnzipSelectButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::RmSelectCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetRm(this->RmSelectButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::TemporaryDirectoryCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetTemporaryDirectory(this->TemporaryDirectoryButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::EnableAsynchronousIOCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    app->SetEnableAsynchronousIO(state ? 1 : 0);       
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::EnableForceRedownloadCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    app->SetEnableForceRedownload(state ? 1 : 0);       
    }
}

//----------------------------------------------------------------------------
/*
void vtkSlicerApplicationSettingsInterface::EnableRemoteCacheOverwritingCallback(int state)
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app)
    {
    app->SetEnableRemoteCacheOverwriting(state ? 1 : 0);       
    }
}
*/
//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::RemoteCacheDirectoryCallback()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetRemoteCacheDirectory(this->RemoteCacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->GetFileName());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::RemoteCacheLimitCallback(int size)
{
  if (size < 0)
    {
    vtkErrorMacro("Please enter a cache size greater than zero");
    return;
    }
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Store the setting in the application object
    app->SetRemoteCacheLimit(size);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::RemoteCacheFreeBufferSizeCallback(int size)
{
  if (size < 0)
    {
    vtkErrorMacro("Please enter a free buffer size greater than zero");
    return;
    }
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // make sure it's not larger than the cache size
    if (size > app->GetRemoteCacheLimit())
      {
      vtkErrorMacro("Please enter a free buffer size that is less than the cache limit " <<  app->GetRemoteCacheLimit());
      return;
      }
    // Store the setting in the application object
    app->SetRemoteCacheFreeBufferSize(size);
    }
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::UpdateRemoteIOSettings()
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if ( app != NULL )
    {
    if (this->EnableAsynchronousIOCheckButton)
      {
      if ( this->EnableAsynchronousIOCheckButton->GetSelectedState() != app->GetEnableAsynchronousIO() )
        {
        this->EnableAsynchronousIOCheckButton->SetSelectedState( app->GetEnableAsynchronousIO() ? 1 : 0);
        }
      }

    if (this->EnableForceRedownloadCheckButton)
      {
      if ( this->EnableForceRedownloadCheckButton->GetSelectedState() != app->GetEnableForceRedownload() )
        {
        this->EnableForceRedownloadCheckButton->SetSelectedState(app->GetEnableForceRedownload() ? 1 : 0);
        }
      }
/*
  if (this->EnableRemoteCacheOverwritingCheckButton)
  {
  if ( this->EnableRemoteCacheOverwritingCheckButton->GetSelectedState() != app->GetEnableRemoteCacheOverwriting() )
  {
  this->EnableRemoteCacheOverwritingCheckButton->SetSelectedState(app->GetEnableRemoteCacheOverwriting() ? 1 : 0);
  }
  }
*/
    if (this->RemoteCacheDirectoryButton)
      {
      if ( strcmp ( this->RemoteCacheDirectoryButton->GetWidget()->GetText(), app->GetRemoteCacheDirectory() ) )
        {
        this->RemoteCacheDirectoryButton->GetWidget()->SetText(app->GetRemoteCacheDirectory());
        this->RemoteCacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetLastPath(app->GetRemoteCacheDirectory());
        }
      }
    if (this->RemoteCacheLimitSpinBox)
      {
      if ( this->RemoteCacheLimitSpinBox->GetWidget()->GetValue() != app->GetRemoteCacheLimit() )
        {
        this->RemoteCacheLimitSpinBox->GetWidget()->SetValue(app->GetRemoteCacheLimit());
        }
      }
    if (this->RemoteCacheFreeBufferSizeSpinBox)
      {
      if ( this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()->GetValue() != app->GetRemoteCacheFreeBufferSize() )
        {
        this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()->SetValue(app->GetRemoteCacheFreeBufferSize());
        }
      }
    }  
}



//----------------------------------------------------------------------------
void vtkSlicerApplicationSettingsInterface::Update()
{
  vtkSlicerApplication *app
    = vtkSlicerApplication::SafeDownCast(this->GetApplication());

  if (app)
    {
    // Pull values from the application object and put them in the
    // settings interface widgets
    if (this->ConfirmDeleteCheckButton)
      {
      this->ConfirmDeleteCheckButton->SetSelectedState(
        (strncmp(app->GetConfirmDelete(), "1", 1) == 0) ? 1 : 0);
      }
    if (this->EnableDaemonCheckButton)
      {
      this->EnableDaemonCheckButton->SetSelectedState(
        app->GetEnableDaemon() ? 1 : 0);
      }
    if (this->LoadModulesCheckButton)
      {
      this->LoadModulesCheckButton->SetSelectedState(
        app->GetLoadModules() ? 1 : 0);
      }
    if (this->LoadCommandLineModulesCheckButton)
      {
      this->LoadCommandLineModulesCheckButton->SetSelectedState(
        app->GetLoadCommandLineModules() ? 1 : 0);
      }
    if (this->HomeModuleEntry)
      {
      this->HomeModuleEntry->GetWidget()->SetValue(app->GetHomeModule());
      }
    if (this->ModulePathsPresetSelector)
      {
      this->ModulePathsPresetSelector->AddPresetDirectoriesFromDelimitedString(
        app->GetPotentialModulePaths(), '|');
      }
    if (this->ModuleCachePathButton)
      {
      this->ModuleCachePathButton->GetWidget()
        ->SetText(app->GetModuleCachePath());
      this->ModuleCachePathButton->GetWidget()
        ->GetLoadSaveDialog()->SetLastPath(app->GetModuleCachePath());
      }
    if ( this->FontSizeButtons )
      {
      if ( !(strcmp(app->GetApplicationFontSize(), "small" )))
        {
        this->FontSizeButtons->GetWidget(0)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "medium")))
        {
        this->FontSizeButtons->GetWidget(1)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "large")))
        {
        this->FontSizeButtons->GetWidget(2)->SetSelectedState(1);
        }
      else if ( !(strcmp(app->GetApplicationFontSize(), "largest")))
        {
        this->FontSizeButtons->GetWidget(3)->SetSelectedState(1);
        }      
      }
    if ( this->FontFamilyButtons )
      {
      if ( !(strcmp (app->GetApplicationFontFamily(), "Arial" )))
        {
        this->FontFamilyButtons->GetWidget(0)->SetSelectedState ( 1 );
        }
      if ( !(strcmp (app->GetApplicationFontFamily(), "Helvetica" )))
        {
        this->FontFamilyButtons->GetWidget(1)->SetSelectedState ( 1 );
        }
      if ( !(strcmp (app->GetApplicationFontFamily(), "Verdana" )))
        {
        this->FontFamilyButtons->GetWidget(2)->SetSelectedState ( 1 );
        }
      }
    if (this->TemporaryDirectoryButton)
      {
      this->TemporaryDirectoryButton->GetWidget()
        ->SetText(app->GetTemporaryDirectory());
      this->TemporaryDirectoryButton->GetWidget()
        ->GetLoadSaveDialog()->SetLastPath(app->GetTemporaryDirectory());
      }
    if (this->BrowserSelectButton)
      {
      this->BrowserSelectButton->GetWidget()->SetText(app->GetWebBrowser());
      }
    if (this->ZipSelectButton)
      {
      this->ZipSelectButton->GetWidget()->SetText(app->GetZip());
      }
    if (this->UnzipSelectButton)
      {
      this->UnzipSelectButton->GetWidget()->SetText(app->GetUnzip());
      }
    if (this->RmSelectButton)
      {
      this->RmSelectButton->GetWidget()->SetText(app->GetRm());
      }

    if (this->EnableAsynchronousIOCheckButton)
      {
      this->EnableAsynchronousIOCheckButton->SetSelectedState(
        app->GetEnableAsynchronousIO() ? 1 : 0);
      }
    if (this->EnableForceRedownloadCheckButton)
      {
      this->EnableForceRedownloadCheckButton->SetSelectedState(
        app->GetEnableForceRedownload() ? 1 : 0);
      }
/*
    if (this->EnableRemoteCacheOverwritingCheckButton)
      {
      this->EnableRemoteCacheOverwritingCheckButton->SetSelectedState(
        app->GetEnableRemoteCacheOverwriting() ? 1 : 0);
      }
*/
    if (this->RemoteCacheDirectoryButton)
      {
      this->RemoteCacheDirectoryButton->GetWidget()
        ->SetText(app->GetRemoteCacheDirectory());
      this->RemoteCacheDirectoryButton->GetWidget()
        ->GetLoadSaveDialog()->SetLastPath(app->GetRemoteCacheDirectory());
      }
    if (this->RemoteCacheLimitSpinBox)
      {
      this->RemoteCacheLimitSpinBox->GetWidget()
        ->SetValue(app->GetRemoteCacheLimit());
      }
    if (this->RemoteCacheFreeBufferSizeSpinBox)
      {
      this->RemoteCacheFreeBufferSizeSpinBox->GetWidget()
        ->SetValue(app->GetRemoteCacheFreeBufferSize());
      }
    }  
}
