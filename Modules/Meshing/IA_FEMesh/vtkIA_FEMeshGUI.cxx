/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkIA_FEMeshGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkSlicerApplication.h"

#include "vtkIA_FEMeshGUI.h"

// vtkSlicer includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkRenderWindowInteractor.h"

// STD includes
#include <string>
#include <iostream>
#include <sstream>

// *** Declarations added for Univ. of Iowa Meshing Integration into Slicer3

// include declarations from Univ. of Iowa standalone meshing workflow GUI class hierarchy.  The
// original notebook uses locally-maintained linked lists.  The MRML notebook moves the storage into
// the MRML tree and keeps the same API for the client module. Change of code is minimized betweeen
// the standalone application and the slicer module.

//#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWRenderWidget.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLLayoutNode.h"

#include "vtkKWMimxMainNotebook.h"
//#include "vtkIA_FEMeshMRMLNotebook.h"
#include "vtkKWMimxViewProperties.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkLinkedListWrapperTree.h"

//------------------------------------------------------------------------------
vtkIA_FEMeshGUI* vtkIA_FEMeshGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkIA_FEMeshGUI");
  if(ret)
    {
    return (vtkIA_FEMeshGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIA_FEMeshGUI;
}


//----------------------------------------------------------------------------
vtkIA_FEMeshGUI::vtkIA_FEMeshGUI()
{

  this->Logic = NULL;
  this->MeshingUI = NULL;
//    this->MimxMainNotebook = NULL;
//    this->ViewProperties = NULL;
//    this->DoUndoTree = NULL;
//    this->MainUserInterfacePanel = NULL;
//    this->DisplayPropertyDialog = NULL;


//  Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
//  Mimxcommon_Init(interp);
//  Buildingblock_Init(interp);
  this->InitializeSupportingLibraries();
    
  this->SavedBoxState = 0;
  this->SavedAxisLabelState = 0;
  this->SavedLayoutEnumeration = 0;
  this->FirstEntryToModule=true;

}

//----------------------------------------------------------------------------
vtkIA_FEMeshGUI::~vtkIA_FEMeshGUI()
{

    // remove MRML callbacks registered when Gui was built
    //this->RemoveMRMLObservers();

  if (this->Logic != NULL)
    {
    this->Logic->Delete();
    }
  
  if (this->MeshingUI != NULL)
    {
    this->MeshingUI->SetParent(NULL);
    this->MeshingUI->Delete();
    }

}

void vtkIA_FEMeshGUI::InitializeSupportingLibraries()
{
    // load supporting libraries dynamically.  This is needed
    // since the toplevel is a loadable module but the other libraries
    // didn't get loaded
    Tcl_Interp* interp = this->GetApplication()->GetMainInterp();
    Mimxcommon_Init(interp);
    Buildingblock_Init(interp);
}

//----------------------------------------------------------------------------
void vtkIA_FEMeshGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::AddGUIObservers ( )
{

}



//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::RemoveGUIObservers ( )
{

}


//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::AddMRMLObservers ( )
{
    // register with the MRML scene to receive callbacks when the scene is closed or opened.
    this->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneImportedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneAboutToBeImportedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->GetMRMLScene()->AddObserver(vtkMRMLScene::NewSceneEvent, (vtkCommand *)this->MRMLCallbackCommand);

}



//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::RemoveMRMLObservers ( )
{
    // release callbacks so nothing is called while module is not active
     this->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneImportedEvent,  (vtkCommand *)this->MRMLCallbackCommand);
     this->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneAboutToBeImportedEvent,  (vtkCommand *)this->MRMLCallbackCommand);
     this->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneClosedEvent,  (vtkCommand *)this->MRMLCallbackCommand);
     this->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NewSceneEvent,  (vtkCommand *)this->MRMLCallbackCommand);
}

//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::ProcessGUIEvents ( vtkObject *vtkNotUsed(caller),
                                         unsigned long vtkNotUsed(event),
                                         void *vtkNotUsed(callData) )
{
//  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);

  //cout << "FE callback received!" << endl;
//
  // process events on the object menu
//  if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent )
//      {
//      this->BuildSeparateFEMeshGUI();
//      }

}


//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::ProcessMRMLEvents ( vtkObject *caller,
                                          unsigned long event,
                                          void *callData )
{
      // If there is a scene close or open event, the wizard should
      // keep the lists and viewProperties in sync
      std::cout << "IA_FEMesh: MRML callback command received" << std::endl;

      if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene  && (event == vtkMRMLScene::SceneClosedEvent ))
        {
            std::cout << "IA-FEMesh: got MRML scene close event.  Do housekeeping here to clear out lists and view properties. " << endl;
        }
      if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene  && (event == vtkMRMLScene::NewSceneEvent ))
        {
            std::cout << "IA-FEMesh: got MRML new scene event. " << endl;
        }
}




//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->UIPanel->AddPage ( "IA_FEMesh", "IA_FEMesh", NULL );
  
  // ---
  // MODULE GUI FRAME
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  const char* help = "IA-FEMesh is a multi-block approach for the generation of finite element models. Mesh generation initiates from an existing surface. Building blocks can be placed around the structure of interest which are then projected onto the surface to create a solid mesh. The module documentation can be found at <a>http://slicer.spl.harvard.edu/slicerWiki/index.php/Modules:IA_FEMesh-Documentation-3.4</a>.";
  
  std::string about = "IA-FEMesh is developed by the Musculoskeletal Imaging, Modelling and Experimentation (MIMX) Program\n";
  about += "Center for Computer Aided Design\n";
  about += "The University of Iowa\nIowa City, IA 52242.\n";
  about += "<a>http://www.ccad.uiowa.edu/mimx/</a>\n\n";
  about += "Software developers include Nicole Grosland, Vincent Magnotta, Kiran Shivanna, Curtis Lisle, and Steve Pieper.\n\n";
  about += "This work was funded by awards R21EB001501 and R01EB005973 from the National Institute of Biomedical Imaging";
  about += " and Bioengineering, National Institutes of Health.";
  
  this->BuildHelpAndAboutFrame(this->UIPanel->GetPageWidget ( "IA_FEMesh" ), help, about.c_str() );

  // HELP FRAME
//  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
//  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "IA_FEMesh" ) );
//  helpFrame->Create ( );
//  helpFrame->CollapseFrame ( );
//  helpFrame->SetLabelText ("Help");
//  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
//                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("IA_FEMesh")->GetWidgetName());

  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  //vtkKWFrameWithLabel *moduleFrame = vtkKWFrameWithLabel::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "IA_FEMesh" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("IA_FEMesh");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s -fill both",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("IA_FEMesh")->GetWidgetName());
  moduleFrame->Delete();
   
  // Create the MIMX Main Window.  This is a composite widget which serves as the top of the independent
  // meshing application.  This widget is created and the slicer render window and KWWindow are passed so that
  // widgets created below here are controllable through slicer's interface.
  
  this->MeshingUI = vtkKWMimxMainWindow::New();
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
    this->MeshingUI->SetRenderWidget( this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer() );
    }
  this->MeshingUI->SetMainWindow( this->GetApplicationGUI()->GetMainSlicerWindow() );
  this->MeshingUI->SetParent( moduleFrame );
  this->MeshingUI->Create();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s -fill both",
              this->MeshingUI->GetWidgetName(), moduleFrame->GetFrame()->GetWidgetName());

  this->MeshingUI->CustomApplicationSettingsModuleEntry();

  // setup callbacks to keep track of MRML changes and update the module automatically
  //this->AddMRMLObservers();

}

void vtkIA_FEMeshGUI::SetActiveViewer(vtkSlicerViewerWidget *activeViewer)
{
  if (activeViewer)
    {
    this->MeshingUI->SetRenderWidget( activeViewer->GetMainViewer() );
    }
  else
    {
    this->MeshingUI->SetRenderWidget( NULL );
    }
}

//---------------------------------------------------------------------------
void vtkIA_FEMeshGUI::TearDownGUI ( )
{
  if ( this->MeshingUI )
    {
    this->MeshingUI->SetRenderWidget(NULL);
    this->MeshingUI->SetMainWindow(NULL);
    this->MeshingUI->SetParent(NULL);
    this->MeshingUI->GetViewProperties()->SetMimxMainWindow(NULL);
    this->MeshingUI->GetViewProperties()->SetDoUndoTree(NULL);
    this->MeshingUI->GetMainUserInterfacePanel()->TearDown();
    this->MeshingUI->GetMainUserInterfacePanel()->SetMimxMainWindow(NULL);
    this->MeshingUI->GetMainUserInterfacePanel()->SetDoUndoTree(NULL);
    }
}

// Description:
// Describe behavior at module startup and exit.
void vtkIA_FEMeshGUI::Enter ( )
{
    // moved out of the constructor to reduce danger of memory leaks
    if (this->FirstEntryToModule)
    {
        //this->InitializeSupportingLibraries();
    }

    // get pointers to the current scene.
  //vtkMRMLScene *SlicerScene = vtkMRMLScene::GetActiveScene();
  vtkMRMLViewNode *viewnode = this->GetApplicationGUI()->GetViewControlGUI()->GetActiveView();
  vtkMRMLLayoutNode *layoutnode = this->GetApplicationGUI()->GetGUILayoutNode();
  if (this->SavedBoxState) this->SavedBoxState = viewnode->GetBoxVisible();
  if (this->SavedAxisLabelState) this->SavedAxisLabelState = viewnode->GetAxisLabelsVisible();
  if (this->SavedLayoutEnumeration) this->SavedLayoutEnumeration = layoutnode->GetViewArrangement();
  if (viewnode) {
      viewnode->GetBackgroundColor(this->SavedBackgroundColor);
      viewnode->SetBoxVisible(0);
      viewnode->SetAxisLabelsVisible(0);
      double blackBackground[3]; blackBackground[0]=blackBackground[1]=blackBackground[2] = 0.0;
      viewnode->SetBackgroundColor(blackBackground);
  }
  // add the specific application settings for this module here

  if (layoutnode) layoutnode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
  //this->MeshingUI->AddOrientationAxis();
  this->MeshingUI->CustomApplicationSettingsModuleEntry();
  



  // restore the state of object visibility depending on how they were when exiting the module
  // This is gated to happen only after returning to the module.  Not the first time, when the 
  // lists aren't initialized yet. 
  
  if (this->FirstEntryToModule)
  {
    //std::cout << "IA_FEMesh: First entry received" << std::endl;
    this->FirstEntryToModule=false;
  }
  else
    this->MeshingUI->RestoreVisibilityStateOfObjectLists(); 
}
 
 
 
void vtkIA_FEMeshGUI::Exit ( )
{
  // restore the MRML Scene state
  vtkMRMLViewNode *viewnode = this->GetApplicationGUI()->GetViewControlGUI()->GetActiveView();
  vtkMRMLLayoutNode *layoutnode = this->GetApplicationGUI()->GetGUILayoutNode();
  // remove the specific application settings for this module here
  if (layoutnode) layoutnode->SetViewArrangement(this->SavedLayoutEnumeration);
  if (viewnode) {
    viewnode->SetBoxVisible(this->SavedBoxState);
    viewnode->SetAxisLabelsVisible(this->SavedAxisLabelState);
    viewnode->SetBackgroundColor(this->SavedBackgroundColor);
  }
  //this->MeshingUI->RemoveOrientationAxis();
  this->MeshingUI->CustomApplicationSettingsModuleExit();
  // save the state of object visibility so we can restore later
  this->MeshingUI->SaveVisibilityStateOfObjectLists();


}
