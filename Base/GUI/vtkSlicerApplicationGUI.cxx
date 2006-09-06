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
#include "vtkImplicitPlaneWidget.h"

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

#include "vtkSlicerWindow.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerMRMLSaveDataWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");

// temporary crud for vtkDebugLeak hunting. Will remove
// these and other related #ifndefs-#endifs throughout.

//#define LOGODISPLAY_DEBUG
//#define TOOLBAR_DEBUG
//#define VIEWCONTROL_DEBUG
//#define SLICESCONTROL_DEBUG
//#define MODULECHOOSE_DEBUG

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
    //---  
    // widgets used in the Slice module
    //---

    //--- slicer main window
    this->MainSlicerWin = vtkSlicerWindow::New ( );

    // Frames that comprise the Main Slicer GUI

    this->LogoFrame = vtkKWFrame::New();
    this->ModuleChooseFrame = vtkKWFrame::New();
    this->SlicesControlFrame = vtkKWFrame::New();
    this->ViewControlFrame = vtkKWFrame::New();
    this->LightboxFrame = NULL;

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
#ifndef MODULECHOOSE_DEBUG    
    this->ModuleChooseGUI = vtkSlicerModuleChooseGUI::New ( );
#endif
#ifndef LOGODISPLAY_DEBUG    
    this->LogoDisplayGUI = vtkSlicerLogoDisplayGUI::New ( );
#endif
    
    //--- Main viewer, 3 main slice viewers and collection.
    this->ViewerWidget = NULL;
    this->MainSliceGUI0 = NULL;
    this->MainSliceGUI1 = NULL;
    this->MainSliceGUI2 = NULL;
    this->SliceGUICollection = NULL;
    
    //--- Save the main slice logic in these.
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;

    this->PlaneWidget = NULL;

    //--- Save and load scene dialogs, widgets
    this->LoadSceneDialog = vtkKWLoadSaveDialog::New();
    this->SaveSceneDialog = vtkKWLoadSaveDialog::New();   
    this->SaveDataWidget = NULL;
    this->SaveDataDialog = NULL;      

    //--- unique tag used to mark all view notebook pages
    //--- so that they can be identified and deleted when 
    //--- viewer is reformatted.
    this->ViewerPageTag = 1999;

}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( )
{

    if ( this->ViewControlGUI ) {
      this->ViewControlGUI->Delete ( );
      this->ViewControlGUI = NULL;
    }
    if ( this->ModuleChooseGUI ) {
      this->ModuleChooseGUI->Delete ();
      this->ModuleChooseGUI = NULL;
    }
    if ( this->LogoDisplayGUI ) {
      this->LogoDisplayGUI->Delete ( );
      this->LogoDisplayGUI = NULL;
    }
    if ( this->SlicesControlGUI ) {
      this->SlicesControlGUI->Delete ( );
      this->SlicesControlGUI = NULL;
    }
    if ( this->ApplicationToolbar ) {
      this->ApplicationToolbar->Delete ( );
      this->ApplicationToolbar = NULL;
    }
    if ( this->SliceGUICollection )
      {
        this->SliceGUICollection->RemoveAllItems();
        this->SetSliceGUICollection ( NULL );
      }

    this->DestroyMain3DViewer ( );
    this->DestroyMainSliceViewers ( );

    // Delete frames
    if ( this->LogoFrame ) {
      this->LogoFrame->SetParent ( NULL );
        this->LogoFrame->Delete ();
        this->LogoFrame = NULL;
    }
    if ( this->ModuleChooseFrame ) {
      this->ModuleChooseFrame->SetParent ( NULL );
        this->ModuleChooseFrame->Delete ();
        this->ModuleChooseFrame = NULL;
    }
    if ( this->SlicesControlFrame ) {
      this->SlicesControlFrame->SetParent ( NULL );
        this->SlicesControlFrame->Delete ( );
        this->SlicesControlFrame = NULL;
    }
    if ( this->ViewControlFrame ) {
      this->ViewControlFrame->SetParent ( NULL );
        this->ViewControlFrame->Delete ( );
        this->ViewControlFrame = NULL;
    }

    if ( this->LoadSceneDialog ) {
      this->LoadSceneDialog->SetParent ( NULL );
        this->LoadSceneDialog->Delete();
        this->LoadSceneDialog = NULL;
    }
    if ( this->SaveSceneDialog ) {
      this->SaveSceneDialog->SetParent ( NULL );
        this->SaveSceneDialog->Delete();
        this->SaveSceneDialog = NULL;
    }
    if ( this->MainSlicerWin ) {
      this->MainSlicerWin->SetParent ( NULL );
        this->MainSlicerWin->Delete ( );
        this->MainSlicerWin = NULL;
    }
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;

    if (this->SaveDataWidget)
      {
        this->SaveDataWidget->SetParent ( NULL );
      this->SaveDataWidget->Delete();
      }
    if (this->SaveDataDialog)
      {
        this->SaveDataDialog->SetParent ( NULL );
      this->SaveDataDialog->Delete();
      }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerApplicationGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "MainSlicerWin: " << this->GetMainSlicerWin ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessLoadSceneCommand()
{
    this->LoadSceneDialog->RetrieveLastPathFromRegistry(
      "OpenPath");

    this->LoadSceneDialog->Invoke();
    // If a file has been selected for loading...
    char *fileName = this->LoadSceneDialog->GetFileName();
    if ( fileName ) 
      {
        if (this->GetMRMLScene()) 
          {
          this->GetMRMLScene()->SetURL(fileName);
          this->GetMRMLScene()->Connect();
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
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
        if (this->GetMRMLScene()) 
          {
          this->GetMRMLScene()->SetURL(fileName);
          this->GetMRMLScene()->Import();
          this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
          }
      }
    return;
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessSaveSceneAsCommand()
{
    this->SaveSceneDialog->RetrieveLastPathFromRegistry(
      "OpenPath");

    
   
    this->SaveSceneDialog->Invoke();



    // If a file has been selected for saving...
    char *fileName = this->SaveSceneDialog->GetFileName();
    if ( fileName ) 
      {
      this->SaveDataDialog = vtkKWDialog::New();
      this->SaveDataDialog->SetParent ( this->MainSlicerWin );
      this->SaveDataDialog->SetTitle("Save Unsaved Data");
      this->SaveDataDialog->SetSize(400, 100);
      this->SaveDataDialog->Create ( );
      //this->Script ( "pack %s -fill both -expand true",
      //            this->SaveDataDialog->GetWidgetName());
      
      this->SaveDataWidget = vtkSlicerMRMLSaveDataWidget::New();
      this->SaveDataWidget->SetParent ( this->SaveDataDialog);
      this->SaveDataWidget->SetAndObserveMRMLScene(this->GetMRMLScene());

      vtksys_stl::string dir =  vtksys::SystemTools::GetParentDirectory(fileName);   
      dir = dir + vtksys_stl::string("/");
      this->SaveDataWidget->SetFileDirectoryName(dir.c_str());

      this->SaveDataWidget->Create();
      this->SaveDataWidget->AddObserver ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );

      // TODO: make update event driven so that we don't have to call this
      int nrows = this->SaveDataWidget->UpdateFromMRML();

      if (nrows > 0) 
      {
        
        this->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                  this->SaveDataWidget->GetWidgetName());
        this->SaveDataDialog->Invoke ( );
      }

      this->SaveDataWidget->RemoveObservers ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
      this->SaveDataWidget->SetParent(NULL);
      this->SaveDataDialog->SetParent(NULL);    
      this->SaveDataWidget->Delete();
      this->SaveDataDialog->Delete();      
      this->SaveDataWidget=NULL;
      this->SaveDataDialog=NULL;      

      if (this->GetMRMLScene()) 
        {
        // convert absolute paths to relative
        this->MRMLScene->InitTraversal();

        vtkMRMLNode *node;
        while ( (node = this->MRMLScene->GetNextNodeByClass("vtkMRMLStorageNode") ) != NULL)
          {
          vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
          if (!this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
            {
            vtksys_stl::string directory = vtksys::SystemTools::GetParentDirectory(fileName);   
            directory = directory + vtksys_stl::string("/");

            itksys_stl::string relPath = itksys::SystemTools::RelativePath((const char*)directory.c_str(), snode->GetFileName());
            snode->SetFileName(relPath.c_str());
            }
          }

        this->GetMRMLScene()->SetURL(fileName);
        this->GetMRMLScene()->Commit();  
        this->SaveSceneDialog->SaveLastPathToRegistry("OpenPath");
        }
      }
    return;
}    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( )
{

  this->GetMainSlicerWin()->GetFileMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
  this->LoadSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
#ifndef TOOLBAR_DEBUG
  this->GetApplicationToolbar()->AddGUIObservers ( );
#endif
#ifndef VIEWCONTROL_DEBUG
  this->GetViewControlGUI()->AddGUIObservers ( );
#endif
#ifndef SLICESCONTROL_DEBUG
  this->GetSlicesControlGUI ( )->AddGUIObservers ( );
#endif
#ifndef MODULECHOOSE_DEBUG
  this->GetModuleChooseGUI ( )->AddGUIObservers ( );
#endif
#ifndef LOGODISPLAY_DEBUG
  this->GetLogoDisplayGUI ( )->AddGUIObservers ( );
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( )
{
    this->GetMainSlicerWin()->GetFileMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
    this->LoadSceneDialog->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *) this->GUICallbackCommand );
    this->SaveSceneDialog->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *) this->GUICallbackCommand );

#ifndef TOOLBAR_DEBUG
    this->GetApplicationToolbar()->RemoveGUIObservers ( );
#endif
#ifndef VIEWCONTROL_DEBUG
    this->GetViewControlGUI ( )->RemoveGUIObservers ( );
#endif
#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI ( )->RemoveGUIObservers ( );
#endif
#ifndef MODULECHOOSE_DEBUG    
    this->GetModuleChooseGUI ( )->RemoveGUIObservers ( );
#endif
#ifndef LOGODISPLAY_DEBUG
    this->GetLogoDisplayGUI ( )->RemoveGUIObservers ( );
#endif    
    this->RemoveMainSliceViewerObservers ( );

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

    vtkKWLoadSaveDialog *filebrowse = vtkKWLoadSaveDialog::SafeDownCast(caller);
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
    vtkSlicerGUILayout *layout = app->GetMainLayout ( );
        
    vtkSlicerMRMLSaveDataWidget *saveDataWidget = vtkSlicerMRMLSaveDataWidget::SafeDownCast(caller);
    if (saveDataWidget == this->SaveDataWidget && event == vtkSlicerMRMLSaveDataWidget::DataSavedEvent)
      {
        this->SaveDataDialog->OK();
      }
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
void vtkSlicerApplicationGUI::BuildGUI ( )
{
    int i;
    
    // Set up the conventional window: 3Dviewer, slice widgets, UI panel for now.
    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerGUILayout *layout = app->GetMainLayout ( );
        
        // Set a pointer to the MainSlicerWin in vtkSlicerGUILayout, and
        // Set default sizes for all main frames (UIpanel and viewers) in GUI
        layout->SetMainSlicerWin ( this->MainSlicerWin );
        layout->InitializeLayoutDimensions ( );

        if ( this->MainSlicerWin != NULL ) {

            // set up Slicer's main window
            this->MainSlicerWin->SecondaryPanelVisibilityOn ( );
            this->MainSlicerWin->MainPanelVisibilityOn ( );
            app->AddWindow ( this->MainSlicerWin );

            // Create the console before the window
            // - this will make the console independent of the main window
            //   so it can be raised/lowered independently
            this->MainSlicerWin->GetTclInteractor()->SetApplication(app);
            this->MainSlicerWin->GetTclInteractor()->Create();

            // TODO: it would be nice to make this a menu option on the tkcon itself,
            // but for now just up the font size
            this->MainSlicerWin->Script(".vtkKWTkcon0.tab1 configure -font {Courier 12}");

            this->MainSlicerWin->Create ( );        

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

            // Build Module Selection GUI Panel
#ifndef MODULECHOOSE_DEBUG
            vtkSlicerModuleChooseGUI * mcGUI = this->GetModuleChooseGUI ( );
            mcGUI->SetApplicationGUI ( this );
            mcGUI->SetApplication ( app );
            mcGUI->BuildGUI ( this->ModuleChooseFrame );
#endif
            // Build SlicesControl panel
#ifndef SLICESCONTROL_DEBUG            
            vtkSlicerSlicesControlGUI *scGUI = this->GetSlicesControlGUI ( );
            scGUI->SetApplicationGUI ( this );
            scGUI->SetApplication ( app );
            scGUI->BuildGUI ( this->SlicesControlFrame );
#endif

            // Build 3DView Control panel
#ifndef VIEWCONTROL_DEBUG
            vtkSlicerViewControlGUI *vcGUI = this->GetViewControlGUI ( );
            vcGUI->SetApplicationGUI ( this );
            vcGUI->SetApplication ( app );
            vcGUI->BuildGUI ( this->ViewControlFrame );
#endif

            // Turn off the tabs for pages in the ModuleControlGUI
            this->MainSlicerWin->GetMainNotebook()->ShowIconsOff ( );

            this->MainSlicerWin->GetMainNotebook()->SetReliefToFlat();
            this->MainSlicerWin->GetMainNotebook()->SetBorderWidth ( 0 );
            this->MainSlicerWin->GetMainNotebook()->SetHighlightThickness ( 0 );

            //this->MainSlicerWin->GetMainNotebook()->SetAlwaysShowTabs ( 0 );
            this->MainSlicerWin->GetMainNotebook()->SetUseFrameWithScrollbars ( 1 );
            // Build 3DViewer and Slice Viewers

            this->RemoveMainSliceViewersFromCollection ( );            
            this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutDefaultView );
            this->AddMainSliceViewersToCollection ( );            

            // Construct menu bar and set up global key bindings
            // 
            // File Menu
            //
            this->GetMainSlicerWin()->GetFileMenu()->InsertCommand (
                      this->GetMainSlicerWin()->GetFileMenuInsertPosition(),
                                      "Load Scene...", this, "ProcessLoadSceneCommand");

            this->GetMainSlicerWin()->GetFileMenu()->InsertCommand (
                      this->GetMainSlicerWin()->GetFileMenuInsertPosition(),
                                      "Import Scene...", this, "ProcessImportSceneCommand");

            this->GetMainSlicerWin()->GetFileMenu()->InsertCommand (this->GetMainSlicerWin()->GetFileMenuInsertPosition(),
                                               "Save Scene As...", this, "ProcessSaveSceneAsCommand");

            this->GetMainSlicerWin()->GetFileMenu()->InsertSeparator (
                this->GetMainSlicerWin()->GetFileMenuInsertPosition());

            //
            // Edit Menu
            //
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ("Set Home", NULL, NULL);
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+H");
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ( "Undo", NULL, "$::slicer3::MRMLScene Undo" );
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Z");
            i = this->MainSlicerWin->GetEditMenu()->AddCommand ( "Redo", NULL, "$::slicer3::MRMLScene Redo" );
            this->MainSlicerWin->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+Y");

            //
            // View Menu
            //
            this->GetMainSlicerWin()->GetViewMenu()->InsertCommand (
                      this->GetMainSlicerWin()->GetViewMenuInsertPosition(),
                                      "Single Slice", NULL, "$::slicer3::ApplicationGUI UnpackMainSliceViewerFrames ; $::slicer3::ApplicationGUI PackFirstSliceViewerFrame ");
            this->GetMainSlicerWin()->GetViewMenu()->InsertCommand (
                      this->GetMainSlicerWin()->GetViewMenuInsertPosition(),
                                      "Three Slices", NULL, "$::slicer3::ApplicationGUI UnpackMainSliceViewerFrames ; $::slicer3::ApplicationGUI PackFirstSliceViewerFrame ");


            //i = this->MainSlicerWin->GetWindowMenu()->AddCommand ( ? );
            //i = this->MainSlicerWin->GetHelpMenu()->AddCommand ( ? );

            this->LoadSceneDialog->SetParent ( this->MainSlicerWin );
            this->LoadSceneDialog->Create ( );
            this->LoadSceneDialog->SetFileTypes("{ {MRML Scene} {*.mrml} }");
            this->LoadSceneDialog->RetrieveLastPathFromRegistry("OpenPath");

            this->SaveSceneDialog->SetParent ( this->MainSlicerWin );
            this->SaveSceneDialog->Create ( );
            this->SaveSceneDialog->SetFileTypes("{ {MRML Scene} {*.mrml} }");
            this->SaveSceneDialog->SaveDialogOn();
            this->SaveSceneDialog->RetrieveLastPathFromRegistry("OpenPath");
        }

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
    
      // Destroy main 3D viewer
      //
      if ( this->ViewerWidget )
        {
          if ( this->PlaneWidget )
            {
              this->PlaneWidget->SetInteractor( NULL );
              this->PlaneWidget->Delete ( );
              this->PlaneWidget = NULL;
            }
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
    this->MainSlicerWin->Display ( );
    int w = this->MainSlicerWin->GetWidth ( );
    int h = this->MainSlicerWin->GetHeight ( );
    int vh = app->GetMainLayout()->GetDefault3DViewerHeight();
    int sh = app->GetMainLayout()->GetDefaultSliceGUIFrameHeight();
    int sfh = this->MainSlicerWin->GetSecondarySplitFrame()->GetFrame1Size();
    int sf2h = this->MainSlicerWin->GetSecondarySplitFrame()->GetFrame2Size();
    }
}

    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildMainViewer ( int arrangementType)
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      vtkSlicerWindow *win = this->MainSlicerWin;
        
      // If Main viewer and 3 main sliceGUIs already exist, destroy them.
      this->Save3DViewConfig ( );
      this->DestroyMain3DViewer ( );
      this->DestroyMainSliceViewers ( );

      this->CreateMainSliceViewers ( arrangementType );
      this->CreateMain3DViewer (arrangementType );
      this->Restore3DViewConfig ( );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CreateMainSliceViewers ( int arrangementType )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
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
          this->MainSliceGUI0->AddGUIObservers ( );
          this->MainSliceGUI0->SetAndObserveMRMLScene ( this->MRMLScene );
        }
      if ( this->MainSliceGUI1 )
        {
          this->MainSliceGUI1->AddGUIObservers ( );
          this->MainSliceGUI1->SetAndObserveMRMLScene ( this->MRMLScene );
        }
      if ( this->MainSliceGUI2 )
        {
          this->MainSliceGUI2->AddGUIObservers ( );
          this->MainSliceGUI2->SetAndObserveMRMLScene ( this->MRMLScene );
        }

      // parent the sliceGUI  based on selected view arrangement & build
      switch ( arrangementType )
        {
        case vtkSlicerGUILayout::SlicerLayoutInitialView:
          this->DisplayConventionalView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutDefaultView:
          this->DisplayConventionalView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutFourUpView:
          this->DisplayFourUpView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutOneUp3DView:
          this->DisplayOneUp3DView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutOneUpSliceView:
          this->DisplayOneUpSliceView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutTabbed3DView:
          this->DisplayTabbed3DViewSliceViewers ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutTabbedSliceView:
          this->DisplayTabbedSliceView ( );
          break;
        case vtkSlicerGUILayout::SlicerLayoutLightboxView:
          this->DisplayLightboxView ( );
          break;
        default:
          break;
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
      // 3D Viewer
      //
      // only re-create the 3D view when we need it...
      if ( (arrangementType != vtkSlicerGUILayout::SlicerLayoutOneUpSliceView) &&
           (arrangementType != vtkSlicerGUILayout::SlicerLayoutLightboxView ) &&
           (arrangementType != vtkSlicerGUILayout::SlicerLayoutTabbedSliceView) )
        {
          this->MainSlicerWin->GetViewNotebook()->RemovePagesMatchingTag(this->ViewerPageTag );      
          this->ViewerWidget = vtkSlicerViewerWidget::New ( );
          this->ViewerWidget->SetApplication( app );
          if ( arrangementType == vtkSlicerGUILayout::SlicerLayoutFourUpView )
            {
              this->ViewerWidget->SetParent ( this->GetLightboxFrame ( ) );
            }
          else
            {
              this->ViewerWidget->SetParent(this->MainSlicerWin->GetViewFrame());
            }
          this->ViewerWidget->SetMRMLScene(this->MRMLScene);
          this->ViewerWidget->Create();
          this->ViewerWidget->GetMainViewer()->SetRendererBackgroundColor (app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
          this->ViewerWidget->UpdateFromMRML();
          if ( arrangementType == vtkSlicerGUILayout::SlicerLayoutFourUpView )
            {
              this->ViewerWidget->GridWidget ( 0, 1 );
            }
          else
            {
              this->ViewerWidget->PackWidget();
            }

          // TODO: this requires a change to KWWidgets
          this->PlaneWidget = vtkImplicitPlaneWidget::New();
          this->PlaneWidget->SetInteractor( this->GetRenderWindowInteractor() );
          this->PlaneWidget->PlaceWidget();
          this->PlaneWidget->On();
        }
    }
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
void vtkSlicerApplicationGUI::DisplayConventionalView ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame and secondary panel frame.
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 1 );
      
      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIRed );
      this->MainSliceGUI0->PackGUI ( );      
      // Yellow slice viewer
      this->MainSliceGUI1->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer          
      this->MainSliceGUI2->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIGreen );
      this->MainSliceGUI2->PackGUI ( );

      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      //      layout->ConfigureMainSlicerWindow ( );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutDefaultView );
      this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame1Size ( layout->GetDefaultSliceGUIFrameHeight () );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayOneUp3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame only
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 0 );
      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );      

      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIRed );
      this->MainSliceGUI0->PackGUI ( );
      // Yellow slice viewer
      this->MainSliceGUI1->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer          
      this->MainSliceGUI2->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIGreen );
      this->MainSliceGUI2->PackGUI ( );
      
      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutOneUp3DView );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayOneUpSliceView ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose the main panel frame only
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 0 );
      
      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->MainSlicerWin->GetViewFrame ( ), color->SliceGUIRed );
      this->MainSliceGUI0->PackGUI ( );
      // Yellow slice viewer
      this->MainSliceGUI1->BuildGUI ( NULL, color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer
      this->MainSliceGUI2->BuildGUI ( NULL, color->SliceGUIGreen );
      this->MainSliceGUI2->PackGUI ( );

      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutOneUpSliceView );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayFourUpView ( )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      // Expose both the main panel frame and secondary panel frame
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 0 );
      this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame1Size ( 0 );
      
      // Use this frame in MainSlicerWin's ViewFrame to grid in the various viewers.
      this->LightboxFrame = vtkKWFrame::New ( );
      this->LightboxFrame->SetParent ( this->MainSlicerWin->GetViewFrame ( ) );
      this->LightboxFrame->Create ( );
      this->Script ( "pack %s -side top -fill both -expand y -padx 0 -pady 0 ", this->LightboxFrame->GetWidgetName ( ) );
      this->Script ("grid rowconfigure %s 0 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid rowconfigure %s 1 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid columnconfigure %s 0 -weight 1", this->LightboxFrame->GetWidgetName() );
      this->Script ("grid columnconfigure %s 1 -weight 1", this->LightboxFrame->GetWidgetName() );
      
      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->GetLightboxFrame ( ), color->SliceGUIRed );
       this->MainSliceGUI0->GridGUI ( 0, 0 );
      // Yellow slice viewer
      this->MainSliceGUI1->BuildGUI ( this->GetLightboxFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->GridGUI ( 1, 0 );
      // Green slice viewer          
      this->MainSliceGUI2->BuildGUI ( this->GetLightboxFrame ( ), color->SliceGUIGreen );
      this->MainSliceGUI2->GridGUI ( 1, 1 );

      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutFourUpView );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayTabbed3DViewSliceViewers ( )
{

  // TODO: implement multi-tabbed ViewerWidgets
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame1Size ( layout->GetDefaultSliceGUIFrameHeight() );      
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 0 );

      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIRed );
      this->MainSliceGUI0->PackGUI ( );
      // Yellow slice viewer
      this->MainSliceGUI1->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer
      this->MainSliceGUI2->BuildGUI ( this->MainSlicerWin->GetSecondaryPanelFrame ( ), color->SliceGUIGreen );
      this->MainSliceGUI2->PackGUI ( );
      // Tab the 3D view
      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutTabbed3DView );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayTabbedSliceView ( )
{
  // TODO: implement this and add an icon on the toolbar for it
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );
      this->MainSlicerWin->SetMainPanelVisibility ( 1 );
      this->MainSlicerWin->SetSecondaryPanelVisibility ( 0 );

      // Red slice viewer
      this->MainSliceGUI0->BuildGUI ( this->MainSlicerWin->GetViewFrame ( ), color->SliceGUIRed );
      this->MainSliceGUI0->PackGUI ( );
      // Yellow slice viewer
      this->MainSlicerWin->GetViewNotebook()->AddPage("yellow slice", NULL, NULL, this->ViewerPageTag );
      this->MainSliceGUI1->BuildGUI ( this->MainSlicerWin->GetViewFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer          
      this->MainSlicerWin->GetViewNotebook()->AddPage("green slice", NULL, NULL, this->ViewerPageTag );
      this->MainSliceGUI2->BuildGUI ( this->MainSlicerWin->GetViewFrame ( ), color->SliceGUIGreen );
      this->MainSliceGUI2->PackGUI ( );      
      // Tab the Slice views
      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutTabbedSliceView );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DisplayLightboxView ( )
{
  /*
  // TO DO implement this.
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      this->MainSlicerWin->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetCurrentViewArrangement ( vtkSlicerGUILayout::SlicerLayoutLightboxView );
    }
  */
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
  this->SetMainSliceLogic0 ( l0 );
  this->SetMainSliceLogic1 ( l1 );
  this->SetMainSliceLogic2 ( l2 );

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PopulateModuleChooseList ( )
{
    const char* mName;
    vtkSlicerModuleGUI *m;

    if ( this->GetApplication( )  != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        //--- ALL modules pull-down menu 
        if ( app->GetModuleGUICollection ( ) != NULL ) {
            app->GetModuleGUICollection( )->InitTraversal( );
            m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            while ( m != NULL ) {
                mName = m->GetUIPanel( )->GetName( );
                this->GetModuleChooseGUI()->GetModulesMenuButton()->GetMenu( )->AddRadioButton( mName );
                m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            }
        }
        //--- TODO: make the initial value be module user sets as "home"
        this->GetModuleChooseGUI()->GetModulesMenuButton()->SetValue ("Volumes");
    }

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
        
        if ( this->MainSlicerWin != NULL ) {

            this->MainSlicerWin->GetMainPanelFrame()->SetWidth ( layout->GetDefaultGUIPanelWidth() );
            this->MainSlicerWin->GetMainPanelFrame()->SetHeight ( layout->GetDefaultGUIPanelHeight() );
            this->MainSlicerWin->GetMainPanelFrame()->SetReliefToSunken();

            this->LogoFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->LogoFrame->Create( );
            this->LogoFrame->SetHeight ( layout->GetDefaultLogoFrameHeight ( ) );

            this->ModuleChooseFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ModuleChooseFrame->Create( );
            this->ModuleChooseFrame->SetHeight ( layout->GetDefaultModuleChooseFrameHeight ( ) );

            this->SlicesControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SlicesControlFrame->Create( );
            this->SlicesControlFrame->SetHeight ( layout->GetDefaultSlicesControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ViewControlFrame->Create( );
            this->ViewControlFrame->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );
            
            // pack logo and slicer control frames
            this->Script ( "pack %s -side top -fill x -padx 1 -pady 1", this->LogoFrame->GetWidgetName() );
            app->Script ( "pack %s -side top -fill x -padx 1 -pady 10", this->ModuleChooseFrame->GetWidgetName() );

            // pack slice and view control frames
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -pady 10", this->ViewControlFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -pady 10", this->SlicesControlFrame->GetWidgetName() );

        }
    }

}


