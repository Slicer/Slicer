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
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScale.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWToolbar.h"

#include "vtkKWSplitFrame.h"
#include "vtkKWUserInterfaceManagerNotebook.h"

#include "vtkSlicerWindow.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerToolbarIcons.h"
#include "vtkSlicerLogoIcons.h"
#include "vtkSlicerModuleNavigationIcons.h"
#include "vtkSlicerViewControlIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
    //---  
    // widgets used in the Slice module
    //---

    //--- slicer main window
    this->MainSlicerWin = vtkSlicerWindow::New ( );

    //--- slicer toolbars
    vtkKWToolbar::SetGlobalToolbarAspectToUnChanged();
    vtkKWToolbar::SetGlobalWidgetsAspectToUnChanged();

    this->ModulesToolbar = vtkKWToolbar::New ( );
    this->LoadSaveToolbar = vtkKWToolbar::New ( );
    this->ViewToolbar = vtkKWToolbar::New ( );
    this->MouseModeToolbar = vtkKWToolbar::New ( );
    
    //--- slicer icons
    this->SlicerLogoIcons = vtkSlicerLogoIcons::New ();
    this->SlicerViewControlIcons = vtkSlicerViewControlIcons::New();
    this->SlicerToolbarIcons = vtkSlicerToolbarIcons::New ();
    this->SlicerModuleNavigationIcons = vtkSlicerModuleNavigationIcons::New ();

    //--- logo widgets to which icons are assigned.
    this->SlicerLogoLabel = vtkKWLabel::New();

    //--- toolbar widgets to which icons are assigned.
    this->HomeIconButton = vtkKWPushButton::New ( );
    this->DataIconButton = vtkKWPushButton::New ( );
    this->VolumeIconButton = vtkKWPushButton::New ( );
    this->ModelIconButton = vtkKWPushButton::New ( );
    this->EditorIconButton = vtkKWPushButton::New ( );
    this->EditorToolboxIconButton = vtkKWPushButton::New ( );
    this->ColorIconButton = vtkKWPushButton::New ( );
    this->FiducialsIconButton = vtkKWPushButton::New ( );
    this->MeasurementsIconButton = vtkKWPushButton::New ( );
    this->TransformIconButton = vtkKWPushButton::New ( );
    this->SaveSceneIconButton = vtkKWPushButton::New ( );
    this->LoadSceneIconButton = vtkKWPushButton::New ( );
    this->ConventionalViewIconButton = vtkKWPushButton::New ( );
    this->OneUp3DViewIconButton = vtkKWPushButton::New ( );
    this->OneUpSliceViewIconButton = vtkKWPushButton::New ( );
    this->FourUpViewIconButton = vtkKWPushButton::New ( );
    this->Tabbed3DViewIconButton = vtkKWPushButton::New ( );
    this->TabbedSliceViewIconButton = vtkKWPushButton::New ( );
    this->LightBoxViewIconButton = vtkKWPushButton::New ( );
    this->MousePickIconButton = vtkKWPushButton::New ( );
    this->MousePlaceFiducialIconButton = vtkKWPushButton::New ( );
    this->MousePanIconButton = vtkKWPushButton::New ( );
    this->MouseRotateIconButton = vtkKWPushButton::New ( );
    this->MouseZoomIconButton = vtkKWPushButton::New ( );
    
    // Control frames that comprise the Main Slicer GUI
    this->LogoFrame = vtkKWFrame::New();
    this->ModuleChooseFrame = vtkKWFrame::New();
    this->SliceControlFrame = vtkKWFrame::New();    
    this->ViewControlFrame = vtkKWFrame::New();    
    //    this->DefaultSlice0Frame = vtkKWFrame::New ();
    //    this->DefaultSlice1Frame = vtkKWFrame::New ();
    //    this->DefaultSlice2Frame = vtkKWFrame::New ();

    //--- ui for the ModuleChooseFrame,
    this->ModulesMenuButton = vtkKWMenuButton::New();
    this->ModulesLabel = vtkKWLabel::New();
    this->ModulesPrev = vtkKWPushButton::New ( );
    this->ModulesNext = vtkKWPushButton::New ( );
    this->ModulesHistory = vtkKWPushButton::New ( );
    this->ModulesRefresh = vtkKWPushButton::New ( );
    
    //--- ui for the SliceControlframe.
    this->ToggleAnnotationButton = vtkKWPushButton::New ( );
    this->ToggleFgBgButton = vtkKWPushButton::New ( );
    this->SliceFadeScale = vtkKWScale::New ( );
    this->SliceOpacityScale = vtkKWScale::New ( );
    
    //--- ui for the ViewControlFrame
    this->SpinButton = vtkKWCheckButton::New ( );
    this->RockButton = vtkKWCheckButton::New ( );
    this->OrthoButton = vtkKWCheckButton::New ( );
    this->CenterButton = vtkKWPushButton::New ( );
    this->SelectButton = vtkKWMenuButton::New ( );
    this->FOVEntry = vtkKWEntryWithLabel::New ( );

    //--- ui for the ViewControlFrame
    this->RotateAroundAIconButton = vtkKWLabel::New ( );
    this->RotateAroundPIconButton = vtkKWLabel::New ( );
    this->RotateAroundRIconButton = vtkKWLabel::New ( );
    this->RotateAroundLIconButton = vtkKWLabel::New ( );
    this->RotateAroundSIconButton = vtkKWLabel::New ( );
    this->RotateAroundIIconButton = vtkKWLabel::New ( );
    this->RotateAroundMiddleIconButton = vtkKWLabel::New ( );
    this->RotateAroundTopCornerIconButton = vtkKWLabel::New ( );
    this->RotateAroundBottomCornerIconButton = vtkKWLabel::New ( );

    this->LookFromAIconButton = vtkKWLabel::New ( );
    this->LookFromPIconButton = vtkKWLabel::New ( );
    this->LookFromRIconButton = vtkKWLabel::New ( );
    this->LookFromLIconButton = vtkKWLabel::New ( );
    this->LookFromSIconButton = vtkKWLabel::New ( );
    this->LookFromIIconButton = vtkKWLabel::New ( );
    this->LookFromMiddleIconButton = vtkKWLabel::New ( );
    this->LookFromTopCornerIconButton = vtkKWLabel::New ( );
    this->LookFromBottomCornerIconButton = vtkKWLabel::New ( );

    this->NavZoomInIconButton = vtkKWPushButton::New ( );
    this->NavZoomOutIconButton = vtkKWPushButton::New ( );
    this->NavZoomScale = vtkKWScale::New ( );
    
    //--- main viewer and 3 main slice views
    this->ViewerWidget = NULL;
    this->MainSliceGUI0 = NULL;
    this->MainSliceGUI1 = NULL;
    this->MainSliceGUI2 = NULL;

    //--- save the main slice logic in these.
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;
    this->SliceGUICollection = NULL;
    this->PlaneWidget = NULL;
    this->LightboxFrame = NULL;

    this->LoadSceneDialog = vtkKWLoadSaveDialog::New();
    this->SaveSceneDialog = vtkKWLoadSaveDialog::New();   

    //--- unique tag used to mark all view notebook pages
    //--- so that they can be identified and deleted when 
    //--- viewer is reformatted.
    this->ViewerPageTag = 1999;
}



//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( )
{


    if ( this->SlicerLogoIcons ) {
        this->SlicerLogoIcons->Delete ( );
        this->SlicerLogoIcons = NULL;
    }
    if ( this->SlicerViewControlIcons ) {
        this->SlicerViewControlIcons->Delete ( );
        this->SlicerViewControlIcons = NULL;
    }
    if ( this->SlicerToolbarIcons ) {
        this->SlicerToolbarIcons->Delete ( );
        this->SlicerToolbarIcons = NULL;
    }
    if ( this->SlicerModuleNavigationIcons ) {
        this->SlicerModuleNavigationIcons->Delete ( );
        this->SlicerModuleNavigationIcons = NULL;
    }

    this->DeleteGUIPanelWidgets ( );
    this->DeleteToolbarWidgets ( );

    vtkSlicerWindow *win = this->MainSlicerWin;
    if ( win ) {
        vtkKWToolbarSet *tbs = win->GetMainToolbarSet();
        if (tbs ) {
            tbs->RemoveAllToolbars () ;
        }
    }
    if ( this->ModulesToolbar ) {
        this->ModulesToolbar->Delete ( );
        this->ModulesToolbar = NULL;
    }
    if ( this->LoadSaveToolbar ) {
        this->LoadSaveToolbar->Delete ( );
        this->LoadSaveToolbar = NULL;
    }
    if ( this->ViewToolbar ) {
        this->ViewToolbar->Delete ( );
        this->ViewToolbar = NULL;
    }
    if ( this->MouseModeToolbar ) {
        this->MouseModeToolbar->Delete ( );
        this->MouseModeToolbar = NULL;
    }

    if ( this->SliceGUICollection )
      {
        this->SetSliceGUICollection ( NULL );
      }

    this->DestroyMain3DViewer ( );
    this->DestroyMainSliceViewers ( );
    this->DeleteFrames ( );

    if ( this->LoadSceneDialog ) {
        this->LoadSceneDialog->Delete();
        this->LoadSceneDialog = NULL;
    }
    if ( this->SaveSceneDialog ) {
        this->SaveSceneDialog->Delete();
        this->SaveSceneDialog = NULL;
    }
    if ( this->MainSlicerWin ) {
        this->MainSlicerWin->Delete ( );
        this->MainSlicerWin = NULL;
    }
    this->MainSliceLogic0 = NULL;
    this->MainSliceLogic1 = NULL;
    this->MainSliceLogic2 = NULL;
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

    // If a file has been selected for loading...
    char *fileName = this->SaveSceneDialog->GetFileName();
    if ( fileName ) 
      {
        if (this->GetMRMLScene()) 
          {
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

    // add observer onto the menubutton in the SlicerControl frame
  this->ModulesMenuButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    
    // add observers onto the module icon buttons 
    this->HomeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    // view configuration icon button observers...
    this->ConventionalViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUp3DViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUpSliceViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FourUpViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Tabbed3DViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->TabbedSliceViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LightBoxViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->GetMainSlicerWin()->GetFileMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
    this->LoadSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SaveSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );

    this->ToggleFgBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->SliceOpacityScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SliceOpacityScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( )
{
  this->ModulesMenuButton->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->HomeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ConventionalViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUp3DViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUpSliceViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FourUpViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Tabbed3DViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->TabbedSliceViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LightBoxViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->LoadSceneDialog->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *) this->GUICallbackCommand );
    this->SaveSceneDialog->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *) this->GUICallbackCommand );
    this->GetMainSlicerWin()->GetFileMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
    this->SliceOpacityScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SliceOpacityScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
    this->ToggleFgBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
    vtkSlicerModuleGUI * m;
    const char *mName;
    vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
    vtkKWMenuButton *menub = vtkKWMenuButton::SafeDownCast (caller );
    vtkKWMenu *menu = vtkKWMenu::SafeDownCast (caller );
    vtkKWLoadSaveDialog *filebrowse = vtkKWLoadSaveDialog::SafeDownCast(caller);
    vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
    vtkSlicerGUILayout *layout = app->GetMainLayout ( );
        
    // Process events from top row of buttons
    // For now, Home button takes us to the Volumes module.
    if ( pushb == this->HomeIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(
                                                                   this->GetApplication())->GetModuleGUIByName("Volumes");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Volumes" );
      }
    else if (pushb == this->DataIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Data");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Data" );
      }
    else if (pushb == this->VolumeIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Volumes" );
      }
    else if (pushb == this->ModelIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Models");
        if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Models" );
      }
    else if (pushb == this->TransformIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        //vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Transformments");
        //if ( m != NULL ) { m->GetUIPanel()->Raise(); }
        this->ModulesMenuButton->SetValue ( "Transform" );
      }
    else if ( pushb == this->ConventionalViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {

        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutDefaultView );
        this->AddMainSliceViewersToCollection ( );
      }
    else if ( pushb == this->OneUp3DViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutOneUp3DView );
        this->AddMainSliceViewersToCollection ( );
      }
    else if ( pushb == this->OneUpSliceViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutOneUpSliceView );
        this->AddMainSliceViewersToCollection ( );
      }
    else if ( pushb == this->FourUpViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutFourUpView );
        this->AddMainSliceViewersToCollection ( );
      }
    else if ( pushb == this->Tabbed3DViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutTabbed3DView );
        this->AddMainSliceViewersToCollection ( );
      }
    else if ( pushb == this->TabbedSliceViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        // TODO: fix this. 
        //        this->RemoveMainSliceViewersFromCollection ( );            
        //        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutTabbedSliceView );
        //        this->AddMainSliceViewersToCollection ( );
      }

    else if ( pushb == this->LightBoxViewIconButton && event == vtkKWPushButton::InvokedEvent )
      {
        // TODO: implement this
        /*
        this->RemoveMainSliceViewersFromCollection ( );            
        this->BuildMainViewer ( vtkSlicerGUILayout::SlicerLayoutLightboxView );
        this->AddMainSliceViewersToCollection ( );
        */
      }
    else if (menu == this->GetMainSlicerWin()->GetFileMenu() && event == vtkKWMenu::MenuItemInvokedEvent)
    {
      int index = (int) (*((int *)callData));
      if (index == 2)
        {
          // use command directly instead of this
          //this->ProcessLoadSceneCommand()
        }
      else if (index == 3)
        {
          // use command directly instead of this
          //this->ProcessSaveSceneCommand()
        }
    }

    //--- Process events from menubutton
    //--- TODO: change the Logic's "active module" and raise the appropriate UIPanel.
    //    if ( menub == this->ModulesMenuButton && event == vtkCommand::ModifiedEvent )
    if ( menub == this->ModulesMenuButton && event == vtkCommand::ModifiedEvent )
        {
            if ( app->GetModuleGUICollection ( ) != NULL )
                {
                    app->GetModuleGUICollection( )->InitTraversal( );
                    m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
                    while (m != NULL )
                        {
                            mName = m->GetUIPanel()->GetName();
                            if ( !strcmp (this->ModulesMenuButton->GetValue(), mName) ) {
                                m->GetUIPanel()->Raise();
                                break;
                            }
                            m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
                        }
                    //this->ModulesMenuButton->SetValue ( "Modules" );
                }
        }

    // Process the Fade scale and button
    // -- set save state when manipulation starts
    // -- toggle the value if needed
    // -- adjust the Opacity of every composite node on every event
    if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueStartChangingEvent ||
         pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent )
      {
      if (this->GetMRMLScene()) 
        {
        this->GetMRMLScene()->SaveStateForUndo();
        }
      }

    if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueChangingEvent ||
         pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent )
      {

      if ( pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ) 
        {
        this->SliceFadeScale->SetValue( 1.0 - this->SliceFadeScale->GetValue() );
        }

      int i, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
      vtkMRMLSliceCompositeNode *cnode;
      for (i = 0; i < nnodes; i++)
        {
        cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                this->MRMLScene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
        cnode->SetForegroundOpacity( this->SliceFadeScale->GetValue() );
        }
      }

    // Process the label Opacity scale 
    // -- set save state when manipulation starts
    // -- adjust the Opacity of every composite node on every event
    if ( scale == this->SliceOpacityScale && event == vtkKWScale::ScaleValueStartChangingEvent )
      {
      if (this->GetMRMLScene()) 
        {
        this->GetMRMLScene()->SaveStateForUndo();
        }
      }

    if ( scale == this->SliceOpacityScale && event == vtkKWScale::ScaleValueChangingEvent )
      {

      int i, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
      vtkMRMLSliceCompositeNode *cnode;
      for (i = 0; i < nnodes; i++)
        {
        cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                this->MRMLScene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
        cnode->SetLabelOpacity( this->SliceOpacityScale->GetValue() );
        }
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

            this->MainSlicerWin->Create ( );        

            // configure initial GUI layout
            layout->InitializeMainSlicerWindowSize ( );
            layout->ConfigureMainSlicerWindowPanels ( );

            // Build main GUI and components
            this->BuildToolBar();
            this->BuildGUIPanel ( );
            this->BuildLogoGUIPanel ( );
            this->BuildModuleChooseGUIPanel ( );
            this->BuildSliceControlGUIPanel ( );
            this->BuildViewControlGUIPanel ( );

            // Turn off the tabs for pages in the ModuleControlGUI
            this->MainSlicerWin->GetMainNotebook()->ShowIconsOff ( );
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
          this->MainSliceLogic0->Register ( this );
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
          this->MainSliceLogic1->Register ( this );
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
          this->MainSliceLogic2->Register ( this );
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
void vtkSlicerApplicationGUI::DeleteToolbarWidgets ( )
{

    if ( this->ModulesToolbar ) {
        this->ModulesToolbar->RemoveAllWidgets( );
    }
    if ( this->LoadSaveToolbar ) {
        this->LoadSaveToolbar->RemoveAllWidgets ( );
    }
    if ( this->ViewToolbar ) {
        this->ViewToolbar->RemoveAllWidgets ( );
    }

    if ( this->HomeIconButton ) {
        this->HomeIconButton->Delete ( );
        this->HomeIconButton = NULL;
    }
    if ( this->DataIconButton ) {
        this->DataIconButton->Delete ( );
        this->DataIconButton = NULL;
    }
    if ( this->VolumeIconButton ) {
        this->VolumeIconButton->Delete ( );
        this->VolumeIconButton = NULL;
    }
    if ( this->ModelIconButton ) {
        this->ModelIconButton->Delete ( );
        this->ModelIconButton = NULL;
    }
    if ( this->EditorIconButton ) {
        this->EditorIconButton->Delete ( );
        this->EditorIconButton = NULL;
    }
    if ( this->EditorToolboxIconButton ) {
        this->EditorToolboxIconButton->Delete ( );
        this->EditorToolboxIconButton = NULL;
    }
    if ( this->TransformIconButton ) {
        this->TransformIconButton->Delete ( );
        this->TransformIconButton = NULL;
    }
    if ( this->ColorIconButton ) {
        this->ColorIconButton->Delete ( );
        this->ColorIconButton = NULL;
    }
    if ( this->FiducialsIconButton ) {
        this->FiducialsIconButton->Delete ( );
        this->FiducialsIconButton = NULL;
    }
    if ( this->MeasurementsIconButton ) {
        this->MeasurementsIconButton->Delete ( );
        this->MeasurementsIconButton = NULL;
    }
    if ( this->SaveSceneIconButton ) {
        this->SaveSceneIconButton->Delete ( );
        this->SaveSceneIconButton = NULL;
    }
    if ( this->LoadSceneIconButton ) {
        this->LoadSceneIconButton->Delete ( );
        this->LoadSceneIconButton = NULL;
    }
    if ( this->ConventionalViewIconButton ) {
        this->ConventionalViewIconButton->Delete ( );
        this->ConventionalViewIconButton = NULL;
    }
    if ( this->OneUp3DViewIconButton ) {
        this->OneUp3DViewIconButton->Delete ( );
        this->OneUp3DViewIconButton = NULL;
    }
    if ( this->OneUpSliceViewIconButton ) {
        this->OneUpSliceViewIconButton->Delete ( );
        this->OneUpSliceViewIconButton = NULL;
    }
    if ( this->FourUpViewIconButton ) {
        this->FourUpViewIconButton->Delete ( );
        this->FourUpViewIconButton = NULL;
    }
    if ( this->Tabbed3DViewIconButton ) {
        this->Tabbed3DViewIconButton->Delete ( );
        this->Tabbed3DViewIconButton = NULL;
    }
    if ( this->TabbedSliceViewIconButton ) {
        this->TabbedSliceViewIconButton->Delete ( );
        this->TabbedSliceViewIconButton = NULL;
    }
    if ( this->LightBoxViewIconButton ) {
        this->LightBoxViewIconButton->Delete ( );
        this->LightBoxViewIconButton = NULL;
    }
    if ( this->MousePickIconButton ) {
        this->MousePickIconButton->Delete ( );
        this->MousePickIconButton = NULL;
    }
    if ( this->MousePlaceFiducialIconButton ) {
        this->MousePlaceFiducialIconButton->Delete ( );
        this->MousePlaceFiducialIconButton = NULL;
    }
    if ( this->MousePanIconButton ) {
        this->MousePanIconButton->Delete ( );
        this->MousePanIconButton = NULL;
    }
    if ( this->MouseRotateIconButton ) {
        this->MouseRotateIconButton->Delete ( );
        this->MouseRotateIconButton = NULL;
    }
    if ( this->MouseZoomIconButton ) {
        this->MouseZoomIconButton->Delete ( );
        this->MouseZoomIconButton = NULL;
    }

    
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteGUIPanelWidgets ( )
{
    //--- widgets from the ModuleChooseFrame
    if ( this->ModulesMenuButton ) {
        this->ModulesMenuButton->Delete();
        this->ModulesMenuButton = NULL;
    }
    if ( this->ModulesLabel ) {
        this->ModulesLabel->Delete ( );
        this->ModulesLabel = NULL;
    }
    if ( this->ModulesPrev ) {
        this->ModulesPrev->Delete ( );
        this->ModulesPrev = NULL;
    }
    if ( this->ModulesNext ) {
        this->ModulesNext->Delete ( );
        this->ModulesNext = NULL;
    }
    if ( this->ModulesHistory) {
        this->ModulesHistory->Delete ( );
        this->ModulesHistory = NULL;
    }
    if ( this->ModulesRefresh) {
        this->ModulesRefresh->Delete ( );
        this->ModulesRefresh = NULL;
    }

    //--- widgets from ViewControlFrame
    if ( this->SpinButton ) {
        this->SpinButton->Delete();
        this->SpinButton = NULL;
    }
    if ( this->RockButton) {
        this->RockButton->Delete();
        this->RockButton = NULL;
    }
    if ( this->OrthoButton ) {
        this->OrthoButton->Delete();
        this->OrthoButton = NULL;
    }
    if ( this->CenterButton ) {
        this->CenterButton->Delete();
        this->CenterButton = NULL;
    }
    if ( this->SelectButton ) {
        this->SelectButton->Delete();
        this->SelectButton = NULL;
    }
    if ( this->FOVEntry ) {
        this->FOVEntry->Delete();
        this->FOVEntry= NULL;
    }

    //--- widgets from LogoFrame
    if (this->SlicerLogoLabel ) {
        this->SlicerLogoLabel->Delete();
        this->SlicerLogoLabel = NULL;
    }

    //--- widgets from the SliceControlFrame
    if ( this->ToggleAnnotationButton ) {
        this->ToggleAnnotationButton->Delete ( );
        this->ToggleAnnotationButton = NULL;
    }
    if ( this->ToggleFgBgButton ) {
        this->ToggleFgBgButton->Delete ( );
        this->ToggleFgBgButton = NULL;
    }
    if ( this->SliceFadeScale ) {
        this->SliceFadeScale->Delete ( );
        this->SliceFadeScale = NULL;
    }
    if ( this->SliceOpacityScale ) {
        this->SliceOpacityScale->Delete ( );
        this->SliceOpacityScale = NULL;
    }

    //--- widgets from the ViewControlFrame
    if ( this->RotateAroundAIconButton ) {
        this->RotateAroundAIconButton->Delete ( );
        this->RotateAroundAIconButton = NULL;
    }
    if ( this->RotateAroundPIconButton ) {
        this->RotateAroundPIconButton->Delete ( );
        this->RotateAroundPIconButton = NULL;
    }
    if ( this->RotateAroundRIconButton ) {
        this->RotateAroundRIconButton->Delete ( );
        this->RotateAroundRIconButton = NULL;
    }
    if ( this->RotateAroundLIconButton ) {
        this->RotateAroundLIconButton->Delete ( );
        this->RotateAroundLIconButton = NULL;
    }
    if ( this->RotateAroundSIconButton ) {
        this->RotateAroundSIconButton->Delete ( );
        this->RotateAroundSIconButton = NULL;
    }
    if ( this->RotateAroundIIconButton ) {
        this->RotateAroundIIconButton->Delete ( );
        this->RotateAroundIIconButton = NULL;
    }
    if ( this->RotateAroundMiddleIconButton ) {
        this->RotateAroundMiddleIconButton->Delete ( );
        this->RotateAroundMiddleIconButton = NULL;
    }
    if ( this->RotateAroundTopCornerIconButton ) {
        this->RotateAroundTopCornerIconButton->Delete ( );
        this->RotateAroundTopCornerIconButton = NULL;
    }
    if ( this->RotateAroundBottomCornerIconButton ) {
        this->RotateAroundBottomCornerIconButton->Delete ( );
        this->RotateAroundBottomCornerIconButton = NULL;
    }
    if ( this->LookFromAIconButton ) {
        this->LookFromAIconButton->Delete ( );
        this->LookFromAIconButton = NULL;
    }
    if ( this->LookFromPIconButton ) {
        this->LookFromPIconButton->Delete ( );
        this->LookFromPIconButton = NULL;
    }
    if ( this->LookFromRIconButton ) {
        this->LookFromRIconButton->Delete ( );
        this->LookFromRIconButton = NULL;
    }
    if ( this->LookFromLIconButton ) {
        this->LookFromLIconButton->Delete ( );
        this->LookFromLIconButton = NULL;
    }
    if ( this->LookFromSIconButton ) {
        this->LookFromSIconButton->Delete ( );
        this->LookFromSIconButton = NULL;
    }
    if ( this->LookFromIIconButton ) {
        this->LookFromIIconButton->Delete ( );
        this->LookFromIIconButton = NULL;
    }
    if ( this->LookFromMiddleIconButton ) {
        this->LookFromMiddleIconButton->Delete ( );
        this->LookFromMiddleIconButton = NULL;
    }
    if ( this->LookFromTopCornerIconButton ) {
        this->LookFromTopCornerIconButton->Delete ( );
        this->LookFromTopCornerIconButton = NULL;
    }
    if ( this->LookFromBottomCornerIconButton ) {
        this->LookFromBottomCornerIconButton->Delete ( );
        this->LookFromBottomCornerIconButton = NULL;
    }
    if ( this->NavZoomInIconButton ) {
        this->NavZoomInIconButton->Delete ( );
        this->NavZoomInIconButton = NULL;
    }
    if ( this->NavZoomOutIconButton ) {
        this->NavZoomOutIconButton->Delete ( );
        this->NavZoomOutIconButton = NULL;
    }
    if ( this->NavZoomScale ) {
        this->NavZoomScale->Delete ( );
        this->NavZoomScale = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DeleteFrames ( )
{
    if ( this->LogoFrame ) {
        this->LogoFrame->Delete ();
        this->LogoFrame = NULL;
    }
    if ( this->ModuleChooseFrame ) {
        this->ModuleChooseFrame->Delete ();
        this->ModuleChooseFrame = NULL;
    }
    if ( this->SliceControlFrame ) {
        this->SliceControlFrame->Delete ( );
        this->SliceControlFrame = NULL;
    }
    if ( this->ViewControlFrame ) {
        this->ViewControlFrame->Delete ( );
        this->ViewControlFrame = NULL;
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

      // add observers on GUI, MRML 
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

      // add observers on Logic
      if ( this->MainSliceLogic0 )
        {
          this->MainSliceGUI0->SetAndObserveModuleLogic ( this->MainSliceLogic0 );
          this->MainSliceLogic0->UnRegister ( this );
        }
      if (this->MainSliceLogic1 )
        {
          this->MainSliceGUI1->SetAndObserveModuleLogic ( this->MainSliceLogic1 );
          this->MainSliceLogic1->UnRegister ( this );
        }
      if (this->MainSliceLogic2 )
        {
          this->MainSliceGUI2->SetAndObserveModuleLogic ( this->MainSliceLogic2 );
          this->MainSliceLogic2->UnRegister ( this );
        }
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
          this->MainSlicerWin->GetMainNotebook()->RemovePagesMatchingTag(this->ViewerPageTag );      
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
      this->MainSlicerWin->GetMainNotebook()->AddPage("yellow slice", NULL, NULL, this->ViewerPageTag );
      this->MainSliceGUI1->BuildGUI ( this->MainSlicerWin->GetViewFrame ( ), color->SliceGUIYellow );
      this->MainSliceGUI1->PackGUI ( );
      // Green slice viewer          
      this->MainSlicerWin->GetMainNotebook()->AddPage("green slice", NULL, NULL, this->ViewerPageTag );
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
            // save the main slice logic to reassign when viewers are
            // destroyed and recreated during view layout changes.
            this->MainSliceLogic0 = this->MainSliceGUI0->GetLogic ( );
            this->MainSliceLogic0->Register( this );
            this->MainSliceLogic1 = this->MainSliceGUI1->GetLogic ( );
            this->MainSliceLogic1->Register( this );
            this->MainSliceLogic2 = this->MainSliceGUI2->GetLogic ( );
            this->MainSliceLogic2->Register( this );
          }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildToolBar()
{
    if ( this->GetApplication() != NULL ) {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

        //
        //--- toolbars
        //
        //--- configure the window's main toolbarset.
        vtkSlicerWindow *win = this->MainSlicerWin;
        vtkKWToolbarSet *tbs = win->GetMainToolbarSet();
        tbs->SetToolbarsWidgetsAspect ( vtkKWToolbar::WidgetsAspectUnChanged );
        tbs->BottomSeparatorVisibilityOn ( );
        tbs->TopSeparatorVisibilityOn ( );

        //--- configure toolbars
        vtkKWToolbar *mtb = this->GetModulesToolbar ( );
        mtb->SetParent ( tbs->GetToolbarsFrame ( ) );
        mtb->Create();
        mtb->SetWidgetsFlatAdditionalPadX ( 0 );
        mtb->SetWidgetsFlatAdditionalPadY ( 0 );
        mtb->ResizableOff ( );
        mtb->SetReliefToGroove ( );
        mtb->SetWidgetsPadX ( 3 );
        mtb->SetWidgetsPadY ( 2 );

        vtkKWToolbar *ltb = this->GetLoadSaveToolbar ( );
        ltb->SetParent ( tbs->GetToolbarsFrame ( ) );
        ltb->Create();
        ltb->SetWidgetsFlatAdditionalPadX ( 0 );
        ltb->SetWidgetsFlatAdditionalPadY ( 0 );
        ltb->ResizableOff ( );
        ltb->SetReliefToGroove ( );
        ltb->SetWidgetsPadX ( 3 );
        ltb->SetWidgetsPadY ( 2 );

        vtkKWToolbar *vtb = this->GetViewToolbar ( );
        vtb->SetParent ( tbs->GetToolbarsFrame ( ) );
        vtb->Create();
        vtb->SetWidgetsFlatAdditionalPadX ( 0 );
        vtb->SetWidgetsFlatAdditionalPadY ( 0 );
        vtb->ResizableOff ( );
        vtb->SetReliefToGroove ( );
        vtb->SetWidgetsPadX ( 3 );
        vtb->SetWidgetsPadY ( 2 );

        vtkKWToolbar *mmtb = this->GetMouseModeToolbar ( );
        mmtb->SetParent ( tbs->GetToolbarsFrame ( ) );
        mmtb->Create();
        mmtb->SetWidgetsFlatAdditionalPadX ( 0 );
        mmtb->SetWidgetsFlatAdditionalPadY ( 0 );
        mmtb->ResizableOff ( );
        mmtb->SetReliefToGroove ( );
        mmtb->SetWidgetsPadX ( 3 );
        mmtb->SetWidgetsPadY ( 2 );
        
        //--- and add toolbars to the window's main toolbar set.        
        tbs->AddToolbar ( this->GetLoadSaveToolbar() );
        tbs->AddToolbar ( this->GetModulesToolbar() );
        tbs->AddToolbar ( this->GetViewToolbar() );
        tbs->AddToolbar ( this->GetMouseModeToolbar() );
        
        //
        //--- create icons and the labels that display them and add to toolbar
        //
        // save scene icon
        this->SaveSceneIconButton->SetParent ( ltb->GetFrame ( ));
        this->SaveSceneIconButton->Create ( );
        this->SaveSceneIconButton->SetReliefToFlat ( );
        this->SaveSceneIconButton->SetBorderWidth ( 0 );
        this->SaveSceneIconButton->SetOverReliefToNone ( );
        this->SaveSceneIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSaveSceneIcon( ) );
        this->SaveSceneIconButton->SetBalloonHelpString ( "Save a MRML scene to a file.");
        ltb->AddWidget ( this->SaveSceneIconButton );

        // load scene icon
        this->LoadSceneIconButton->SetParent ( ltb->GetFrame ( ) );
        this->LoadSceneIconButton->Create();
        this->LoadSceneIconButton->SetReliefToFlat ( );
        this->LoadSceneIconButton->SetBorderWidth ( 0 );
        this->LoadSceneIconButton->SetOverReliefToNone ( );
        this->LoadSceneIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetLoadSceneIcon( ) );
        this->LoadSceneIconButton->SetBalloonHelpString ( "Load a MRML scene.");
        ltb->AddWidget ( this->LoadSceneIconButton );

        // home icon
        this->HomeIconButton->SetParent ( mtb->GetFrame ( ));
        this->HomeIconButton->Create ( );
        this->HomeIconButton->SetReliefToFlat ( );
        this->HomeIconButton->SetBorderWidth ( 0 );
        this->HomeIconButton->SetOverReliefToNone ( );
        this->HomeIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetHomeIcon( ) );
        this->HomeIconButton->SetBalloonHelpString ( "Home" );
        mtb->AddWidget ( this->HomeIconButton );

        // data module icon
        this->DataIconButton->SetParent ( mtb->GetFrame ( ));
        this->DataIconButton->Create ( );
        this->DataIconButton->SetReliefToFlat ( );
        this->DataIconButton->SetBorderWidth ( 0 );
        this->DataIconButton->SetOverReliefToNone ( );
        this->DataIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetDataIcon ( ) );
        this->DataIconButton->SetBalloonHelpString ( "Data");
        mtb->AddWidget ( this->DataIconButton );

        // volume module icon
        this->VolumeIconButton->SetParent ( mtb->GetFrame ( ));
        this->VolumeIconButton->Create ( );
        this->VolumeIconButton->SetReliefToFlat ( );
        this->VolumeIconButton->SetBorderWidth ( 0 );
        this->VolumeIconButton->SetOverReliefToNone ( );
        this->VolumeIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetVolumeIcon ( ));
        this->VolumeIconButton->SetBalloonHelpString ( "Volumes");
        mtb->AddWidget ( this->VolumeIconButton );

        // models module icon
        this->ModelIconButton->SetParent (mtb->GetFrame ( ) );
        this->ModelIconButton->Create ( );
        this->ModelIconButton->SetReliefToFlat ( );
        this->ModelIconButton->SetBorderWidth ( 0 );
        this->ModelIconButton->SetOverReliefToNone ( );
        this->ModelIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetModelIcon ( ) );
        this->ModelIconButton->SetBalloonHelpString ( "Models");
        mtb->AddWidget ( this->ModelIconButton );

        // transforms module icon
        this->TransformIconButton->SetParent ( mtb->GetFrame ( ) );
        this->TransformIconButton->Create ( );
        this->TransformIconButton->SetReliefToFlat ( );
        this->TransformIconButton->SetBorderWidth ( 0 );
        this->TransformIconButton->SetOverReliefToNone ( );
        this->TransformIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTransformIcon ( ) );
        this->TransformIconButton->SetBalloonHelpString ( "Transforms");
        mtb->AddWidget ( this->TransformIconButton );

        // fiducial utility icon
        this->FiducialsIconButton->SetParent ( mtb->GetFrame ( ) );
        this->FiducialsIconButton->Create ( );
        this->FiducialsIconButton->SetReliefToFlat ( );
        this->FiducialsIconButton->SetBorderWidth ( 0 );
        this->FiducialsIconButton->SetOverReliefToNone ( );
        this->FiducialsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetFiducialsIcon ( ) );
        this->FiducialsIconButton->SetBalloonHelpString ( "Fiducials");
        mtb->AddWidget ( this->FiducialsIconButton );

        // editor module icon
        this->EditorToolboxIconButton->SetParent ( mtb->GetFrame ( ) );
        this->EditorToolboxIconButton->Create ( );
        this->EditorToolboxIconButton->SetReliefToFlat ( );
        this->EditorToolboxIconButton->SetBorderWidth ( 0 );
        this->EditorToolboxIconButton->SetOverReliefToNone ( );
        this->EditorToolboxIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetEditorToolboxIcon ( ) );
        this->EditorToolboxIconButton->SetBalloonHelpString ( "Editor Toolbox");        
        mtb->AddWidget ( this->EditorToolboxIconButton );
        // editor module icon
        this->EditorIconButton->SetParent ( mtb->GetFrame ( ) );
        this->EditorIconButton->Create ( );
        this->EditorIconButton->SetReliefToFlat ( );
        this->EditorIconButton->SetBorderWidth ( 0 );
        this->EditorIconButton->SetOverReliefToNone ( );
        this->EditorIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetEditorIcon ( ) );
        this->EditorIconButton->SetBalloonHelpString ( "Editor");        
        mtb->AddWidget ( this->EditorIconButton );

        // measurements module icon
        this->MeasurementsIconButton->SetParent ( mtb->GetFrame ( ) );
        this->MeasurementsIconButton->Create ( );
        this->MeasurementsIconButton->SetReliefToFlat ( );
        this->MeasurementsIconButton->SetBorderWidth ( 0 );
        this->MeasurementsIconButton->SetOverReliefToNone ( );
        this->MeasurementsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMeasurementsIcon ( ) );
        this->MeasurementsIconButton->SetBalloonHelpString ( "Measurements");        
        mtb->AddWidget ( this->MeasurementsIconButton );

        // color utility icon
        this->ColorIconButton->SetParent ( mtb->GetFrame ( ) );
        this->ColorIconButton->Create ( );
        this->ColorIconButton->SetReliefToFlat ( );
        this->ColorIconButton->SetBorderWidth ( 0 );
        this->ColorIconButton->SetOverReliefToNone ( );
        this->ColorIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetColorIcon ( ) );
        this->ColorIconButton->SetBalloonHelpString ( "Colors");
        mtb->AddWidget ( this->ColorIconButton );

        // conventional view icon
        this->ConventionalViewIconButton->SetParent (vtb->GetFrame ( ) );
        this->ConventionalViewIconButton->Create ( );
        this->ConventionalViewIconButton->SetReliefToFlat ( );
        this->ConventionalViewIconButton->SetBorderWidth ( 0 );
        this->ConventionalViewIconButton->SetOverReliefToNone ( );
        this->ConventionalViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetConventionalViewIcon ( ) );        
        this->ConventionalViewIconButton->SetBalloonHelpString ("Display the 3D viewer over 3 slice windows");
        vtb->AddWidget ( this->ConventionalViewIconButton );
        // 3Dview-only icon
        this->OneUp3DViewIconButton->SetParent ( vtb->GetFrame ( ) );
        this->OneUp3DViewIconButton->Create ( );
        this->OneUp3DViewIconButton->SetReliefToFlat ( );
        this->OneUp3DViewIconButton->SetBorderWidth ( 0 );
        this->OneUp3DViewIconButton->SetOverReliefToNone ( );
        this->OneUp3DViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetOneUp3DViewIcon ( ) );
        this->OneUp3DViewIconButton->SetBalloonHelpString ( "Display the 3D viewer without any slice windows" );
        vtb->AddWidget (this->OneUp3DViewIconButton );

        // Slice view-only icon
        this->OneUpSliceViewIconButton->SetParent ( vtb->GetFrame ( ) );
        this->OneUpSliceViewIconButton->Create ( );
        this->OneUpSliceViewIconButton->SetReliefToFlat ( );
        this->OneUpSliceViewIconButton->SetBorderWidth ( 0 );
        this->OneUpSliceViewIconButton->SetOverReliefToNone ( );
        this->OneUpSliceViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetOneUpSliceViewIcon ( ) );
        this->OneUpSliceViewIconButton->SetBalloonHelpString ( "Display one slice window with no 3D viewer" );
        vtb->AddWidget (this->OneUpSliceViewIconButton );

        // 4 equal windows icon
        this->FourUpViewIconButton->SetParent ( vtb->GetFrame ( ) );
        this->FourUpViewIconButton->Create ( );
        this->FourUpViewIconButton->SetReliefToFlat ( );
        this->FourUpViewIconButton->SetBorderWidth ( 0 );
        this->FourUpViewIconButton->SetOverReliefToNone ( );
        this->FourUpViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetFourUpViewIcon ( ) );
        this->FourUpViewIconButton->SetBalloonHelpString ( "Display the 3D viewer and 3 slice windows in a matrix" );
        vtb->AddWidget ( this->FourUpViewIconButton );

        // tabbed view icon
        this->TabbedSliceViewIconButton->SetParent ( vtb->GetFrame ( ) );
        this->TabbedSliceViewIconButton->Create ( );
        this->TabbedSliceViewIconButton->SetReliefToFlat ( );
        this->TabbedSliceViewIconButton->SetBorderWidth ( 0 );
        this->TabbedSliceViewIconButton->SetOverReliefToNone ( );
        this->TabbedSliceViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTabbedSliceViewIcon ( ) );
        this->TabbedSliceViewIconButton->SetBalloonHelpString ( "Display a collection of slices in a notebook" );
        vtb->AddWidget ( this->TabbedSliceViewIconButton );

        // tabbed view icon
        this->Tabbed3DViewIconButton->SetParent ( vtb->GetFrame ( ) );
        this->Tabbed3DViewIconButton->Create ( );
        this->Tabbed3DViewIconButton->SetReliefToFlat ( );
        this->Tabbed3DViewIconButton->SetBorderWidth ( 0 );
        this->Tabbed3DViewIconButton->SetOverReliefToNone ( );
        this->Tabbed3DViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTabbed3DViewIcon ( ) );
        this->Tabbed3DViewIconButton->SetBalloonHelpString ( "Display a collection of scenes in a notebook" );
        vtb->AddWidget ( this->Tabbed3DViewIconButton );

        // lightbox view icon
        this->LightBoxViewIconButton->SetParent ( vtb->GetFrame ( ));
        this->LightBoxViewIconButton->Create ( );
        this->LightBoxViewIconButton->SetReliefToFlat ( );
        this->LightBoxViewIconButton->SetBorderWidth ( 0 );
        this->LightBoxViewIconButton->SetOverReliefToNone ( );
        this->LightBoxViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetLightBoxViewIcon( ) );
        this->LightBoxViewIconButton->SetBalloonHelpString ( "Display a slice-matrix and no 3D view" );
        vtb->AddWidget ( this->LightBoxViewIconButton );

        // mouse mode icons; mouse pick icon
        this->MousePickIconButton->SetParent (mmtb->GetFrame ( ));
        this->MousePickIconButton->Create ( );
        this->MousePickIconButton->SetReliefToFlat ( );
        this->MousePickIconButton->SetBorderWidth ( 0 );
        this->MousePickIconButton->SetOverReliefToNone ( );
        this->MousePickIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePickIcon( ) );
        this->MousePickIconButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'pick'" );
        mmtb->AddWidget ( this->MousePickIconButton );

        // mouse mode icons; mouse pan icon
        this->MousePanIconButton->SetParent (mmtb->GetFrame ( ));
        this->MousePanIconButton->Create ( );
        this->MousePanIconButton->SetReliefToFlat ( );
        this->MousePanIconButton->SetBorderWidth ( 0 );
        this->MousePanIconButton->SetOverReliefToNone ( );
        this->MousePanIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePanIcon( ) );
        this->MousePanIconButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'pan' " );
        mmtb->AddWidget ( this->MousePanIconButton );

        // mouse mode icons; mouse rotate icon
        this->MouseRotateIconButton->SetParent (mmtb->GetFrame ( ));
        this->MouseRotateIconButton->Create ( );
        this->MouseRotateIconButton->SetReliefToFlat ( );
        this->MouseRotateIconButton->SetBorderWidth ( 0 );
        this->MouseRotateIconButton->SetOverReliefToNone ( );
        this->MouseRotateIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMouseRotateIcon( ) );
        this->MouseRotateIconButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'rotate' " );
        mmtb->AddWidget ( this->MouseRotateIconButton );

        // mouse mode icons; mouse zoom  icon
        this->MouseZoomIconButton->SetParent (mmtb->GetFrame ( ));
        this->MouseZoomIconButton->Create ( );
        this->MouseZoomIconButton->SetReliefToFlat ( );
        this->MouseZoomIconButton->SetBorderWidth ( 0 );
        this->MouseZoomIconButton->SetOverReliefToNone ( );
        this->MouseZoomIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMouseZoomIcon( ) );
        this->MouseZoomIconButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'zoom' " );
        mmtb->AddWidget ( this->MouseZoomIconButton );

        // mouse mode icons; mouse pick icon
        this->MousePlaceFiducialIconButton->SetParent (mmtb->GetFrame ( ));
        this->MousePlaceFiducialIconButton->Create ( );
        this->MousePlaceFiducialIconButton->SetReliefToFlat ( );
        this->MousePlaceFiducialIconButton->SetBorderWidth ( 0 );
        this->MousePlaceFiducialIconButton->SetOverReliefToNone ( );
        this->MousePlaceFiducialIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceFiducialIcon( ) );
        this->MousePlaceFiducialIconButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'place fiducials'" );
        mmtb->AddWidget ( this->MousePlaceFiducialIconButton );

        tbs->ShowToolbar ( this->GetModulesToolbar ( ));
        tbs->ShowToolbar ( this->GetLoadSaveToolbar ( ));
        tbs->ShowToolbar ( this->GetViewToolbar ( ));
        tbs->ShowToolbar ( this->GetMouseModeToolbar ( ));
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildLogoGUIPanel ( )
{
    if ( this->GetApplication( )  != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication () );
        this->SlicerLogoLabel->SetParent ( this->LogoFrame );
        this->SlicerLogoLabel->Create();
        this->SlicerLogoLabel->SetImageToIcon ( this->SlicerLogoIcons->GetSlicerLogo() );
        this->SlicerLogoLabel->SetBalloonHelpString ("placeholder logo");
        app->Script ( "pack %s -side top -anchor w -padx 2 -pady 0", this->SlicerLogoLabel->GetWidgetName( ) );        
    }
    
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
                this->ModulesMenuButton->GetMenu( )->AddRadioButton( mName );
                m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            }
        }
        //--- TODO: make the initial value be module user sets as "home"
        this->ModulesMenuButton->SetValue ("Volumes");
    }

}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildModuleChooseGUIPanel ( )
{
    
    if ( this->GetApplication( )  != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        
        //--- ALL modules menu button label
        this->ModulesLabel->SetParent ( this->ModuleChooseFrame );
        this->ModulesLabel->Create ( );

        this->ModulesLabel->SetText ( "Modules:");
        this->ModulesLabel->SetAnchorToWest ( );
        this->ModulesLabel->SetWidth ( 7 );

        //--- All modules menu button
        this->ModulesMenuButton->SetParent ( this->ModuleChooseFrame );
        this->ModulesMenuButton->Create ( );
        this->ModulesMenuButton->SetWidth ( 28 );
        this->ModulesMenuButton->IndicatorVisibilityOn ( );
        this->ModulesMenuButton->SetBalloonHelpString ("Select a Slicer module.");

        //--- Next and previous module button
        this->ModulesNext->SetParent ( this->ModuleChooseFrame );
        this->ModulesNext->Create ( );
        this->ModulesNext->SetBorderWidth ( 0 );
        this->ModulesNext->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleNextIcon() );
        this->ModulesNext->SetBalloonHelpString ("Navigate to the next module in your use history.");

        this->ModulesPrev->SetParent ( this->ModuleChooseFrame );
        this->ModulesPrev->Create ( );
        this->ModulesPrev->SetBorderWidth ( 0 );
        this->ModulesPrev->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModulePrevIcon() );
        this->ModulesPrev->SetBalloonHelpString ("Navigate to the previous module in your use history.");
        
        this->ModulesHistory->SetParent ( this->ModuleChooseFrame );
        this->ModulesHistory->Create ( );
        this->ModulesHistory->SetBorderWidth ( 0 );
        this->ModulesHistory->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleHistoryIcon() );
        this->ModulesHistory->SetBalloonHelpString ("Pop up a window showing your module use history.");

        this->ModulesRefresh->SetParent ( this->ModuleChooseFrame );
        this->ModulesRefresh->Create ( );
        this->ModulesRefresh->SetBorderWidth ( 0 );
        this->ModulesRefresh->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleRefreshIcon() );
        this->ModulesRefresh->SetBalloonHelpString ("Refresh the list of available modules.");
        
        //--- pack everything up.
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 3", this->ModulesLabel->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 1 -ipady 0 -pady 2", this->ModulesMenuButton->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesPrev->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesNext->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesHistory->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesRefresh->GetWidgetName( ) );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildSliceControlGUIPanel ( )
{

    //--- Populate the Slice Control Frame

    if ( this->GetApplication( )  != NULL ) {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        this->SliceControlFrame->SetReliefToGroove();
        
        //--- create frames
        vtkKWFrame *f1 = vtkKWFrame::New ( );
        f1->SetParent ( this->SliceControlFrame );
        f1->Create ( );
        vtkKWFrame *f2 = vtkKWFrame::New ( );
        f2->SetParent ( this->SliceControlFrame );
        f2->Create ( );
        vtkKWFrame *f3 = vtkKWFrame::New ( );
        f3->SetParent ( this->SliceControlFrame );
        f3->Create ( );
        
        //--- pack everything up: buttons, labels, scales
        app->Script ( "pack %s -side left -anchor n -padx 0 -pady 5", f1->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 0 -pady 5", f2->GetWidgetName( ) );
        app->Script ( "pack %s -side left -anchor n -padx 0 -pady 5", f3->GetWidgetName( ) );

        //--- make buttons for toggling Bg/Fg and annotations
        this->ToggleFgBgButton->SetParent ( f1 );
        this->ToggleFgBgButton->Create ( );
        this->ToggleFgBgButton->SetWidth ( 16 );
        this->ToggleFgBgButton->SetText ( "Toggle Bg/Fg" );
        this->ToggleAnnotationButton->SetParent ( f1 );
        this->ToggleAnnotationButton->Create ( );
        this->ToggleAnnotationButton->SetWidth ( 16 );
        this->ToggleAnnotationButton->SetText ( "Toggle Annotation" );
    
        app->Script ( "pack %s -side top -anchor w -padx 1 -pady 1", this->ToggleFgBgButton->GetWidgetName( ) );
        app->Script ( "pack %s -side top -anchor w -padx 1 -pady 1", this->ToggleAnnotationButton->GetWidgetName( ) );

        //--- make labels (can't reposition the Scale's labels, so
        //--- supressing those and using a new set.)
        vtkKWLabel *fadeLabel = vtkKWLabel::New ( );
        vtkKWLabel *opacityLabel = vtkKWLabel::New ( );
        fadeLabel->SetParent ( f2 );
        fadeLabel->Create ( );
        fadeLabel->SetWidth ( 14 );
        fadeLabel->SetAnchorToEast ( );
        fadeLabel->SetText ( "Fade (Bg/Fg):");
        opacityLabel->SetParent ( f2 );
        opacityLabel->Create ( );
        opacityLabel->SetWidth ( 14 );
        opacityLabel->SetAnchorToEast ( );
        opacityLabel->SetText ( "Label Opacity:");
        app->Script ( "pack %s -side top -anchor e -padx 1 -pady 1", fadeLabel->GetWidgetName( ) );
        app->Script ( "pack %s -side top -anchor e -padx 1 -pady 2", opacityLabel->GetWidgetName( ) );
        
        //--- make scales for sliding slice visibility in the SliceViewers
        //--- and for sliding slice opacity in the 3D Viewer.
        this->SliceFadeScale->SetParent ( f3 );
        this->SliceFadeScale->Create ( );
        this->SliceFadeScale->SetRange (0.0, 1.0);
        this->SliceFadeScale->SetResolution ( 0.01 );
        this->SliceFadeScale->SetValue ( 0.0 );
        this->SliceFadeScale->SetLength ( 120 );
        this->SliceFadeScale->SetOrientationToHorizontal ( );
        this->SliceFadeScale->ValueVisibilityOff ( );
        this->SliceFadeScale->SetBalloonHelpString ( "Scale fades between Bg and Fg Slice Layers" );

        this->SliceOpacityScale->SetParent ( f3 );
        this->SliceOpacityScale->Create ( );
        this->SliceOpacityScale->SetRange ( 0.0, 1.0 );
        this->SliceOpacityScale->SetResolution ( 0.01 );
        this->SliceOpacityScale->SetValue ( 0.0 );
        this->SliceOpacityScale->SetLength ( 120 );
        this->SliceOpacityScale->SetOrientationToHorizontal ( );
        this->SliceOpacityScale->ValueVisibilityOff ( );
        this->SliceOpacityScale->SetBalloonHelpString ( "Scale sets the opacity label overlay" );

        app->Script ( "pack %s -side top -anchor w -padx 0 -pady 1", this->SliceFadeScale->GetWidgetName( ) );
        app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0", this->SliceOpacityScale->GetWidgetName( ) );

        fadeLabel->Delete ( );
        opacityLabel->Delete ( );
        f1->Delete ( );
        f2->Delete ( );
        f3->Delete ( );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AssignViewControlIcons ( )
{
        //--- assign image data to each label
        this->RotateAroundAIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundAIconLO() );
        this->RotateAroundPIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundPIconLO( ) );
        this->RotateAroundRIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundRIconLO ( ));
        this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconLO ( ));        
        this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconLO ( ));
        this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconLO ( ) );
        this->RotateAroundMiddleIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundMiddleIcon ( ) );
        this->RotateAroundTopCornerIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundTopCornerIcon ( ));
        this->RotateAroundBottomCornerIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundBottomCornerIcon ( ));
        this->LookFromAIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromAIconLO() );
        this->LookFromPIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromPIconLO( ) );
        this->LookFromRIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromRIconLO ( ));
        this->LookFromLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromLIconLO ( ));        
        this->LookFromSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromSIconLO ( ));
        this->LookFromIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromIIconLO ( ) );
        this->LookFromMiddleIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromMiddleIcon ( ) );
        this->LookFromTopCornerIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromTopCornerIcon ( ));
        this->LookFromBottomCornerIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromBottomCornerIcon ( ));
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::MakeViewControlRolloverBehavior ( )
{

        //--- configure and bind for rollover interaction
        this->RotateAroundAIconButton->SetBorderWidth (0);
        this->RotateAroundAIconButton->SetBinding ( "<Enter>",  this, "EnterRotateAroundACallback");
        this->RotateAroundAIconButton->SetBinding ( "<Leave>",  this, "LeaveRotateAroundACallback");
        this->Script ( "%s ListMethods", this->GetTclName() );

        this->RotateAroundPIconButton->SetBorderWidth (0);
        this->RotateAroundPIconButton->SetBinding ( "<Enter>", this, "EnterRotateAroundPCallback");
        this->RotateAroundPIconButton->SetBinding ( "<Leave>", this, "LeaveRotateAroundPCallback");

        this->RotateAroundRIconButton->SetBorderWidth (0);
        this->RotateAroundRIconButton->SetBinding ( "<Enter>", this, "EnterRotateAroundRCallback");
        this->RotateAroundRIconButton->SetBinding ( "<Leave>", this, "LeaveRotateAroundRCallback");

        this->RotateAroundLIconButton->SetBorderWidth (0);
        this->RotateAroundLIconButton->SetBinding ( "<Enter>", this, "EnterRotateAroundLCallback");
        this->RotateAroundLIconButton->SetBinding ( "<Leave>", this, "LeaveRotateAroundLCallback");

        this->RotateAroundSIconButton->SetBorderWidth (0);
        this->RotateAroundSIconButton->SetBinding ( "<Enter>", this, "EnterRotateAroundSCallback");
        this->RotateAroundSIconButton->SetBinding ( "<Leave>", this, "LeaveRotateAroundSCallback");
        
        this->RotateAroundIIconButton->SetBorderWidth (0);
        this->RotateAroundIIconButton->SetBinding ( "<Enter>", this, "EnterRotateAroundICallback");
        this->RotateAroundIIconButton->SetBinding ( "<Leave>", this, "LeaveRotateAroundICallback");
        
        this->RotateAroundMiddleIconButton->SetBorderWidth (0);
        this->RotateAroundTopCornerIconButton->SetBorderWidth (0);
        this->RotateAroundBottomCornerIconButton->SetBorderWidth (0);

        this->LookFromAIconButton->SetBorderWidth (0);
        this->LookFromAIconButton->SetBinding ( "<Enter>", this, "EnterLookFromACallback");
        this->LookFromAIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromACallback");
        
        this->LookFromPIconButton->SetBorderWidth (0);
        this->LookFromPIconButton->SetBinding ( "<Enter>", this, "EnterLookFromPCallback");
        this->LookFromPIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromPCallback");
        
        this->LookFromRIconButton->SetBorderWidth (0);
        this->LookFromRIconButton->SetBinding ( "<Enter>", this, "EnterLookFromRCallback");
        this->LookFromRIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromRCallback");
        
        this->LookFromLIconButton->SetBorderWidth (0);
        this->LookFromLIconButton->SetBinding ( "<Enter>", this, "EnterLookFromLCallback");
        this->LookFromLIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromLCallback");
        
        this->LookFromSIconButton->SetBorderWidth (0);
        this->LookFromSIconButton->SetBinding ( "<Enter>", this, "EnterLookFromSCallback");
        this->LookFromSIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromSCallback");
        
        this->LookFromIIconButton->SetBorderWidth (0);
        this->LookFromIIconButton->SetBinding ( "<Enter>", this, "EnterLookFromICallback");
        this->LookFromIIconButton->SetBinding ( "<Leave>", this, "LeaveLookFromICallback");
        
        this->LookFromMiddleIconButton->SetBorderWidth (0);
        this->LookFromTopCornerIconButton->SetBorderWidth (0);
        this->LookFromBottomCornerIconButton->SetBorderWidth (0);
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildViewControlGUIPanel ( )
{
    if ( this->GetApplication( )  != NULL ) {

        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        vtkSlicerGUILayout *layout = app->GetMainLayout ( );

        vtkKWFrame *f1 = vtkKWFrame::New ( );
        vtkKWFrame *f1a = vtkKWFrame::New ( );    
        vtkKWFrame *f1b = vtkKWFrame::New ( );    
        vtkKWFrame *f2 = vtkKWFrame::New ( );

        // divide the GUI panel into two frames of identical wid.
        int wid = layout->GetDefaultGUIPanelWidth() ;
        int buf = 4;
        int thirdwid = wid/3 - buf;
        
        // create frames and set their widths.
        f1->SetParent ( this->ViewControlFrame );
        f1->Create ( );
        f1->SetWidth ( thirdwid );
        f1->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );
        f1->SetReliefToGroove();

        f2->SetParent ( this->ViewControlFrame );
        f2->Create ( );
        f2->SetWidth ( 2 * thirdwid );
        f2->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );

        f1a->SetParent ( f1 );
        f1a->Create ( );
        f1a->SetWidth (thirdwid );

        f1b->SetParent ( f1 );
        f1b->Create ( );
        f1b->SetWidth ( thirdwid );
        
        //--- create rotate-around and look-from image mosaics from vtkKWLabels
        this->RotateAroundAIconButton->SetParent ( f1b );
        this->RotateAroundAIconButton->Create ( );
        this->RotateAroundAIconButton->SetBalloonHelpString ("Rotate camera in 3D view around A-P axis.");
        this->RotateAroundPIconButton->SetParent ( f1b );
        this->RotateAroundPIconButton->Create ( );
        this->RotateAroundPIconButton->SetBalloonHelpString ("Rotate camera in 3D view around A-P axis.");
        this->RotateAroundRIconButton->SetParent ( f1b );
        this->RotateAroundRIconButton->Create ( );
        this->RotateAroundRIconButton->SetBalloonHelpString ("Rotate camera in 3D view around R-L axis.");
        this->RotateAroundLIconButton->SetParent ( f1b );
        this->RotateAroundLIconButton->Create ( );
        this->RotateAroundLIconButton->SetBalloonHelpString ("Rotate camera in 3D view around R-L axis.");
        this->RotateAroundSIconButton->SetParent ( f1b );
        this->RotateAroundSIconButton->Create ( );
        this->RotateAroundSIconButton->SetBalloonHelpString ("Rotate camera in 3D view around S-I axis.");
        this->RotateAroundIIconButton->SetParent ( f1b );
        this->RotateAroundIIconButton->Create ( );
        this->RotateAroundIIconButton->SetBalloonHelpString ("Rotate camera in 3D view around S-I axis.");
        this->RotateAroundMiddleIconButton->SetParent ( f1b );
        this->RotateAroundMiddleIconButton->Create ( );
        this->RotateAroundTopCornerIconButton->SetParent ( f1b );
        this->RotateAroundTopCornerIconButton->Create ( );
        this->RotateAroundBottomCornerIconButton->SetParent ( f1b );
        this->RotateAroundBottomCornerIconButton->Create ( );
        this->LookFromAIconButton->SetParent ( f1b );
        this->LookFromAIconButton->Create ( );
        this->LookFromAIconButton->SetBalloonHelpString ("Position 3D view camera down the A-axis looking toward center.");
        this->LookFromPIconButton->SetParent ( f1b );
        this->LookFromPIconButton->Create ( );
        this->LookFromPIconButton->SetBalloonHelpString ("Position 3D view camera down the P-axis looking toward center.");
        this->LookFromRIconButton->SetParent ( f1b );
        this->LookFromRIconButton->Create ( );
        this->LookFromRIconButton->SetBalloonHelpString ("Position 3D view camera down the R-axis looking toward center.");
        this->LookFromLIconButton->SetParent ( f1b );
        this->LookFromLIconButton->Create ( );
        this->LookFromLIconButton->SetBalloonHelpString ("Position 3D view camera down the L-axis looking toward center.");
        this->LookFromSIconButton->SetParent ( f1b );
        this->LookFromSIconButton->Create ( );
        this->LookFromSIconButton->SetBalloonHelpString ("Position 3D view camera down the S-axis looking toward center.");
        this->LookFromIIconButton->SetParent ( f1b );
        this->LookFromIIconButton->Create ( );
        this->LookFromIIconButton->SetBalloonHelpString ("Position 3D view camera down the I-axis looking toward center.");
        this->LookFromMiddleIconButton->SetParent ( f1b );
        this->LookFromMiddleIconButton->Create ( );
        this->LookFromTopCornerIconButton->SetParent ( f1b );
        this->LookFromTopCornerIconButton->Create ( );
        this->LookFromBottomCornerIconButton->SetParent ( f1b );
        this->LookFromBottomCornerIconButton->Create ( );

        this->AssignViewControlIcons ( );
        this->MakeViewControlRolloverBehavior ( );
        
        //--- create the nav/zoom widgets
        this->NavZoomInIconButton->SetParent ( f2 );
        this->NavZoomInIconButton->Create ( );
        this->NavZoomInIconButton->SetReliefToFlat ( );        
        this->NavZoomOutIconButton->SetParent ( f2 );        
        this->NavZoomOutIconButton->Create ( );
        this->NavZoomOutIconButton->SetReliefToFlat ( );
        this->NavZoomScale->SetParent ( f2 );
        this->NavZoomScale->Create ( );
        this->NavZoomScale->SetRange (0.0, 1.0);
        this->NavZoomScale->SetResolution ( 0.01 );
        this->NavZoomScale->SetBorderWidth ( 1 );
        this->NavZoomScale->SetValue ( 0.0 );
        // make scale long enough to fill the frame,
        // leaving room for the zoomin, zoomout buttons.
        this->NavZoomScale->SetLength ( 120 );
        this->NavZoomScale->SetOrientationToHorizontal ( );
        this->NavZoomScale->ValueVisibilityOff ( );
        this->NavZoomScale->SetBalloonHelpString ( "Use scale to zoom the navigation window in/out" );
        //--- assign image data to the zoom buttons
        this->NavZoomInIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetNavZoomInIcon() );
        this->NavZoomOutIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetNavZoomOutIcon() );

        // temporary thing until navzoom window is built.
        vtkKWLabel *tmpNavZoom = vtkKWLabel::New ( );
        tmpNavZoom->SetParent (f2);
        tmpNavZoom->Create();        
        tmpNavZoom->SetWidth ( 20);
        tmpNavZoom->SetHeight (10 );

        tmpNavZoom->SetText ( "3DNav / SliceZoom" );
        tmpNavZoom->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );

        //--- other camera control widgets
        this->SpinButton->SetParent ( f1a);
        this->SpinButton->Create ( );
        this->SpinButton->SetText ( "Spin" );

        this->RockButton->SetParent ( f1a );
        this->RockButton->Create ( );
        this->RockButton->SetText ( "Rock" );

        this->OrthoButton->SetParent ( f1a );
        this->OrthoButton->Create ( );
        this->OrthoButton->SetText ( "Ortho" );

        this->CenterButton->SetParent ( f1a );
        this->CenterButton->Create ( );
        this->CenterButton->SetText ( "Center");

        this->SelectButton->SetParent ( f1a );
        this->SelectButton->Create ( );
        this->SelectButton->SetValue ( "Select");

        this->FOVEntry->SetParent ( f1a );
        this->FOVEntry->Create ( );
        this->FOVEntry->SetLabelText ( "FOV: ");
        this->FOVEntry->GetWidget()->SetWidth (4);

        
        this->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -expand n", f1->GetWidgetName ( ) );
        this->Script ( "pack %s -side left -anchor n -fill x -padx 5 -pady 2 -expand n", f2->GetWidgetName( ) );    

        this->Script ( "pack %s -side top -padx 0 -pady 0 -anchor n -expand n ", f1a->GetWidgetName( ) );
        this->Script ( "pack %s -side top -padx 0 -pady 0 -anchor n -expand n ", f1b->GetWidgetName() );
        
        this->Script ("grid %s -row 0 -column 0 -sticky w -padx 3 -pady 2", this->SpinButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 0 -sticky w -padx 3 -pady 2", this->RockButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 0 -sticky w -padx 3 -pady 2", this->OrthoButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 0 -column 1 -sticky ew -padx 0 -pady 2", this->CenterButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 1 -sticky ew -padx 0 -pady 2", this->SelectButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 1 -sticky ew -padx 0 -pady 2", this->FOVEntry->GetWidgetName ( ) );
        
        this->Script ("grid %s -row 0 -column 0 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundPIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 0 -column 1 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundSIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 0 -column 2 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0",this->RotateAroundTopCornerIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 0 -column 3 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromPIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 0 -column 4 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0",  this->LookFromSIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 0 -column 5 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromTopCornerIconButton->GetWidgetName ( ));        
                      
        this->Script ("grid %s -row 1 -column 0 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundRIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 1 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundMiddleIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 1 -column 2 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundLIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 3 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromRIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 4 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromMiddleIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 1 -column 5 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromLIconButton->GetWidgetName ( ));        

        this->Script ("grid %s -row 2 -column 0 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundBottomCornerIconButton->GetWidgetName ( ));
        this->Script ("grid %s -row 2 -column 1 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundIIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 2 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundAIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 3 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromBottomCornerIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 4 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromIIconButton->GetWidgetName ( ) );
        this->Script ("grid %s -row 2 -column 5 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->LookFromAIconButton->GetWidgetName ( ));        

        this->Script ( "grid %s -row 1 -column 0 -padx 0 -pady 0 -sticky ew", this->NavZoomOutIconButton->GetWidgetName() );
        this->Script ( "grid %s -row 1 -column 1 -padx 0 -pady 0 -sticky ew", this->NavZoomScale->GetWidgetName() );
        this->Script ( "grid %s -row 1 -column 2 -padx 0 -pady 0 -sticky ew", this->NavZoomInIconButton->GetWidgetName() );
        this->Script ("grid %s -row 0 -columnspan 3 -ipadx 40 -ipady 0 -padx 0 -pady 0 -sticky nsew", tmpNavZoom->GetWidgetName ( ) );
        
        f1a->Delete();
        f1b->Delete();
        f1->Delete();
        f2->Delete();
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
void vtkSlicerApplicationGUI::BuildGUIPanel ( )
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

            this->SliceControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->SliceControlFrame->Create( );
            this->SliceControlFrame->SetHeight ( layout->GetDefaultSliceControlFrameHeight ( ) );
            
            this->ViewControlFrame->SetParent ( this->MainSlicerWin->GetMainPanelFrame ( ) );
            this->ViewControlFrame->Create( );
            this->ViewControlFrame->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );
            
            // pack logo and slicer control frames
            this->Script ( "pack %s -side top -fill x -padx 1 -pady 1", this->LogoFrame->GetWidgetName() );
            app->Script ( "pack %s -side top -fill x -padx 1 -pady 10", this->ModuleChooseFrame->GetWidgetName() );

            // pack slice and view control frames
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -pady 10", this->ViewControlFrame->GetWidgetName() );
            app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -pady 10", this->SliceControlFrame->GetWidgetName() );

        }
    }

}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundACallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconHI() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundACallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconLO() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundPCallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconHI() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundPCallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconLO() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundRCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconHI() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundRCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconLO() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundLCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconHI() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundLCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconLO() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundSCallback ( ) {
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconHI() );
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundSCallback ( ) {
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconLO() );
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterRotateAroundICallback ( ) {
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconHI() );
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveRotateAroundICallback ( ) {
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconLO() );
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconLO() );

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromACallback ( ) {
    this->LookFromAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromACallback ( ) {
    this->LookFromAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromPCallback ( ) {
    this->LookFromPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromPIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromPCallback ( ) {
    this->LookFromPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromPIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromRCallback ( ) {
    this->LookFromRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromRIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromRCallback ( ) {
    this->LookFromRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromRIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromLCallback ( ) {
    this->LookFromLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromLCallback ( ) {
    this->LookFromLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromSCallback ( ) {
    this->LookFromSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromSIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromSCallback ( ) {
    this->LookFromSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromSIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::EnterLookFromICallback ( ) {
    this->LookFromIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromIIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::LeaveLookFromICallback ( ) {
    this->LookFromIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromIIconLO() );
}
