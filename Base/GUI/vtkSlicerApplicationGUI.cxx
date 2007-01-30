/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include <sstream>
#include <string>
#include <vtksys/SystemTools.hxx> 
#include <itksys/SystemTools.hxx> 

#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkObjectFactory.h"
#include "vtkToolkits.h"

// things for temporary MainViewer display.
#include "vtkCubeSource.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"

#include "vtkKWApplication.h"
#include "vtkKWTclInteractor.h"
#include "vtkKWWidget.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWUserInterfaceManagerNotebook.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWToolbarSet.h"

#include "vtkSlicerWindow.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleNavigator.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerMRMLSaveDataWidget.h"
#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkSlicerSliceControllerWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");

// temporary crud for vtkDebugLeak hunting. Will remove
// these and other related #ifndefs-#endifs throughout.

//#define LOGODISPLAY_DEBUG
//#define TOOLBAR_DEBUG
//#define VIEWCONTROL_DEBUG
//#define SLICEVIEWER_DEBUG

//#define MENU_DEBUG
//#define SLICESCONTROL_DEBUG
//#define MODULECHOOSE_DEBUG

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
    //---  
    // widgets used in the Slice module
    //---

    //--- slicer main window
    this->MainSlicerWindow = vtkSlicerWindow::New ( );

    // Frames that comprise the Main Slicer GUI

    this->TopFrame = vtkKWFrame::New();
    this->LogoFrame = vtkKWFrame::New();
    this->SlicesControlFrame = vtkSlicerModuleCollapsibleFrame::New();
    this->ViewControlFrame = vtkSlicerModuleCollapsibleFrame::New();
    this->DropShadowFrame = vtkKWFrame::New();
    this->LightboxFrame = vtkKWFrame::New ( );

    // initialize in case any are not defined.
    this->ApplicationToolbar = NULL;
    this->ViewControlGUI = NULL;
    this->SlicesControlGUI = NULL;
    this->LogoDisplayGUI = NULL;
    
    //--- GUIs containing components packed inside the Frames
#ifndef TOOLBAR_DEBUG
    this->ApplicationToolbar = vtkSlicerToolbarGUI::New ( );
#endif
#ifndef VIEWCONTROL_DEBUG
    this->ViewControlGUI = vtkSlicerViewControlGUI::New ( );
#endif
#ifndef SLICESCONTROL_DEBUG
    this->SlicesControlGUI = vtkSlicerSlicesControlGUI::New ( );
#endif
#ifndef LOGODISPLAY_DEBUG    
    this->LogoDisplayGUI = vtkSlicerLogoDisplayGUI::New ( );
#endif
    
    //--- Main viewer, 3 main slice viewers and collection.
    this->ViewerWidget = NULL;
    this->FiducialListWidget = NULL;

    this->MainSliceGUI0 = NULL;
    this->MainSliceGUI1 = NULL;
    this->MainSliceGUI2 = NULL;
    this->SliceGUICollection = NULL;
    
    //--- Save the main slice logic in these.
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;


    //--- Save and load scene dialogs, widgets
    this->LoadSceneDialog = vtkKWLoadSaveDialog::New();
    this->SaveDataWidget = NULL;

    //--- unique tag used to mark all view notebook pages
    //--- so that they can be identified and deleted when 
    //--- viewer is reformatted.
    this->ViewerPageTag = 1999;
}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( )
{
    if ( this->SliceGUICollection )
      {
        this->SliceGUICollection->RemoveAllItems();
        this->SetSliceGUICollection ( NULL );
      }

    // TODO: fix this method to remove all actors, etc.
    // from Main Viewer, slice viewers before deleting.
    this->DestroyMain3DViewer ( );
    this->DestroyMainSliceViewers ( );

    // Delete frames
    if ( this->TopFrame )
      {
      this->TopFrame->SetParent ( NULL );
      this->TopFrame->Delete ( );
      this->TopFrame = NULL;
      }
    if ( this->LogoFrame )
      {
      this->LogoFrame->SetParent ( NULL );
      this->LogoFrame->Delete ();
      this->LogoFrame = NULL;
      }
    if ( this->DropShadowFrame )
      {
      this->DropShadowFrame->SetParent ( NULL );
      this->DropShadowFrame->Delete ( );
      this->DropShadowFrame = NULL;
      }
    if ( this->SlicesControlFrame )
      {
      this->SlicesControlFrame->SetParent ( NULL );
      this->SlicesControlFrame->Delete ( );
      this->SlicesControlFrame = NULL;
      }
    if ( this->ViewControlFrame )
      {
      this->ViewControlFrame->SetParent ( NULL );
      this->ViewControlFrame->Delete ( );
      this->ViewControlFrame = NULL;
      }

    if ( this->LoadSceneDialog )
      {
      this->LoadSceneDialog->SetParent ( NULL );
      this->LoadSceneDialog->Delete();
      this->LoadSceneDialog = NULL;
      }
    if ( this->MainSlicerWindow )
      {
      if ( this->GetApplication() )
        {
        this->GetApplication()->RemoveWindow ( this->MainSlicerWindow );
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        if ( app->GetMainLayout() )
          {
          app->GetMainLayout()->SetMainSlicerWindow(NULL);
          }
        }
      this->MainSlicerWindow->SetParent ( NULL );
      this->MainSlicerWindow->Delete ( );
      this->MainSlicerWindow = NULL;
      }
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;

    if (this->SaveDataWidget)
      {
      this->SaveDataWidget->SetParent ( NULL );
      this->SaveDataWidget->Delete();
      }

    this->SetApplication(NULL);
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI:: DeleteComponentGUIs()
{
#ifndef VIEWCONTROL_DEBUG
    if ( this->ViewControlGUI )
      {
      this->ViewControlGUI->RemoveSliceGUIObservers();
      this->ViewControlGUI->SetAndObserveMRMLScene ( NULL );
      this->ViewControlGUI->SetApplicationGUI ( NULL);
      this->ViewControlGUI->SetApplication ( NULL );
      this->ViewControlGUI->Delete ( );
      this->ViewControlGUI = NULL;
      }
#endif
#ifndef LOGODISPLAY_DEBUG
    if ( this->LogoDisplayGUI )
      {
      this->LogoDisplayGUI->Delete ( );
      this->LogoDisplayGUI = NULL;
      }
#endif
#ifndef SLICESCONTROL_DEBUG
    if ( this->SlicesControlGUI )
      {
      this->SlicesControlGUI->UnbuildGUI ( );
      this->SlicesControlGUI->Delete ( );
      this->SlicesControlGUI = NULL;
      }
#endif
#ifndef TOOLBAR_DEBUG
    if ( this->ApplicationToolbar )
      {
      this->ApplicationToolbar->Delete ( );
      this->ApplicationToolbar = NULL;
      }
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerApplicationGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "MainSlicerWindow: " << this->GetMainSlicerWindow ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessLoadSceneCommand()
{
    this->LoadSceneDialog->RetrieveLastPathFromRegistry("OpenPath");

    this->LoadSceneDialog->Invoke();
    // If a file has been selected for loading...
    char *fileName = this->LoadSceneDialog->GetFileName();
    if ( fileName ) 
      {
        std::string fl(fileName);
        if (this->GetMRMLScene() && fl.find(".mrml") != std::string::npos ) 
          {
          this->GetMRMLScene()->SetURL(fileName);
          this->GetMRMLScene()->Connect();
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
          }
        else if (this->GetMRMLScene() && fl.find(".xml") != std::string::npos ) 
          {
          this->Script ( "ImportSlicer2Scene %s", fileName);
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
          }

        if (  this->GetMRMLScene()->GetErrorCode() != 0 ) 
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent (  this->MainSlicerWindow );
          dialog->SetStyleToMessage();
          std::string msg = this->GetMRMLScene()->GetErrorMessage();
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          }
      }
    return;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessImportSceneCommand()
{
    this->LoadSceneDialog->RetrieveLastPathFromRegistry(
      "OpenPath");

    this->LoadSceneDialog->Invoke();
    // If a file has been selected for loading...
    char *fileName = this->LoadSceneDialog->GetFileName();
    if ( fileName ) 
      {
        std::string fl(fileName);
        if (this->GetMRMLScene() && fl.find(".mrml") != std::string::npos ) 
          {
          this->GetMRMLScene()->SetURL(fileName);
          this->GetMRMLScene()->Import();
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
          }
        else if (this->GetMRMLScene() && fl.find(".xml") != std::string::npos ) 
          {
          this->Script ( "ImportSlicer2Scene %s", fileName);
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
          }

        if (  this->GetMRMLScene()->GetErrorCode() != 0 ) 
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent (  this->MainSlicerWindow );
          dialog->SetStyleToMessage();
          std::string msg = this->GetMRMLScene()->GetErrorMessage();
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          }
      }
    return;
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessCloseSceneCommand()
{
  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetParent ( this->MainSlicerWindow );
  dialog->SetStyleToOkCancel();
  dialog->SetText("Are you sure you want to close scene?");
  dialog->Create ( );
  if (dialog->Invoke())
  {
    if (this->GetMRMLScene()) 
      {
      //this->MRMLScene->Clear();
      this->MRMLScene->Clear(false);
      }
  }
  dialog->Delete();
}  

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessSaveSceneAsCommand()
{
    this->SaveDataWidget = vtkSlicerMRMLSaveDataWidget::New();
    this->SaveDataWidget->SetParent ( this->MainSlicerWindow);
    this->SaveDataWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
    this->SaveDataWidget->AddObserver ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->SaveDataWidget->Create();  

    this->SaveDataWidget->RemoveObservers ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->SaveDataWidget->SetParent(NULL);
    this->SaveDataWidget->Delete();
    this->SaveDataWidget=NULL;
    return;
}    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( )
{

  this->GetMainSlicerWindow()->GetFileMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  // keep track of changes to Home Module set from Application Settings interface;
  // try trapping the View menu events, and just updating the home module from registry...
  this->LoadSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
#ifndef TOOLBAR_DEBUG
  this->GetApplicationToolbar()->AddGUIObservers ( );
#endif
#ifndef VIEWCONTROL_DEBUG
  this->GetViewControlGUI()->AddGUIObservers ( );
#endif
#ifndef SLICESCONTROL_DEBUG
  this->GetSlicesControlGUI ( )->AddGUIObservers ( );
#endif
#ifndef LOGODISPLAY_DEBUG
  this->GetLogoDisplayGUI ( )->AddGUIObservers ( );
#endif
  if (this->MainSliceGUI0)
    {
    this->MainSliceGUI0->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI0->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MainSliceGUI1)
    {
    this->MainSliceGUI1->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI1->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MainSliceGUI2)
    {
    this->MainSliceGUI2->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI2->GetSliceController()->AddObserver(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( )
{
    this->GetMainSlicerWindow()->GetFileMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LoadSceneDialog->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *) this->GUICallbackCommand );

#ifndef TOOLBAR_DEBUG
    this->GetApplicationToolbar()->RemoveGUIObservers ( );
#endif
#ifndef VIEWCONTROL_DEBUG
    this->GetViewControlGUI ( )->RemoveGUIObservers ( );
#endif
#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI ( )->RemoveGUIObservers ( );
#endif
#ifndef LOGODISPLAY_DEBUG
    this->GetLogoDisplayGUI ( )->RemoveGUIObservers ( );
#endif    
    this->RemoveMainSliceViewerObservers ( );
  if (this->MainSliceGUI0)
    {
    this->MainSliceGUI0->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI0->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MainSliceGUI1)
    {
    this->MainSliceGUI1->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI1->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MainSliceGUI2)
    {
    this->MainSliceGUI2->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ExpandEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    this->MainSliceGUI2->GetSliceController()->RemoveObservers(
      vtkSlicerSliceControllerWidget::ShrinkEvent, 
      (vtkCommand *)this->GUICallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessGUIEvents ( vtkObject *caller,
                                                 unsigned long event, void *callData )
{
  
  // This code is just a placeholder until the logic is set up to use properly:
  // For now, the GUI controls the GUI instead of going thru the logic...
  // TODO:
  // Actually, these events want to set "activeModule" in the logic;
  // using this->Logic->SetActiveModule ( ) which is currently commented out.
  // Observers on that logic should raise and lower the appropriate page.
  // So for now, the GUI is controlling the GUI instead of going thru the logic.
  //---
  if (event == vtkSlicerModuleGUI::ModuleSelectedEvent) 
    {
    this->SelectModule((const char*)callData);
    return;
    }
  vtkKWLoadSaveDialog *filebrowse = vtkKWLoadSaveDialog::SafeDownCast(caller);
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
  vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      
  vtkSlicerMRMLSaveDataWidget *saveDataWidget = vtkSlicerMRMLSaveDataWidget::SafeDownCast(caller);
  if (saveDataWidget == this->SaveDataWidget && event == vtkSlicerMRMLSaveDataWidget::DataSavedEvent)
    {
    }

  vtkSlicerSliceControllerWidget *controller_caller = 
    vtkSlicerSliceControllerWidget::SafeDownCast(caller);
  if (caller)
    {
    if (event == vtkSlicerSliceControllerWidget::ExpandEvent) 
      {
      if (this->MainSliceGUI0)
        {
        this->MainSliceGUI0->GetSliceController()->Expand();
        }
      if (this->MainSliceGUI1)
        {
        this->MainSliceGUI1->GetSliceController()->Expand();
        }
      if (this->MainSliceGUI2)
        {
        this->MainSliceGUI2->GetSliceController()->Expand();
        }
      }
    else if (event == vtkSlicerSliceControllerWidget::ShrinkEvent) 
      {
      if (this->MainSliceGUI0)
        {
        this->MainSliceGUI0->GetSliceController()->Shrink();
        }
      if (this->MainSliceGUI1)
        {
        this->MainSliceGUI1->GetSliceController()->Shrink();
        }
      if (this->MainSliceGUI2)
        {
        this->MainSliceGUI2->GetSliceController()->Shrink();
        }
      }
    }

  // check to see if any caller belongs to SliceControllerGUI
  // and if so, call this->SliceControllerGUI->ProcessGUIEvents ( )
  // with same params. hand it off.

  // check to see if any caller belongs to ViewControlGUI
  // and if so, call this->ViewControlGUI->ProcessGUIEvents ( )
  // with same params. hand it off.


}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessLogicEvents ( vtkObject *caller,
                                                   unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Enter ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Exit ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SelectModule ( const char *moduleName )
{
#ifndef TOOLBAR_DEBUG
  this->GetApplicationToolbar()->GetModuleChooseGUI()->SelectModule(moduleName);
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUI ( )
{
    int i;
    
    // Set up the conventional window: 3Dviewer, slice widgets, UI panel for now.
    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerGUILayout *layout = app->GetMainLayout ( );
        
        // Set a pointer to the MainSlicerWindow in vtkSlicerGUILayout, and
        // Set default sizes for all main frames (UIpanel and viewers) in GUI
        layout->SetMainSlicerWindow ( this->MainSlicerWindow );
        layout->InitializeLayoutDimensions ( );

        if ( this->MainSlicerWindow != NULL ) {

            // set up Slicer's main window
            this->MainSlicerWindow->SecondaryPanelVisibilityOn ( );
            this->MainSlicerWindow->MainPanelVisibilityOn ( );
            app->AddWindow ( this->MainSlicerWindow );

            // Create the console before the window
            // - this will make the console independent of the main window
            //   so it can be raised/lowered independently
            this->MainSlicerWindow->GetTclInteractor()->SetApplication(app);
            this->MainSlicerWindow->GetTclInteractor()->Create();

            // TODO: it would be nice to make this a menu option on the tkcon itself,
            // but for now just up the font size

            this->MainSlicerWindow->Create ( );        
            
//            app->GetTclInteractor()->SetFont("Courier 12");

            // configure initial GUI layout
            layout->InitializeMainSlicerWindowSize ( );
            layout->ConfigureMainSlicerWindowPanels ( );

            // Build main GUI frames and components that fill them
            this->BuildGUIFrames ( );

            // Build Logo GUI panel
#ifndef LOGODISPLAY_DEBUG
            vtkSlicerLogoDisplayGUI *logos = this->GetLogoDisplayGUI ( );
            logos->SetApplicationGUI ( this );
            logos->SetApplication ( app );
            logos->BuildGUI ( this->LogoFrame );
#endif            
            // Build toolbar
#ifndef TOOLBAR_DEBUG
            vtkSlicerToolbarGUI *appTB = this->GetApplicationToolbar ( );
            appTB->SetApplicationGUI ( this );
            appTB->SetApplication ( app );
            appTB->BuildGUI ( );
#endif

            // Build SlicesControl panel
#ifndef SLICESCONTROL_DEBUG            
            vtkSlicerSlicesControlGUI *scGUI = this->GetSlicesControlGUI ( );
            scGUI->SetApplicationGUI ( this );
            scGUI->SetApplication ( app );
            scGUI->SetAndObserveMRMLScene ( this->MRMLScene );
            scGUI->BuildGUI ( this->SlicesControlFrame->GetFrame() );

#endif

            // Build 3DView Control panel
#ifndef VIEWCONTROL_DEBUG
            vtkSlicerViewControlGUI *vcGUI = this->GetViewControlGUI ( );
            vcGUI->SetApplicationGUI ( this );
            vcGUI->SetApplication ( app );
            vcGUI->SetAndObserveMRMLScene ( this->MRMLScene );
            vcGUI->BuildGUI ( this->ViewControlFrame->GetFrame() );
#endif

            this->MainSlicerWindow->GetMainNotebook()->SetUseFrameWithScrollbars ( 1 );
            this->MainSlicerWindow->GetMainNotebook()->SetEnablePageTabContextMenu ( 0 );
            
            // Build 3DViewer and Slice Viewers

#ifndef SLICEVIEWER_DEBUG
            this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutDefaultView );
#endif

            // after SliceGUIs are created, the ViewControlGUI
            // needs to observe them to feed its magnifier
            // Zoom Widget.
#ifndef VIEWCONTROL_DEBUG
            vcGUI->UpdateFromMRML();
            vcGUI->UpdateSliceGUIInteractorStyles();
#endif

#ifndef MENU_DEBUG
            // Construct menu bar and set up global key bindings
            // 
            // File Menu
            //
            this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                      this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                      "Load Scene...", this, "ProcessLoadSceneCommand");

            this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                      this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                      "Import Scene...", this, "ProcessImportSceneCommand");

            this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                               "Save", this, "ProcessSaveSceneAsCommand");

            this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                               "Close Scene", this, "ProcessCloseSceneCommand");

            this->GetMainSlicerWindow()->GetFileMenu()->InsertSeparator (
                this->GetMainSlicerWindow()->GetFileMenuInsertPosition());

            // don't need the 'close command'
            this->GetMainSlicerWindow()->GetFileMenu()->DeleteItem (
                                                                    this->GetMainSlicerWindow()->GetFileMenu()->GetIndexOfItem(
                                                                                                                               this->GetMainSlicerWindow()->GetFileCloseMenuLabel()));
            //
            // Edit Menu
            //
            i = this->MainSlicerWindow->GetEditMenu()->AddCommand ("Set Home", NULL, "$::slicer3::ApplicationGUI SetCurrentModuleToHome");
            this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+H");
            this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);
            i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "Undo", NULL, "$::slicer3::MRMLScene Undo" );
            this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Z");
            this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);
            i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "Redo", NULL, "$::slicer3::MRMLScene Redo" );
            this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Y");
            this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

            //
            // View Menu
            //
/*
            this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                      this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                      "Single Slice", NULL, "$::slicer3::ApplicationGUI UnpackMainSliceViewerFrames ; $::slicer3::ApplicationGUI PackFirstSliceViewerFrame ");
            this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                      this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                      "Three Slices", NULL, "$::slicer3::ApplicationGUI UnpackMainSliceViewerFrames ; $::slicer3::ApplicationGUI PackFirstSliceViewerFrame ");
*/
            //
            // Help Menu
            //
            this->GetMainSlicerWindow()->GetHelpMenu()->InsertCommand (
                                                                       this->GetMainSlicerWindow()->GetHelpMenuInsertPosition(),
                                                                       "Browse tutorials (not yet available)", NULL, "$::slicer3::ApplicationGUI OpenTutorialsLink");

            //
            // Feedback Menu
            //
            this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: report a bug (www)", NULL, "$::slicer3::ApplicationGUI OpenBugLink");
            this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: report usability issue (www)", NULL, "$::slicer3::ApplicationGUI OpenUsabilityLink");
            this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: make a feature request (www)", NULL, "$::slicer3::ApplicationGUI OpenFeatureLink");
            this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Community: Slicer Visual Blog (www)", NULL, "$::slicer3::ApplicationGUI PostToVisualBlog");            

            
            this->LoadSceneDialog->SetParent ( this->MainSlicerWindow );
            this->LoadSceneDialog->Create ( );
            this->LoadSceneDialog->SetFileTypes("{ {MRML Scene} {*.mrml} } { {Slicer2 Scene} {*.xml} }");
            this->LoadSceneDialog->RetrieveLastPathFromRegistry("OpenPath");

#endif
        }

    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetCurrentModuleToHome (  )
{
#ifndef TOOLBAR_DEBUG
  if ( this->GetApplication() != NULL )
      {
      if ( this->GetApplicationToolbar()->GetModuleChooseGUI() )
        {
        if ( this->GetApplicationToolbar()->GetModuleChooseGUI()->GetModuleNavigator() )
          {
          vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
          const char *name = this->GetApplicationToolbar()->GetModuleChooseGUI()->GetModuleNavigator()->GetCurrentModuleName ( );
          //--- save to registry.
          app->SetHomeModule ( name );
          this->GetMainSlicerWindow()->GetApplicationSettingsInterface()->Update();
          }
        }
      }
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenTutorialsLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    //app->OpenLink ("" );
    }
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenBugLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.na-mic.org/Bug/index.php" );
    }
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenUsabilityLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.na-mic.org/Bug/index.php" );
    }

}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenFeatureLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.na-mic.org/Bug/index.php" );
    }
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PostToVisualBlog ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.na-mic.org/Wiki/index.php/Slicer3:VisualBlog" );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DestroyMainSliceViewers ( )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      //
      // Destroy 3 main slice viewers
      //
      if ( this->MainSliceGUI0 )
        {
          this->MainSliceGUI0->SetAndObserveMRMLScene (NULL );
          this->MainSliceGUI0->SetAndObserveModuleLogic ( NULL );
          this->MainSliceGUI0->RemoveGUIObservers ( );
          this->MainSliceGUI0->SetApplicationLogic ( NULL );
          if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
            {
              this->MainSliceGUI0->UngridGUI ( );
            }
          else
            {
              this->MainSliceGUI0->UnpackGUI ( );
            }
          this->MainSliceGUI0->Delete () ;
          this->MainSliceGUI0 = NULL;
        }

      if ( this->MainSliceGUI1 )
        {
          this->MainSliceGUI1->SetAndObserveMRMLScene (NULL );
          this->MainSliceGUI1->SetAndObserveModuleLogic ( NULL );
          this->MainSliceGUI1->RemoveGUIObservers ( );
          this->MainSliceGUI1->SetApplicationLogic ( NULL );
          if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
            {
              this->MainSliceGUI1->UngridGUI ( );
            }
          else
            {
              this->MainSliceGUI1->UnpackGUI ( );
            }
          this->MainSliceGUI1->Delete () ;
          this->MainSliceGUI1 = NULL;
        }

      if ( this->MainSliceGUI2 )
        {
          this->MainSliceGUI2->SetAndObserveMRMLScene (NULL );
          this->MainSliceGUI2->SetAndObserveModuleLogic ( NULL );
          this->MainSliceGUI2->RemoveGUIObservers ( );
          this->MainSliceGUI2->SetApplicationLogic ( NULL );
          if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
            {
              this->MainSliceGUI2->UngridGUI ( );
            }
          else
            {
              this->MainSliceGUI2->UnpackGUI ( );
            }
          this->MainSliceGUI2->Delete () ;
          this->MainSliceGUI2 = NULL;
        }
      if ( this->LightboxFrame )
        {
          app->Script ("pack forget %s ", this->LightboxFrame->GetWidgetName ( ) );
          this->LightboxFrame->Delete ( );
          this->LightboxFrame = NULL;
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DestroyMain3DViewer ( )
{
  //

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Destroy fiducial list
      if ( this->FiducialListWidget )
      {
          this->FiducialListWidget->RemoveMRMLObservers ();
          this->FiducialListWidget->SetParent(NULL);
          this->FiducialListWidget->Delete();
          this->FiducialListWidget = NULL;
      }
      
      // Destroy main 3D viewer
      //
      if ( this->ViewerWidget )
        {
        this->ViewerWidget->RemoveMRMLObservers ( );
        if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
          {
            this->ViewerWidget->UngridWidget ( );
          }
        else
          {
            this->ViewerWidget->UnpackWidget ( );
          }
        this->ViewerWidget->SetParent ( NULL );
        this->ViewerWidget->Delete ( );
        this->ViewerWidget = NULL;
        }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayMainSlicerWindow ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->MainSlicerWindow->Display ( );
    int w = this->MainSlicerWindow->GetWidth ( );
    int h = this->MainSlicerWindow->GetHeight ( );
    int vh = app->GetMainLayout()->GetDefault3DViewerHeight();
    int sh = app->GetMainLayout()->GetDefaultSliceGUIFrameHeight();
    int sfh = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1Size();
    int sf2h = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame2Size();
    }
}

    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildMainViewer ( int arrangementType)
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      vtkSlicerWindow *win = this->MainSlicerWindow;
        
      this->LightboxFrame->SetParent ( this->MainSlicerWindow->GetViewFrame ( ) );
      this->LightboxFrame->Create ( );            
      this->CreateMainSliceViewers ( arrangementType );
      this->CreateMain3DViewer (arrangementType );
      this->PackMainViewer ( arrangementType , NULL );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CreateMainSliceViewers ( int arrangementType )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      //
      // 3 Slice Viewers
      //
      this->MainSliceGUI0 = vtkSlicerSliceGUI::New ( );
      this->MainSliceGUI0->SetApplication ( app );
      this->MainSliceGUI0->SetApplicationLogic ( this->ApplicationLogic );

      this->MainSliceGUI1 = vtkSlicerSliceGUI::New ( );
      this->MainSliceGUI1->SetApplication ( app );
      this->MainSliceGUI1->SetApplicationLogic ( this->ApplicationLogic );

      this->MainSliceGUI2 = vtkSlicerSliceGUI::New ( );
      this->MainSliceGUI2->SetApplication ( app );
      this->MainSliceGUI2->SetApplicationLogic ( this->ApplicationLogic );
      
      // TO DO: move this into CreateMainSliceViewers?
      // add observers on GUI, MRML 
      // add observers on Logic
      if ( this->MainSliceLogic0 )
        {
          this->MainSliceGUI0->SetAndObserveModuleLogic ( this->MainSliceLogic0 );
        }
      if (this->MainSliceLogic1 )
        {
          this->MainSliceGUI1->SetAndObserveModuleLogic ( this->MainSliceLogic1 );
        }
      if (this->MainSliceLogic2 )
        {
          this->MainSliceGUI2->SetAndObserveModuleLogic ( this->MainSliceLogic2 );
        }

      if ( this->MainSliceGUI0 )
        {
        this->MainSliceGUI0->BuildGUI ( this->MainSlicerWindow->GetMainSplitFrame ( ), color->SliceGUIRed );
        this->MainSliceGUI0->AddGUIObservers ( );
        this->MainSliceGUI0->SetAndObserveMRMLScene ( this->MRMLScene );
        }
      if ( this->MainSliceGUI1 )
        {
        this->MainSliceGUI1->BuildGUI ( this->MainSlicerWindow->GetMainSplitFrame( ), color->SliceGUIYellow );
        this->MainSliceGUI1->AddGUIObservers ( );
        this->MainSliceGUI1->SetAndObserveMRMLScene ( this->MRMLScene );
        }
      if ( this->MainSliceGUI2 )
        {
        this->MainSliceGUI2->BuildGUI ( this->MainSlicerWindow->GetMainSplitFrame ( ), color->SliceGUIGreen );
        this->MainSliceGUI2->AddGUIObservers ( );
        this->MainSliceGUI2->SetAndObserveMRMLScene ( this->MRMLScene );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CreateMain3DViewer ( int arrangementType )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      //
      // Make 3D Viewer
      //
      this->ViewerWidget = vtkSlicerViewerWidget::New ( );
      this->ViewerWidget->SetApplication( app );
      if ( arrangementType == vtkSlicerGUILayout::SlicerLayoutFourUpView )
        {
        this->ViewerWidget->SetParent ( this->GetLightboxFrame ( ) );
        }
      else
        {
        this->ViewerWidget->SetParent(this->MainSlicerWindow->GetViewPanelFrame());
        }

      // add events
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
      events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      this->ViewerWidget->SetAndObserveMRMLSceneEvents (this->MRMLScene, events );

      // use the events for the fiducial list widget as well
      //events->Delete();
      this->ViewerWidget->Create();
      this->ViewerWidget->GetMainViewer()->SetRendererBackgroundColor (app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->ViewerWidget->UpdateFromMRML();
      // add the fiducial list widget
      this->FiducialListWidget = vtkSlicerFiducialListWidget::New();
      this->FiducialListWidget->SetApplication( app );
      this->FiducialListWidget->SetMainViewer(this->ViewerWidget->GetMainViewer());
      this->FiducialListWidget->Create();
      this->FiducialListWidget->SetAndObserveMRMLSceneEvents (this->MRMLScene, events );
      events->Delete();
      this->FiducialListWidget->UpdateFromMRML();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackMainViewer ( int arrangmentType, const char *whichSlice)
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      // parent the sliceGUI  based on selected view arrangement & build

      switch ( arrangmentType)
        {
        case vtkSlicerGUILayout::SlicerLayoutInitialView:
          this->PackConventionalView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutDefaultView:
          this->PackConventionalView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutFourUpView:
          this->PackFourUpView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutOneUp3DView:
          this->PackOneUp3DView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutOneUpSliceView:
          this->PackOneUpSliceView ( whichSlice );
          break;
        case vtkSlicerGUILayout::SlicerLayoutTabbed3DView:
          this->PackTabbed3DView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutTabbedSliceView:
          this->PackTabbedSliceView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutLightboxView:
          this->PackLightboxView ( );
          break;
        default:
          break;
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RepackMainViewer ( int arrangementType, const char *whichSlice )
{
  this->UnpackMainSliceViewers ( );
  this->UnpackMain3DViewer ( );
  // Since I can't find a way to re-title this main page titled "View",
  // we make sure it's visible, and then 'hide' it only when we want to
  // show tabs that say things other than "View".
  this->MainSlicerWindow->GetViewNotebook()->ShowPage ( "View");
  this->PackMainViewer ( arrangementType, whichSlice );
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackMainSliceViewers (  )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerGUILayout *layout = app->GetMainLayout ( );
    //
    // Unpack or Ungrid 3 main slice viewers
    //
    if ( this->MainSliceGUI0 )
      {
      if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
        {
        this->MainSliceGUI0->UngridGUI ( );
        }
      else
        {
        this->MainSliceGUI0->UnpackGUI ( );
        }
      }

    if ( this->MainSliceGUI1 )
      {
      if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
        {
        this->MainSliceGUI1->UngridGUI ( );
        }
      else
        {
        this->MainSliceGUI1->UnpackGUI ( );
        }
      }

    if ( this->MainSliceGUI2 )
      {
      if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
        {
        this->MainSliceGUI2->UngridGUI ( );
        }
      else
        {
        this->MainSliceGUI2->UnpackGUI ( );
        }
      }
    if ( this->LightboxFrame )
      {
      app->Script ("pack forget %s ", this->LightboxFrame->GetWidgetName ( ) );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackMain3DViewer (  )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkSlicerGUILayout *layout = app->GetMainLayout ( );
    //
    // 3D Viewer
    //
    this->MainSlicerWindow->GetViewNotebook()->RemovePagesMatchingTag ( this->ViewerPageTag );
    if ( layout->GetCurrentViewArrangement() == vtkSlicerGUILayout::SlicerLayoutFourUpView )
      {
      this->ViewerWidget->UngridWidget ( );
      }
    else
      {
      this->ViewerWidget->UnpackWidget ( );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackConventionalView ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame and secondary panel frame.
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 1 );
      
      this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ) );      
      this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ) );
      this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ) );
      this->ViewerWidget->PackWidget(this->MainSlicerWindow->GetViewFrame() );

      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutDefaultView );
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Size ( layout->GetDefaultSliceGUIFrameHeight () );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackOneUp3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame only
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 0 );
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );      
      this->ViewerWidget->PackWidget(this->MainSlicerWindow->GetViewFrame() );
      
      this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));
      this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));
      this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));
      
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutOneUp3DView );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackOneUpSliceView ( const char * whichSlice )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame only
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 0 );
      
      if ( !strcmp (whichSlice, "Red" ) )
        {
        this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));
        this->MainSliceGUI1->PackGUI ( NULL );
        this->MainSliceGUI2->PackGUI ( NULL );
        }
      else if ( !strcmp ( whichSlice, "Yellow" ) )
        {
        this->MainSliceGUI0->PackGUI ( NULL);
        this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));
        this->MainSliceGUI2->PackGUI ( NULL );
        }
      else if ( !strcmp ( whichSlice, "Green" ) )
        {
        this->MainSliceGUI0->PackGUI ( NULL );
        this->MainSliceGUI1->PackGUI ( NULL );
        this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));
        }

//      this->ViewerWidget->PackWidget(this->MainSlicerWindow->GetViewFrame() );
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutOneUpSliceView );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackFourUpView ( )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose both the main panel frame and secondary panel frame
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 0 );
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Size ( 0 );
      
      // Use this frame in MainSlicerWindow's ViewFrame to grid in the various viewers.
      this->Script ( "pack %s -side top -fill both -expand y -padx 0 -pady 0 ", this->LightboxFrame->GetWidgetName ( ) );
      this->Script ("grid rowconfigure %s 0 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid rowconfigure %s 1 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid columnconfigure %s 0 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid columnconfigure %s 1 -weight 1", this->LightboxFrame->GetWidgetName() );
      
        this->ViewerWidget->GridWidget ( this->LightboxFrame, 0, 1 );
        this->MainSliceGUI0->GridGUI ( this->GetLightboxFrame ( ), 0, 0 );
        this->MainSliceGUI1->GridGUI ( this->GetLightboxFrame ( ), 1, 0 );
        this->MainSliceGUI2->GridGUI ( this->GetLightboxFrame ( ), 1, 1 );

        
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutFourUpView );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackTabbed3DView ( )
{

  // TODO: implement multi-tabbed ViewerWidgets
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Size ( layout->GetDefaultSliceGUIFrameHeight() );      
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 0 );

      this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));
      this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));
      this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ));

      // Add a page for the current view, and each saved view.
      this->MainSlicerWindow->GetViewNotebook()->AddPage("Current view", NULL, NULL, this->ViewerPageTag );
      this->ViewerWidget->PackWidget(this->MainSlicerWindow->GetViewNotebook()->GetFrame ("Current view" ));

      // don't know how to change the title of this one,
      // so just hide it in this configuration, and expose
      // it again when the view configuration changes.
      this->MainSlicerWindow->GetViewNotebook()->HidePage ( "View");
      
      // Tab the 3D view
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutTabbed3DView );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackTabbedSliceView ( )
{
  // TODO: implement this and add an icon on the toolbar for it
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      this->MainSlicerWindow->SetMainPanelVisibility ( 1 );
      this->MainSlicerWindow->SetSecondaryPanelVisibility ( 0 );

      this->MainSlicerWindow->GetViewNotebook()->AddPage("Red slice", NULL, NULL, this->ViewerPageTag );
//      this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));
      this->MainSliceGUI0->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Red slice") );
      this->MainSlicerWindow->GetViewNotebook()->AddPage("Yellow slice", NULL, NULL, this->ViewerPageTag );
//      this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));
      this->MainSliceGUI1->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Yellow slice") );
      this->MainSlicerWindow->GetViewNotebook()->AddPage("Green slice", NULL, NULL, this->ViewerPageTag );
//      this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetViewFrame ( ));      
      this->MainSliceGUI2->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Green slice") );

      // don't know how to change the title of this one,
      // so just hide it in this configuration, and expose
      // it again when the view configuration changes.
      this->MainSlicerWindow->GetViewNotebook()->HidePage ( "View");
//      this->ViewerWidget->PackWidget(this->MainSlicerWindow->GetViewFrame() );
      
      // Tab the slices
      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutTabbedSliceView );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackLightboxView ( )
{
  /*
  // TO DO implement this.
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutLightboxView );
    }
  */
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Save3DViewConfig ( )
{
  if ( this->ViewerWidget )
    {
      // TODO: Save the ViewerWidget's Camera Node
      this->ViewerWidget->GetMainViewer()->GetRenderer()->ComputeVisiblePropBounds ( this->MainRendererBBox );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Restore3DViewConfig ( )
{
  if ( this->ViewerWidget )
    {
      // TODO: Restore the ViewerWidget's Camera Node
      this->ViewerWidget->GetMainViewer()->GetRenderer()->ResetCamera ( );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMainSliceViewersToCollection ( )
{
  if ( this->SliceGUICollection != NULL )
    {
      if ( this->MainSliceGUI0 )
        {
          this->AddSliceGUIToCollection ( this->MainSliceGUI0 );
        }
      if ( this->MainSliceGUI1 )
        {
          this->AddSliceGUIToCollection ( this->MainSliceGUI1 );
        }
      if ( this->MainSliceGUI2 )
        {
          this->AddSliceGUIToCollection ( this->MainSliceGUI2 );
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveMainSliceViewersFromCollection ( )
{
  if ( this->SliceGUICollection != NULL )
    {
      if ( this->MainSliceGUI0 )
        {
          this->RemoveSliceGUIFromCollection ( this->MainSliceGUI0 );
        }
      if ( this->MainSliceGUI1 )
        {
          this->RemoveSliceGUIFromCollection ( this->MainSliceGUI1 );
        }
      if ( this->MainSliceGUI2 )
        {
          this->RemoveSliceGUIFromCollection ( this->MainSliceGUI2 );
        }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddSliceGUIToCollection ( vtkSlicerSliceGUI *s)
{
  
    if ( ( this->SliceGUICollection != NULL) && (s != NULL ) ) {
      this->SliceGUICollection->AddItem ( s );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveSliceGUIFromCollection ( vtkSlicerSliceGUI *s )
{
    if ( (this->SliceGUICollection != NULL) && (s != NULL))
      {
           this->SliceGUICollection->InitTraversal ( );
            vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast ( this->SliceGUICollection->GetNextItemAsObject ( ) );
            while ( g != NULL ) {
                if ( g == s )
                    {
                        this->SliceGUICollection->RemoveItem ( g );
                        break;
                    }
                g = vtkSlicerSliceGUI::SafeDownCast (this->SliceGUICollection->GetNextItemAsObject ( ) );
            }
      }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainSliceViewers ( )
{
  if ( this->MainSliceGUI0 && this->MainSliceGUI1 && this->MainSliceGUI2 )
    {
      this->GetMainSliceGUI0()->GetSliceController()->GetSliceNode()->SetOrientationToAxial();
      this->GetMainSliceGUI1()->GetSliceController()->GetSliceNode()->SetOrientationToSagittal();
      this->GetMainSliceGUI2()->GetSliceController()->GetSliceNode()->SetOrientationToCoronal();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMainSliceViewerObservers ( )
{
  if ( this->MainSliceGUI0 && this->MainSliceGUI1 && this->MainSliceGUI2 )
    {
      this->GetMainSliceGUI0()->AddGUIObservers () ;
      this->GetMainSliceGUI1()->AddGUIObservers ();
      this->GetMainSliceGUI2()->AddGUIObservers ();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveMainSliceViewerObservers ( )
{
  if ( this->MainSliceGUI0 && this->MainSliceGUI1 && this->MainSliceGUI2 )
    {
      this->GetMainSliceGUI0()->RemoveGUIObservers () ;
      this->GetMainSliceGUI1()->RemoveGUIObservers ();
      this->GetMainSliceGUI2()->RemoveGUIObservers ();
    }
}


  
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetAndObserveMainSliceLogic ( vtkSlicerSliceLogic *l0,
                                                            vtkSlicerSliceLogic *l1,
                                                            vtkSlicerSliceLogic *l2 )
{

  if ( this->MainSliceGUI0 && this->MainSliceGUI1 && this->MainSliceGUI2 )
    {
      this->GetMainSliceGUI0()->SetAndObserveModuleLogic ( l0 );
      this->GetMainSliceGUI1()->SetAndObserveModuleLogic ( l1 );
      this->GetMainSliceGUI2()->SetAndObserveModuleLogic ( l2 );
    }
  // Set and register the main slice logic here to reassign when
  // viewers are destroyed and recreated during view layout changes.
  // Maybe don't need this anymore?
  this->SetMainSliceLogic0 ( l0 );
  this->SetMainSliceLogic1 ( l1 );
  this->SetMainSliceLogic2 ( l2 );

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PopulateModuleChooseList ( )
{
#ifndef TOOLBAR_DEBUG
  this->GetApplicationToolbar()->GetModuleChooseGUI()->Populate();
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackFirstSliceViewerFrame ( )
{
  /*
  this->Script ("pack %s -side left  -expand y -fill both -padx 0 -pady 0", 
    this->DefaultSlice0Frame->GetWidgetName( ) );
  */
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUIFrames ( )
{

    if ( this->GetApplication() != NULL ) {
        // pointers for convenience
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
        vtkSlicerGUILayout *layout = app->GetMainLayout ( );
        
        if ( this->MainSlicerWindow != NULL ) {

            this->MainSlicerWindow->GetMainPanelFrame()->SetWidth ( layout->GetDefaultGUIPanelWidth() );
            this->MainSlicerWindow->GetMainPanelFrame()->SetHeight ( layout->GetDefaultGUIPanelHeight() );
            this->MainSlicerWindow->GetMainPanelFrame()->SetReliefToSunken();

            this->TopFrame->SetParent ( this->MainSlicerWindow->GetMainPanelFrame ( ) );
            this->TopFrame->Create ( );
            this->TopFrame->SetHeight ( layout->GetDefaultTopFrameHeight ( ) );

            this->LogoFrame->SetParent ( this->TopFrame );
            this->LogoFrame->Create( );
            this->LogoFrame->SetHeight ( layout->GetDefaultTopFrameHeight ( ) );            
            
            this->DropShadowFrame->SetParent ( this->MainSlicerWindow->GetMainPanelFrame() );
            this->DropShadowFrame->Create ( );
            // why is the theme not setting this???
            this->DropShadowFrame->SetBackgroundColor ( 0.9, 0.9, 1.0);

            this->SlicesControlFrame->SetParent ( this->DropShadowFrame );
            this->SlicesControlFrame->Create( );
            this->SlicesControlFrame->ExpandFrame ( );
            this->SlicesControlFrame->SetLabelText ( "Manipulate Slice Views");
            this->SlicesControlFrame->GetFrame()->SetHeight ( layout->GetDefaultSlicesControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( this->DropShadowFrame );
            this->ViewControlFrame->Create( );
            this->ViewControlFrame->ExpandFrame ( );
            this->ViewControlFrame->SetLabelText ( "Manipulate 3D View" );
            this->ViewControlFrame->GetFrame()->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );
            

            app->Script ( "pack %s -side top -fill x -padx 1 -pady 1", this->TopFrame->GetWidgetName() );
            app->Script ( "pack %s -side left -expand 1 -fill x -padx 1 -pady 1", this->LogoFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -ipady 1 -pady 0", this->DropShadowFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 0 -ipady 5 -pady 2", this->ViewControlFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 0 -ipady 5 -pady 1", this->SlicesControlFrame->GetWidgetName() );

        }
    }

}

