/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $
a
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
#include "vtkKWMessageDialog.h"
#include "vtkKWProgressDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"

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
#include "vtkSlicerViewerInteractorStyle.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerModulesWizardDialog.h"
#include "vtkSlicerModelHierarchyLogic.h"

#include "vtkSlicerFiducialListWidget.h"
#include "vtkSlicerROIViewerWidget.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

#ifdef Slicer3_USE_PYTHON
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#endif

#include <vtksys/stl/vector>
#include <vtksys/stl/map>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");
vtkCxxSetObjectMacro(vtkSlicerApplicationGUI, SlicesGUI, vtkSlicerSlicesGUI);

// temporary crud for vtkDebugLeak hunting. Will remove
// these and other related #ifndefs-#endifs throughout.

//#define LOGODISPLAY_DEBUG
//#define TOOLBAR_DEBUG
//#define VIEWCONTROL_DEBUG
//#define SLICEVIEWER_DEBUG

//#define MENU_DEBUG
//#define SLICESCONTROL_DEBUG
//#define MODULECHOOSE_DEBUG

//----------------------------------------------------------------------------
class vtkSlicerApplicationGUIInternals
{
public:

  typedef vtksys_stl::vector<vtkSlicerViewerWidget*> ViewerWidgetContainerType;
  ViewerWidgetContainerType ViewerWidgets;
};

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
  this->Internals = new vtkSlicerApplicationGUIInternals;

  this->MRMLScene = NULL;
  this->Built = false;
  
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
  this->GridFrame1 = vtkKWFrame::New ( );
  this->GridFrame2 = vtkKWFrame::New ( );
  
  // initialize in case any are not defined.
  this->ApplicationToolbar = NULL;
  this->ViewControlGUI = NULL;
  this->SlicesControlGUI = NULL;
  this->LogoDisplayGUI = NULL;
  this->SlicerFoundationIcons = NULL;
  
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
    
  //--- Main viewer
  this->FiducialListWidget = NULL;
  this->ROIViewerWidget = NULL;

  // use STL::MAP to hold all main slice viewers
  this->SlicesGUI = NULL;

  this->GUILayoutNode = NULL;

  //--- Save and load scene dialogs, widgets
  this->LoadSceneDialog = vtkKWLoadSaveDialog::New();

  this->SaveDataWidget = vtkSlicerMRMLSaveDataWidget::New();

  this->ModulesWizardDialog = NULL;

  //--- unique tag used to mark all view notebook pages
  //--- so that they can be identified and deleted when 
  //--- viewer is reformatted.
  this->ViewerPageTag = 1999;
  this->ProcessingMRMLEvent = 0;
  this->SceneClosing = false;

  this->ModelHierarchyLogic = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( )
{
  if (this->SaveDataWidget)
    {
    this->SaveDataWidget->SetAndObserveMRMLScene ( NULL );
    this->SaveDataWidget->SetParent(NULL);
    this->SaveDataWidget->Delete();
    this->SaveDataWidget=NULL;
    }

  if (this->ModulesWizardDialog)
    {
    this->ModulesWizardDialog->Delete();
    this->ModulesWizardDialog = NULL;
    }

  if ( this->GUILayoutNode )
    {
    this->SetAndObserveGUILayoutNode ( NULL );
    }

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
      if ( app->GetDefaultGeometry() )
        {
        app->GetDefaultGeometry()->SetMainSlicerWindow(NULL);
        }
      }
    this->MainSlicerWindow->SetParent ( NULL );
    this->MainSlicerWindow->Delete ( );
    this->MainSlicerWindow = NULL;
    }

  if ( this->SlicerFoundationIcons )
    {
    this->SlicerFoundationIcons->Delete();
    this->SlicerFoundationIcons =  NULL;
    }

  this->SetApplication(NULL);
  this->SetApplicationLogic ( NULL );

  this->SetModelHierarchyLogic(NULL);

   delete this->Internals;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::TearDownViewers()
{
  // Called by Slicer3.cxx on exit

  this->UnpackMainViewer();

  this->DestroyMain3DViewer ( );
  this->DestroyMainSliceViewers ( );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI:: DeleteComponentGUIs()
{
#ifndef VIEWCONTROL_DEBUG
  if ( this->ViewControlGUI )
    {
    //      this->ViewControlGUI->TearDownGUI ( );
    this->ViewControlGUI->RemoveSliceEventObservers();
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
    this->SlicesControlGUI->TearDownGUI ( );
    this->SlicesControlGUI->Delete ( );
    this->SlicesControlGUI = NULL;
    }
#endif
#ifndef TOOLBAR_DEBUG
  if ( this->ApplicationToolbar )
    {
    this->ApplicationToolbar->RemoveMRMLObservers();
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
  const char *fileName = this->LoadSceneDialog->GetFileName();
  vtkKWProgressDialog *progressDialog = vtkKWProgressDialog::New();
  progressDialog->SetParent( this->MainSlicerWindow );
  progressDialog->SetMasterWindow( this->MainSlicerWindow );
  progressDialog->Create();

  if ( fileName ) 
    {
    std::string fl(fileName);
    if (this->GetMRMLScene() && fl.find(".mrml") != std::string::npos ) 
      {
      std::string message("Loading Scene...\n");
      message += std::string(fileName);
      progressDialog->SetMessageText( message.c_str() );
      // don't observe the scene, to avoid getting render updates
      // during load.  TODO: make a vtk-based progress bar that doesn't
      // call the tcl update method
      //progressDialog->SetObservedObject( this->GetMRMLScene() );
      progressDialog->Display();
      this->GetMRMLScene()->SetURL(fileName);
      this->GetMRMLScene()->Connect();
      this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
      }
    else if (this->GetMRMLScene() && fl.find(".xml") != std::string::npos ) 
      {
      std::string message("Loading Slicer2 Scene...\n");
      message += std::string(fileName);
      progressDialog->SetMessageText( message.c_str() );
      progressDialog->Display();
      this->Script ( "ImportSlicer2Scene \"%s\"", fileName);
      this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
      }
    else if ( this->GetMRMLScene() && fl.find(".xcat") != std::string::npos )
      {
      std::string message("Loading Catalog...\n");
      message += std::string(fileName);
      progressDialog->SetMessageText( message.c_str() );
      progressDialog->Display();
      //---
      //--- perform some of the operations to clean out the scene
      //--- usually done inside the MRMLScene->Connect method.
      //---
      bool undoFlag = this->GetMRMLScene()->GetUndoFlag();
      this->GetMRMLScene()->Clear(false);
      this->Script ( "XcatalogImport \"%s\"", fileName);
      this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
      this->GetMRMLScene()->SetUndoFlag ( undoFlag );
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
  progressDialog->SetParent(NULL);
  progressDialog->Delete();
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessPublishToXnatCommand()
{
  this->Script ("XnatPublish_PublishScene");
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessImportSceneCommand()
{
  this->LoadSceneDialog->RetrieveLastPathFromRegistry(
                                                      "OpenPath");

  vtkKWProgressDialog *progressDialog = vtkKWProgressDialog::New();
  progressDialog->SetParent( this->MainSlicerWindow );
  progressDialog->SetMasterWindow( this->MainSlicerWindow );
  progressDialog->Create();

  this->LoadSceneDialog->Invoke();
  // If a file has been selected for loading...
  const char *fileName = this->LoadSceneDialog->GetFileName();
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
      this->Script ( "ImportSlicer2Scene \"%s\"", fileName);
      this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");
      }
    else if ( this->GetMRMLScene() && fl.find(".xcat") != std::string::npos )
      {
      this->Script ( "XcatalogImport \"%s\"", fileName);
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

  progressDialog->SetParent(NULL);
  progressDialog->Delete();
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddDataCommand()
{
  this->GetApplication()->Script("::Loader::ShowDialog");
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddVolumeCommand()
{
  this->GetApplication()->Script("::LoadVolume::ShowDialog");
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddTransformCommand()
{
  this->GetApplication()->Script("::LoadTransform::ShowDialog");
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
      this->MRMLScene->Clear(false);
      }
    }
  dialog->Delete();
}  

//---------------------------------------------------------------------------
const char* vtkSlicerApplicationGUI::GetCurrentLayoutStringName ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication ( ));
    if ( this->GetGUILayoutNode() != NULL )
      {
      int layout = this->GetGUILayoutNode()->GetViewArrangement ();
    
      if ( layout == vtkMRMLLayoutNode::SlicerLayoutConventionalView)
        {
        return ( "Conventional layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutCompareView)
        {
        return ( "Compare layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutInitialView )
        {
        return ( "Conventional layout" );
        }
      else if (layout == vtkMRMLLayoutNode::SlicerLayoutFourUpView )
        {
        return ( "Four-up layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutOneUp3DView)
        {
        return ( "3D only layout" );
        }
      else if (layout == vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
        {
        return ( "Red slice only layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView)
        {
        return ( "Yellow slice only layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView )
        {
        return ( "Green slice only layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView )
        {
        return ( "Red slice only layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
        {
        return ( "Tabbed 3D layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
        {
        return ( "Tabbed slice layout" );
        }
      else if (layout == vtkMRMLLayoutNode::SlicerLayoutLightboxView )
        {
        return ( "Lightbox layout" );
        }
      else if (layout == vtkMRMLLayoutNode::SlicerLayoutNone )
        {
        return ( "None" );
        }
      else
        {
        return (NULL);
        }
      }
    else
      {
      return ( NULL );
      }
    }
  return ( NULL );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateLayout ( )
{
  int mode;

  if ( this->Built == false )
    {
    return;
    } 

  // make the panel frame sizes match the node (jvm)
  // std::cout << "Setting sizes: " << *this->GUILayoutNode << std::endl;
  this->MainSlicerWindow->GetMainSplitFrame()->SetFrame1Size( this->GUILayoutNode->GetMainPanelSize() );
  this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Size( this->GUILayoutNode->GetSecondaryPanelSize() );  
  
  // make the panel visibility match the node (jvm)
  this->MainSlicerWindow->SetMainPanelVisibility( this->GUILayoutNode->GetGUIPanelVisibility() );
  this->MainSlicerWindow->SetSecondaryPanelVisibility( this->GUILayoutNode->GetBottomPanelVisibility() );

  
  //--- stop spinning/rocking... and
  //--- repack the layout in main viewer if required.
  int target = this->GUILayoutNode->GetViewArrangement();
  
  if ( target == vtkMRMLLayoutNode::SlicerLayoutInitialView &&
      this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutInitialView )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer (vtkMRMLLayoutNode::SlicerLayoutInitialView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutInitialView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutConventionalView &&
      this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutConventionalView )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer (vtkMRMLLayoutNode::SlicerLayoutConventionalView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutConventionalView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUp3DView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUp3DView )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUp3DView, NULL);
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUp3DView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutFourUpView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutFourUpView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutFourUpView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutFourUpView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutTabbed3DView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutTabbed3DView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutTabbed3DView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView, "Yellow");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView, "Green");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView &&
             this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );    
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView &&
             this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView)
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );    
    }
  else if ( (target == vtkMRMLLayoutNode::SlicerLayoutCompareView) )
    {
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutCompareView, NULL);
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutCompareView );    
    }
  else if ( (target == vtkMRMLLayoutNode::SlicerLayoutNone) )
    {
    this->UnpackMainViewer();
    this->SetCurrentLayout ( target == vtkMRMLLayoutNode::SlicerLayoutNone );   
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetAndObserveGUILayoutNode(vtkMRMLLayoutNode *node)
{
  vtkSetAndObserveMRMLNodeMacro ( this->GUILayoutNode, node);
}

//---------------------------------------------------------------------------
vtkMRMLLayoutNode *vtkSlicerApplicationGUI::GetGUILayoutNode()
{
  vtkMRMLLayoutNode *layout;
  
  if ( this->GUILayoutNode == NULL )
    {
    //--- if there's no layout node yet, create it,
    //--- add it to the scene, and make the
    //--- applicationGUI observe it.
    layout = vtkMRMLLayoutNode::New();
    this->MRMLScene->AddNode(layout);
    this->SetAndObserveGUILayoutNode ( layout );
    layout->Delete();    
    }
  
  //--- bail out if infrastructure isn't there.
  if ( this->ApplicationLogic == NULL )
    {
    return (NULL);
    }
  if ( this->ApplicationLogic->GetSelectionNode() == NULL )
    {
    return (NULL);
    }      
  if ( this->GUILayoutNode == NULL )
    {
    return (NULL);
    }
  //--- update MRML selection node.
  this->ApplicationLogic->GetSelectionNode()->SetActiveLayoutID( this->GUILayoutNode->GetID() );
  return ( this->GUILayoutNode);
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessSaveSceneAsCommand()
{
  this->SaveDataWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
  this->SaveDataWidget->AddObserver ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataWidget->Invoke();  

  this->SaveDataWidget->RemoveObservers ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
  return;
}    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ShowModulesWizard()
{
  if (!this->ModulesWizardDialog)
    {
    this->ModulesWizardDialog = vtkSlicerModulesWizardDialog::New();
    }
  if (!this->ModulesWizardDialog->IsCreated())
    {
    this->ModulesWizardDialog->SetApplication( this->GetApplication() );
    this->ModulesWizardDialog->Create();
    }

  this->ModulesWizardDialog->Invoke();  

  if (this->ModulesWizardDialog)
    {
    this->ModulesWizardDialog->Delete();
    this->ModulesWizardDialog = NULL;
    }

  return;
}    

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( )
{
  this->MainSlicerWindow->GetMainSplitFrame()->GetFrame1()->SetBinding("<Configure>", this, "MainSplitFrameConfigureCallback %w %h");
  this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1()->SetBinding("<Configure>", this, "SecondarySplitFrameConfigureCallback %w %h");
  
  
        vtkSlicerApplication::SafeDownCast ( this->GetApplication() )->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
        this->GetMainSlicerWindow()->GetFileMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
        // keep track of changes to Home Module set from Application Settings interface;
        // try trapping the View menu events, and just updating the home module from registry...
        this->LoadSceneDialog->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
#ifndef TOOLBAR_DEBUG
        this->GetApplicationToolbar()->AddGUIObservers ( );
  this->GetApplicationToolbar()->AddMRMLObservers ( );
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
        
        if (this->SlicesGUI)
          {
            vtkSlicerSliceGUI *g = NULL;
            char *layoutname = NULL;
            int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
            for (int i = 0; i < nSliceGUI; i++)
              {
                if (i == 0)
                  {
                    g = this->SlicesGUI->GetFirstSliceGUI();
                    layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
                  }
                else
                  {
                    g = this->SlicesGUI->GetNextSliceGUI(layoutname);
                    layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
                  }
                
                g->GetSliceController()->AddObserver(vtkSlicerSliceControllerWidget::ExpandEvent, 
                                                     (vtkCommand *)this->GUICallbackCommand);
                g->GetSliceController()->AddObserver(vtkSlicerSliceControllerWidget::ShrinkEvent, 
                                                     (vtkCommand *)this->GUICallbackCommand);
              }
          }

        if (this->SaveDataWidget)
          {
            this->SaveDataWidget->AddObserver ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
          }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveGUIObservers ( )
{
  this->MainSlicerWindow->GetMainSplitFrame()->GetFrame1()->RemoveBinding("<Configure>", this, "MainSplitFrameConfigureCallback %w %h");
  this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1()->RemoveBinding("<Configure>", this, "SecondarySplitFrameConfigureCallback %w %h");

  vtkSlicerApplication::SafeDownCast ( this->GetApplication() )->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
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

  if (this->SlicesGUI)
    {
    vtkSlicerSliceGUI *g = NULL;
    char *layoutname = NULL;
    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        g = this->SlicesGUI->GetFirstSliceGUI();
        layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        }
      else
        {
        g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        }
      
      g->GetSliceController()->RemoveObservers(vtkSlicerSliceControllerWidget::ExpandEvent, 
                                               (vtkCommand *)this->GUICallbackCommand);
      g->GetSliceController()->RemoveObservers(vtkSlicerSliceControllerWidget::ShrinkEvent, 
                                               (vtkCommand *)this->GUICallbackCommand);
      }
    }
    
  this->RemoveMainSliceViewerObservers ( );

  if (this->SaveDataWidget)
    {
    this->SaveDataWidget->RemoveObservers ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
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

  vtkKWLoadSaveDialog *filebrowse = vtkKWLoadSaveDialog::SafeDownCast(caller);
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
  vtkSlicerMRMLSaveDataWidget *saveDataWidget = vtkSlicerMRMLSaveDataWidget::SafeDownCast(caller);

  // catch changes to ApplicationSettings, and update the ApplicationSettingsInterface
  if ( event == vtkCommand::ModifiedEvent && vtkSlicerApplication::SafeDownCast(caller) == app )
    {
    this->MainSlicerWindow->GetApplicationSettingsInterface()->Update();
    }

  if (event == vtkSlicerModuleGUI::ModuleSelectedEvent) 
    {
    this->SelectModuleForNode((vtkMRMLNode *)callData);
    return;
    }

  if (event == vtkKWEvent::NotebookRaisePageEvent && 
      caller == this->MainSlicerWindow->GetViewNotebook()) 
    {
    const char **cargs = (const char **)callData;
    const char *page_title = cargs[0];
    if (page_title)
      {
      int nb_viewer_widgets = this->GetNumberOfViewerWidgets();
      for (int i = 0; i < nb_viewer_widgets; ++i)
        {
        vtkSlicerViewerWidget *viewer_widget = this->GetNthViewerWidget(i);
        if (viewer_widget && 
            viewer_widget->GetViewNode() && 
            viewer_widget->GetViewNode()->GetName() && 
            !strcmp(page_title, viewer_widget->GetViewNode()->GetName()))
          {
          viewer_widget->GetViewNode()->SetActive(1);
          break;
          }
        }
      }
    }

  if (saveDataWidget == this->SaveDataWidget && event == vtkSlicerMRMLSaveDataWidget::DataSavedEvent)
    {
    }

  vtkSlicerSliceControllerWidget *controller_caller = 
    vtkSlicerSliceControllerWidget::SafeDownCast(caller);
  if (caller)
    {
    if (event == vtkSlicerSliceControllerWidget::ExpandEvent) 
      {
      if (this->SlicesGUI)
        {
        vtkSlicerSliceGUI *g = NULL;
        char *layoutname = NULL;
        int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
        for (int i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            g = this->SlicesGUI->GetFirstSliceGUI();
            layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
            }
          else
            {
            g = this->SlicesGUI->GetNextSliceGUI(layoutname);
            layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
            }

          g->GetSliceController()->Expand();
          }
        }
      }
    else if (event == vtkSlicerSliceControllerWidget::ShrinkEvent) 
      {
      if (this->SlicesGUI)
        {
        vtkSlicerSliceGUI *g = NULL;
        char *layoutname = NULL;
        int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
        for (int i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            g = this->SlicesGUI->GetFirstSliceGUI();
            layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
            }
          else
            {
            g = this->SlicesGUI->GetNextSliceGUI(layoutname);
            layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
            }

          g->GetSliceController()->Shrink();
          }
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
  if ( this->ProcessingMRMLEvent != 0 )
    {
    return;
    }

  this->ProcessingMRMLEvent = event;
  vtkDebugMacro ( "processing event" << event );

  vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
  vtkMRMLLayoutNode *ln = vtkMRMLLayoutNode::SafeDownCast ( caller );

  if ( this->GUILayoutNode != NULL &&
       ln == this->GUILayoutNode &&
       event == vtkCommand::ModifiedEvent )
    {
    //std::cout << "Layout node modified: " << *this->GUILayoutNode << std::endl;
    this->ApplicationToolbar->UpdateLayoutMenu();
    this->UpdateLayout();
    }

  if (event == vtkMRMLViewNode::ActiveModifiedEvent)
    {
    this->UpdateActiveViewerWidgetDependencies();
    }

  if (scene != NULL &&
      scene == this->MRMLScene &&
      event == vtkCommand::ModifiedEvent )
    {
    // std::cout << "Scene modified" << std::endl;
    if(this->ApplicationToolbar)
      {
      this->ApplicationToolbar->UpdateLayoutMenu();
      }
    // do not update the layout on every scene modified. only update
    // the layout when the layout node is modified (first case) or
    // when switching to a new layout node (third case)
    }
  else if (scene != NULL &&
           scene == this->MRMLScene &&
           event == vtkMRMLScene::SceneCloseEvent )
    {
    // is the scene closing?
    this->SceneClosing = true;
    //-- todo: is this right?
    //    this->SetAndObserveGUILayoutNode ( NULL );
    this->UpdateMain3DViewers();
    }
  else if (scene != NULL &&
           scene == this->MRMLScene
           && event == vtkMRMLScene::NodeRemovedEvent )
    {
    vtkMRMLViewNode *view_node = 
      vtkMRMLViewNode::SafeDownCast((vtkObjectBase *)callData);
    if (view_node)
      {
      this->OnViewNodeRemoved(view_node);
      }
    }
  else if (scene != NULL &&
           scene == this->MRMLScene
           && event == vtkMRMLScene::NodeAddedEvent )
    {
    vtkMRMLViewNode *view_node = 
      vtkMRMLViewNode::SafeDownCast((vtkObjectBase *)callData);
    if (view_node)
      {
      this->OnViewNodeAdded(view_node);
      }

    //--- if node is new layout node, set and observe it.
    //--- and update layout.
    vtkMRMLLayoutNode *layout = vtkMRMLLayoutNode::SafeDownCast ( (vtkObjectBase *)callData);
    if (this->Built == true &&  layout != NULL  && layout != this->GUILayoutNode )
      {
      //--- unset the old layout and use the new.
      //--- Delete the old node after removing it from scene.
      //--- Set and observe the new node. 
/*
      this->MRMLScene->RemoveNode ( this->GUILayoutNode );
      this->GUILayoutNode->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->MRMLCallbackCommand );      
      this->GUILayoutNode->Delete();
      this->GUILayoutNode = NULL;
      this->SetAndObserveGUILayoutNode ( layout );
*/
      // std::cout << "Switching layout nodes" << std::endl;
      this->UpdateLayout();
      this->ApplicationToolbar->UpdateLayoutMenu();
      if ( this->ApplicationLogic != NULL )
        {
        if ( this->ApplicationLogic->GetSelectionNode() != NULL )
          {
          this->ApplicationLogic->GetSelectionNode()->SetActiveLayoutID ( layout->GetID() );
          }
        }
      }
    }
  else 
    {
    this->SceneClosing = false;
    }
  this->ProcessingMRMLEvent = 0;
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
void vtkSlicerApplicationGUI::SelectModuleForNode ( vtkMRMLNode *node )
{
  const char *moduleName = NULL;
  if (node->IsA("vtkMRMLVolumeNode"))
    {
    moduleName = "Volumes";
    }
  else if (node->IsA("vtkMRMLModelNode"))
    {
    moduleName = "Models";
    }
  else if (node->IsA("vtkMRMLTransformNode"))
    {
    moduleName = "Transforms";
    }
  else if (node->IsA("vtkMRMLFiducialListNode"))
    {
    moduleName = "Fiducials";
    }
  if (moduleName)
    {
    this->SelectModule(moduleName, node);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SelectModule ( const char *moduleName )
{
  this->SelectModule( moduleName, NULL );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SelectModule ( const char *moduleName, vtkMRMLNode *node )
{
#ifndef TOOLBAR_DEBUG
  this->GetApplicationToolbar()->GetModuleChooseGUI()->SelectModule(moduleName, node);
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUI ( )
{
  int i;

  if ( this->GetMRMLScene() == NULL )
    {
    return;
    }
  //--- if there is no layout node yet, create one.
  if ( this->GetGUILayoutNode() == NULL )
    {
    vtkMRMLLayoutNode *layout = vtkMRMLLayoutNode::New();
    this->GetMRMLScene()->AddNode ( layout );
    layout->Delete();
    this->SetAndObserveGUILayoutNode ( layout );
    }
  //--- set and observe this node and set it to the active layout.
  this->GUILayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutInitialView);
  if ( this->ApplicationLogic != NULL )
    {
    if ( this->ApplicationLogic->GetSelectionNode() )
      {
      this->ApplicationLogic->GetSelectionNode()->SetActiveLayoutID( this->GetGUILayoutNode()->GetID() );
      }
    }

  this->SaveDataWidget->SetParent ( this->MainSlicerWindow);
  this->SaveDataWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
    
  // Set up the conventional window: 3Dviewer, slice widgets, UI panel for now.
  if ( this->GetApplication() != NULL ) {

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
        
  // Set a pointer to the MainSlicerWindow in vtkSlicerGUILayout, and
  // Set default sizes for all main frames (UIpanel and viewers) in GUI
  geom->SetMainSlicerWindow ( this->MainSlicerWindow );
  geom->InitializeLayoutDimensions ( app->GetApplicationWindowWidth(),
                                     app->GetApplicationWindowHeight(),
                                     app->GetApplicationSlicesFrameHeight());

  this->SlicerFoundationIcons = vtkSlicerFoundationIcons::New();
        
  if ( this->MainSlicerWindow != NULL ) {

  // set up Slicer's main window
  if (this->ProcessingMRMLEvent)
    {
    // can't just modify the node because we'll never get
    // ModifiedEvent on the node to change the visibilities
    this->MainSlicerWindow->SecondaryPanelVisibilityOn ( );
    this->MainSlicerWindow->MainPanelVisibilityOn ( );
    }
  this->GUILayoutNode->SetGUIPanelVisibility(1);
  this->GUILayoutNode->SetBottomPanelVisibility(1);
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
  geom->InitializeMainSlicerWindowSize ( );
  geom->ConfigureMainSlicerWindowPanels ( );

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
  appTB->SetApplicationLogic ( this->GetApplicationLogic());
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

  this->MainSlicerWindow->GetViewNotebook()->AddObserver(
    vtkKWEvent::NotebookRaisePageEvent,(vtkCommand *)this->GUICallbackCommand);
              
  // Build 3DViewer and Slice Viewers

#ifndef SLICEVIEWER_DEBUG
  // restore view layout from application registry...
  this->BuildMainViewer ( app->GetApplicationLayoutType());
  this->ApplicationToolbar->SetLayoutMenubuttonValueToLayout ( app->GetApplicationLayoutType() );
#endif

  // after SliceGUIs are created, the ViewControlGUI
  // needs to observe them to feed its magnifier
  // Zoom Widget.

#ifndef MENU_DEBUG
  // Construct menu bar and set up global key bindings
  // 
  // File Menu
  //
  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Load Scene...", this, "ProcessLoadSceneCommand");
  this->MainSlicerWindow->GetFileMenu()->SetItemAccelerator ( i, "Ctrl-O");
  this->MainSlicerWindow->GetFileMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

  this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                             "Import Scene...", this, "ProcessImportSceneCommand");

  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Add Data...", this, "ProcessAddDataCommand");
  this->MainSlicerWindow->GetFileMenu()->SetItemAccelerator ( i, "Ctrl-A");
  this->MainSlicerWindow->GetFileMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Add Volume...", this, "ProcessAddVolumeCommand");
  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Add Transform...", this, "ProcessAddTransformCommand");

  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Save", this, "ProcessSaveSceneAsCommand");
  this->MainSlicerWindow->GetFileMenu()->SetItemAccelerator ( i, "Ctrl-S");
  this->MainSlicerWindow->GetFileMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Publish to XNAT Host...", this, "ProcessPublishToXnatCommand");

  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCommand (this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                                 "Close Scene", this, "ProcessCloseSceneCommand");
  this->MainSlicerWindow->GetFileMenu()->SetItemAccelerator ( i, "Ctrl-W");
  this->MainSlicerWindow->GetFileMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

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
            
  this->GetMainSlicerWindow()->GetEditMenu()->InsertSeparator (this->GetMainSlicerWindow()->GetEditMenu()->GetNumberOfItems());
  i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "Edit Box", NULL, "::EditBox::ShowDialog" );
  this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "space");
  this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

#ifdef Slicer3_USE_PYTHON
  i = this->MainSlicerWindow->GetWindowMenu()->AddCommand ( "Python console", NULL, "$::slicer3::ApplicationGUI PythonConsole" );
  this->MainSlicerWindow->GetWindowMenu()->SetItemAccelerator ( i, "Ctrl+P");
  this->MainSlicerWindow->GetWindowMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);
#endif

#ifndef FIDUCIALS_DEBUG
  this->GetMainSlicerWindow()->GetEditMenu()->InsertSeparator (this->GetMainSlicerWindow()->GetEditMenu()->GetNumberOfItems());
  // make the new fiducial list, but delete the returned node as
  // it's held onto by the scene
  i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "New Fiducial List", NULL, "[$::slicer3::FiducialsGUI GetLogic] AddFiducialListSelected" );
  this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+L");
  this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);
#endif

  //
  // View Menu
  //

  this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                                             "Cache & Remote I/O Manager", NULL, "$::slicer3::RemoteIOGUI DisplayManagerWindow");

  i = this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                                                 "Module Search", this, "ShowModulesWizard");
  this->GetMainSlicerWindow()->GetViewMenu()->SetItemAccelerator ( i, "slash");
  this->GetMainSlicerWindow()->GetViewMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

  //
  // Help Menu
  //
  this->GetMainSlicerWindow()->GetHelpMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetHelpMenuInsertPosition(),
                                                             "Browse tutorials (www)", NULL, "$::slicer3::ApplicationGUI OpenTutorialsLink");
  this->GetMainSlicerWindow()->GetHelpMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetHelpMenuInsertPosition(),
                                                             "Interface Documentation (www)", NULL, "$::slicer3::ApplicationGUI OpenDocumentationLink");
  this->GetMainSlicerWindow()->GetHelpMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetHelpMenuInsertPosition(),
                                                             "Slicer Publications (www)", NULL, "$::slicer3::ApplicationGUI OpenPublicationsLink");
  //
  // Feedback Menu
  //
  this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: report a bug (www)", NULL, "$::slicer3::ApplicationGUI OpenBugLink");
  this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: report usability issue (www)", NULL, "$::slicer3::ApplicationGUI OpenUsabilityLink");
  this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Feedback: make a feature request (www)", NULL, "$::slicer3::ApplicationGUI OpenFeatureLink");
  this->GetMainSlicerWindow()->GetFeedbackMenu()->AddCommand ("Community: Slicer Visual Blog (www)", NULL, "$::slicer3::ApplicationGUI PostToVisualBlog");            

            
  this->LoadSceneDialog->SetParent ( this->MainSlicerWindow );
  this->LoadSceneDialog->Create ( );
  this->LoadSceneDialog->SetFileTypes("{ {Scenes} {.mrml .xml .xcat} } { {MRML Scene} {.mrml} } { {Slicer2 Scene} {.xml} } { {Xcede Catalog} {.xcat} } { {All} {.*} }");
  this->LoadSceneDialog->RetrieveLastPathFromRegistry("OpenPath");

#endif
  }
  this->Built = true;
//  this->UpdateLayout();
//  this->ApplicationToolbar->UpdateLayoutMenu();

  }
}
   
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitializeSlicesControlGUI (  )
{
#ifndef SLICESCONTROL_DEBUG
  vtkSlicerSlicesControlGUI *scGUI = this->GetSlicesControlGUI ( );
  scGUI->UpdateFromMRML();
  scGUI->UpdateSliceGUIInteractorStyles ( );
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::InitializeViewControlGUI (  )
{
#ifndef VIEWCONTROL_DEBUG
  vtkSlicerViewControlGUI *vcGUI = this->GetViewControlGUI ( );
  vcGUI->UpdateFromMRML();
  vcGUI->UpdateSliceGUIInteractorStyles();
  vcGUI->UpdateMainViewerInteractorStyles( );
  vcGUI->InitializeNavigationWidgetCamera( );
  vcGUI->UpdateNavigationWidgetViewActors ( );
  vcGUI->ConfigureNavigationWidgetRender ( );
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PythonConsole (  )
{
  
#ifdef Slicer3_USE_PYTHON
  PyObject* d = 
    (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary());
  if ( d == NULL )
    {
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", "Failed to startup python interpreter: dictionary null" );
    return;
    }
    
  PyObject* v = PyRun_StringFlags ( "import sys;\n"
                                    "try:\n"
                                    "  import Slicer;\n"
                                    "  reload ( Slicer );\n"
                                    "  Slicer.StartConsole();\n"
                                    "except Exception, e:\n"
                                    "  print 'Failed to import Slicer', e\n"
                                    "sys.stdout.flush();\n"
                                    "sys.stderr.flush();\n",
                                    Py_file_input,
                                    d,
                                    d,
                                    NULL);

  if (v == NULL)
    {
    PyErr_Print();
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", "Failed to startup python interpreter" );
    return;
    }
  Py_DECREF ( v );
  if (Py_FlushLine())
    {
    PyErr_Clear();
    }
#endif
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
        if (name != NULL)
          {
          app->SetHomeModule ( name );
          }
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
    app->OpenLink ("http://wiki.na-mic.org/Wiki/index.php/Slicer3.2:Training" );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenDocumentationLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.slicer.org/slicerWiki/index.php/Documentation" );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OpenPublicationsLink ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    app->OpenLink ("http://www.slicer.org/publications" );
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
    app->OpenLink ("http://www.slicer.org/slicerWiki/index.php/Slicer3:VisualBlog" );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DestroyMainSliceViewers ( )
{

  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }

    //
    // Destroy slice viewers
    //
    if (this->SlicesGUI)
      {
      vtkSlicerSliceGUI *g = NULL;
      char *layoutname = NULL;
      int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          g = this->SlicesGUI->GetFirstSliceGUI();
          layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
          }
        else
          {
          g = this->SlicesGUI->GetNextSliceGUI(layoutname);
          layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
          }
        g->SetAndObserveMRMLScene(NULL);
        g->SetAndObserveModuleLogic(NULL);
        g->RemoveGUIObservers();
        g->SetApplicationLogic(NULL);

        g->Delete();
        g = NULL;
        }
      }
    if ( this->GridFrame1 )
      {
      this->GridFrame1->Delete ( );
      this->GridFrame1 = NULL;
      }
    if ( this->GridFrame2 )
      {
      this->GridFrame2->Delete ( );
      this->GridFrame2 = NULL;
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
    int vh = app->GetDefaultGeometry()->GetDefault3DViewerHeight();
    int sh = app->GetDefaultGeometry()->GetDefaultSliceGUIFrameHeight();
    int sfh = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1Size();
    int sf2h = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame2Size();
      
    // pop up a warning dialog here if the computer's
    // display resolution in x is less than 1000 pixels.

    //--- comment out this block
/*
      const char *wstr = app->Script ("winfo screenwidth .");
      const char *hstr = app->Script ("winfo screenheight .");
      int screenwidth = atoi (wstr);
      int screenheight = atoi ( hstr );
      if (screenwidth < 1000 )
        {
        vtkKWMessageDialog *message = vtkKWMessageDialog::New();
        message->SetParent ( this->MainSlicerWindow );
        message->SetStyleToMessage();
        message->SetDialogName("WarningScreenResolution");
        message->SetText ("Slicer requires a horizontal screen resolution of at least 1024 pixels to display it's user interface. Some GUI elements may not be visible.");
        message->Create();
        message->Invoke();
        message->Delete();
        }
*/
    }
}
   

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildMainViewer ( int arrangementType)
{

  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkSlicerWindow *win = this->MainSlicerWindow;
        
    this->GridFrame1->SetParent ( this->MainSlicerWindow->GetViewFrame ( ) );
    this->GridFrame1->Create ( );            
    this->GridFrame2->SetParent ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ) );
    this->GridFrame2->Create ( );            
    this->CreateMainSliceViewers ( );
    this->UpdateMain3DViewers ( );
    this->PackMainViewer ( arrangementType , NULL );
    }
}

//---------------------------------------------------------------------------
vtkSlicerSliceGUI* vtkSlicerApplicationGUI::GetMainSliceGUI(const char *layoutName)
{
  if (this->SlicesGUI)
    return this->SlicesGUI->GetSliceGUI(layoutName);
  else
    return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::CreateMainSliceViewers ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    
    //
    // 3 Slice Viewers
    //

    // Red
    vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::New ( );
    this->SlicesGUI->AddSliceGUI("Red", g);
    g->SetApplication(app);
    g->SetApplicationLogic(this->ApplicationLogic);
    if ( this->GetApplicationLogic()->GetSliceLogic("Red") )
      {
      g->SetAndObserveModuleLogic( this->GetApplicationLogic()->GetSliceLogic("Red") );
      }
    g->BuildGUI( this->MainSlicerWindow->GetMainSplitFrame( ), color->SliceGUIRed );
    g->AddGUIObservers();
    g->SetAndObserveMRMLScene( this->MRMLScene );

    // Yellow
    g = vtkSlicerSliceGUI::New ( );
    this->SlicesGUI->AddSliceGUI("Yellow", g);
    g->SetApplication(app);
    g->SetApplicationLogic(this->ApplicationLogic);
    if ( this->GetApplicationLogic()->GetSliceLogic("Yellow") )
      {
      g->SetAndObserveModuleLogic( this->GetApplicationLogic()->GetSliceLogic("Yellow") );
      }
    g->BuildGUI( this->MainSlicerWindow->GetMainSplitFrame( ), color->SliceGUIYellow );
    g->AddGUIObservers();
    g->SetAndObserveMRMLScene( this->MRMLScene );

    // Green
    g = vtkSlicerSliceGUI::New ( );
    this->SlicesGUI->AddSliceGUI("Green", g);
    g->SetApplication(app);
    g->SetApplicationLogic(this->ApplicationLogic);
    if ( this->GetApplicationLogic()->GetSliceLogic("Green") )
      {
      g->SetAndObserveModuleLogic( this->GetApplicationLogic()->GetSliceLogic("Green") );
      }
    g->BuildGUI( this->MainSlicerWindow->GetMainSplitFrame( ), color->SliceGUIGreen );
    g->AddGUIObservers();
    g->SetAndObserveMRMLScene( this->MRMLScene );

    }
 }

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateMain3DViewers()
{
  if (this->GetApplication() == NULL || !this->MRMLScene)
    {
    return;
    }

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );

  // Create 3D viewer for the view nodes

  vtksys_stl::map<vtkMRMLViewNode*, int> view_nodes;

  int nb_added = 0;
  vtkMRMLViewNode *node = NULL;
  int n, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLViewNode");
  for (n = 0; n < nnodes; n++)
    {
    node = vtkMRMLViewNode::SafeDownCast (
      this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLViewNode"));
    if (node)
      {
      view_nodes[node] = 1;
      if (!this->GetViewerWidgetForNode(node))
        {
        vtkSlicerViewerWidget *viewer_widget = vtkSlicerViewerWidget::New();
        viewer_widget->SetApplication(app);
        viewer_widget->SetParent(this->MainSlicerWindow);
        viewer_widget->SetAndObserveViewNode(node);
        viewer_widget->SetMRMLScene(this->MRMLScene);
        viewer_widget->SetModelHierarchyLogic(this->ModelHierarchyLogic);
        viewer_widget->Create();
        viewer_widget->UpdateFromMRML();
        viewer_widget->SetApplicationLogic(this->GetApplicationLogic());
        this->Internals->ViewerWidgets.push_back(viewer_widget);
        nb_added++;
        }
      }
    }

  // Remove 3D viewers that have no nodes

  int nb_removed = 0, done;
  do
    {
    done = 1;
    vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator it = 
      this->Internals->ViewerWidgets.begin();
    vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator end = 
      this->Internals->ViewerWidgets.end();
    for (; it != end; ++it)
      {
      vtkSlicerViewerWidget *viewer_widget = (*it);
      if (viewer_widget && 
          view_nodes.find(viewer_widget->GetViewNode()) == view_nodes.end())
        {
        viewer_widget->RemoveMRMLObservers();
        viewer_widget->SetApplicationLogic(NULL);
        viewer_widget->SetParent(NULL);
        viewer_widget->Delete();
        this->Internals->ViewerWidgets.erase(it);
        ++nb_removed;
        done = 0;
        break;
        }
      }
    } while (!done);

  // Add the fiducial list widget

  if (!this->FiducialListWidget)
    {
    this->FiducialListWidget = vtkSlicerFiducialListWidget::New();
    this->FiducialListWidget->SetApplication( app );
    this->FiducialListWidget->Create();
    // add events
    // TODO: this is wrong, this should be in the FiducialListWidget, there is
    // no reason the app should do that... Especially if somebody create
    // his/her own FiducialListWidget instance...
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
    events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    this->FiducialListWidget->SetAndObserveMRMLSceneEvents (this->MRMLScene, events );
    events->Delete();
    }
  
    // add the roi widget
  if (!this->ROIViewerWidget)
    {
    this->ROIViewerWidget = vtkSlicerROIViewerWidget::New();
    this->ROIViewerWidget->SetApplication( app );
    this->ROIViewerWidget->SetMRMLScene(this->MRMLScene);
    this->ROIViewerWidget->Create();
    }

  this->UpdateActiveViewerWidgetDependencies();

  // No 3D view node, let's add one for convenience

  if (!nnodes)
    {
    vtkMRMLViewNode *view_node = vtkMRMLViewNode::New();
    view_node->SetName(
      this->MRMLScene->GetUniqueNameByString(view_node->GetNodeTagName()));
    this->MRMLScene->AddNode(view_node);
    view_node->Delete();
    // This is driving me NUTS. Click on a view node in the scene tree,
    // delete it, then this code should take care of re-creating one
    // BUT NodeAddedEvent is NEVER triggered in ProcessMRMLEvents.
    // Why oh Why??? It is triggered when Slicer3 is started and a default
    // view is created by this very same code.
    // Force that behavior by calling the code explicitly!
    this->OnViewNodeAdded(view_node);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OnViewNodeAdded(vtkMRMLViewNode *view_node)
{
  view_node->AddObserver(vtkMRMLViewNode::ActiveModifiedEvent, 
                         this->MRMLCallbackCommand );
  if (this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLViewNode") == 1)
    {
    view_node->SetActive(1);
    }
  this->UpdateMain3DViewers();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OnViewNodeRemoved(vtkMRMLViewNode *view_node)
{
  view_node->RemoveObservers(vtkMRMLViewNode::ActiveModifiedEvent, 
                             this->MRMLCallbackCommand );
  this->UpdateMain3DViewers();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateActiveViewerWidgetDependencies()
{
  vtkSlicerViewerWidget *active_viewer = this->GetActiveViewerWidget();

  this->FiducialListWidget->SetViewerWidget(active_viewer);
  this->FiducialListWidget->SetMainViewer(
    active_viewer ? active_viewer->GetMainViewer() : NULL);
  this->FiducialListWidget->SetInteractorStyle(active_viewer ? vtkSlicerViewerInteractorStyle::SafeDownCast(active_viewer->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle()) : NULL);
  this->FiducialListWidget->UpdateFromMRML();
  
  this->ROIViewerWidget->SetMainViewerWidget(active_viewer);
  this->ROIViewerWidget->UpdateFromMRML();

#ifndef VIEWCONTROL_DEBUG
  vtkSlicerViewControlGUI *vcGUI = this->GetViewControlGUI ( );
  vcGUI->SetViewNode(active_viewer ? active_viewer->GetViewNode() : NULL);
  //this->InitializeViewControlGUI();
#endif

  if (active_viewer)
    {
    this->MainSlicerWindow->GetViewNotebook()->RaisePage(
      active_viewer->GetViewNode()->GetName());
    }

  vtkMRMLLayoutNode *layout = this->GetGUILayoutNode();
  if (layout)
    {
    // Since neither UpdateLayout or RepackMainViewer can be called 
    // correctly on their own...
    int old_ar = layout->GetViewArrangement();
    layout->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutNone);
    this->UpdateLayout();
    layout->SetViewArrangement(
      old_ar != vtkMRMLLayoutNode::SlicerLayoutNone 
      ? old_ar : vtkMRMLLayoutNode::SlicerLayoutInitialView);
    // WHY THE HELL isn't *this* SetViewArrangement not triggering
    // ProcessMRMLEvents!!! Forcing with UpdateLayout()!!
    this->UpdateLayout();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DestroyMain3DViewer ( )
{
  // Called by TearDownViewers (called by Slicer3.cxx on exit)

  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }
      
    // Destroy fiducial list
    if ( this->FiducialListWidget )
      {
      this->FiducialListWidget->RemoveMRMLObservers ();
      this->FiducialListWidget->SetParent(NULL);
      this->FiducialListWidget->Delete();
      this->FiducialListWidget = NULL;
      }
    // Destroy roi widget
    if ( this->ROIViewerWidget )
      {
      this->ROIViewerWidget->RemoveMRMLObservers ();
      this->ROIViewerWidget->SetParent(NULL);
      this->ROIViewerWidget->Delete();
      this->ROIViewerWidget = NULL;
      }
      
    // Destroy 3D viewers
    //
    vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator it = 
      this->Internals->ViewerWidgets.begin();
    vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator end = 
      this->Internals->ViewerWidgets.end();
    for (; it != end; ++it)
      {
      vtkSlicerViewerWidget *viewer_widget = (*it);
      if (viewer_widget)
        {
        viewer_widget->RemoveMRMLObservers();
        viewer_widget->SetApplicationLogic(NULL);
        viewer_widget->SetParent(NULL);
        viewer_widget->Delete();
        }
      }
    this->Internals->ViewerWidgets.clear();
    }
}

//---------------------------------------------------------------------------
int vtkSlicerApplicationGUI::GetNumberOfViewerWidgets()
{
  return (int)this->Internals->ViewerWidgets.size();
}
 
//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicerApplicationGUI::GetNthViewerWidget(int idx)
{
  if (idx < 0 || idx >= this->GetNumberOfViewerWidgets())
    {
    return NULL;
    }

  return this->Internals->ViewerWidgets[idx];
}
 
//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicerApplicationGUI::GetViewerWidgetForNode(
  vtkMRMLViewNode *node)
{
  int nb_viewer_widgets = this->GetNumberOfViewerWidgets();
  for (int i = 0; i < nb_viewer_widgets; ++i)
    {
    vtkSlicerViewerWidget *viewer_widget = this->GetNthViewerWidget(i);
    if (viewer_widget && viewer_widget->GetViewNode() == node)
      {
      return viewer_widget;
      }
    }

  return NULL;
}
 
//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicerApplicationGUI::GetActiveViewerWidget()
{
  int nb_viewer_widgets = this->GetNumberOfViewerWidgets();
  for (int i = 0; i < nb_viewer_widgets; ++i)
    {
    vtkSlicerViewerWidget *viewer_widget = this->GetNthViewerWidget(i);
    if (viewer_widget && viewer_widget->GetViewNode()->GetActive())
      {
      return viewer_widget;
      }
    }

  // no active found, alright, use first one, if any
  // Legacy support, the active flag on the vtkMRMLViewNode was saved, but not
  // used. Sadly, it was saved as "false", which means that now that this flag
  // is supported, snapshots start disabling the view! Try to work around this.
  return this->GetNthViewerWidget(0); 
}
#if 1
//---------------------------------------------------------------------------
vtkSlicerViewerWidget* vtkSlicerApplicationGUI::GetViewerWidget()
{
  // Deprecated.
  return this->GetActiveViewerWidget();
}
#endif
//---------------------------------------------------------------------------
vtkRenderWindowInteractor* 
vtkSlicerApplicationGUI::GetActiveRenderWindowInteractor()
{
  vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
  if (viewer_widget &&
      viewer_widget->GetMainViewer() &&
      viewer_widget->GetMainViewer()->GetRenderWindow())
    {
    return viewer_widget->GetMainViewer()->GetRenderWindow()->GetInteractor();
    }
  return NULL;
}

//---------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkSlicerApplicationGUI::GetRenderWindowInteractor()
{
  // Deprecated.
  return this->GetActiveRenderWindowInteractor();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Save3DViewConfig ( )
{
  vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
  if (viewer_widget)
    {
    // TODO: Save the ViewerWidget's Camera Node
    viewer_widget->GetMainViewer()->GetRenderer()->ComputeVisiblePropBounds(
      this->MainRendererBBox);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::Restore3DViewConfig ( )
{
  vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
  if (viewer_widget)
    {
    // TODO: Restore the ViewerWidget's Camera Node
    viewer_widget->GetMainViewer()->GetRenderer()->ResetCamera ( );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackMainViewer ( int arrangmentType, const char *whichSlice)
{
  if ( this->GetApplication() != NULL )
    {
      switch ( arrangmentType)
        {
        case vtkMRMLLayoutNode::SlicerLayoutInitialView:
          this->PackConventionalView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutConventionalView:
          this->PackConventionalView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutFourUpView:
          this->PackFourUpView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUp3DView:
          this->PackOneUp3DView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView:
            this->PackOneUpSliceView ("Red");
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView:
          this->PackOneUpSliceView ("Yellow");
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView:
          this->PackOneUpSliceView ("Green");
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView:
          if ( whichSlice == NULL )
            {
            this->PackOneUpSliceView ( "Red" );
            }
          else
            {
            this->PackOneUpSliceView ( whichSlice );
            }
          break;
        case vtkMRMLLayoutNode::SlicerLayoutTabbed3DView:
          this->PackTabbed3DView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView:
          this->PackTabbedSliceView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutLightboxView:
          this->PackLightboxView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutCompareView:
          this->PackCompareView();
          break;
        default:
          this->PackConventionalView ( );
          break;
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackMainViewer ( )
{
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

      switch (this->CurrentLayout)
        {
        case vtkMRMLLayoutNode::SlicerLayoutInitialView:
          this->UnpackConventionalView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutConventionalView:
          this->UnpackConventionalView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutFourUpView:
          this->UnpackFourUpView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUp3DView:
          this->UnpackOneUp3DView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView:
            this->UnpackOneUpSliceView ();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView:
          this->UnpackOneUpSliceView ();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView:
          this->UnpackOneUpSliceView ();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView:
          this->UnpackOneUpSliceView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutTabbed3DView:
          this->UnpackTabbed3DView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView:
          this->UnpackTabbedSliceView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutLightboxView:
          this->UnpackLightboxView ( );
          break;
        case vtkMRMLLayoutNode::SlicerLayoutCompareView:
          this->UnpackCompareView();
          break;
        default:
          this->UnpackConventionalView ( );
          break;
        }

      if ( this->GridFrame1 )
        {
          app->Script ( "pack forget %s", this->GridFrame1->GetWidgetName() );
        }
      if ( this->GridFrame2 )
        {
          app->Script ( "pack forget %s", this->GridFrame2->GetWidgetName() );
        }
    }
  
  this->CurrentLayout = vtkMRMLLayoutNode::SlicerLayoutNone;
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RepackMainViewer ( int arrangementType, const char *whichSlice )
{
  this->UnpackMainViewer();

  // Need to reset the weights, minsize, and pad on the grids to get
  // the grids to resize back to the default size of 0,0
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    if (this->GridFrame1)
      {
      // don't want to resort to this line but it seems to be needed
      // to clear out the slaves
      app->Script("catch {grid forget [grid slaves %s]}",
                  this->GridFrame1->GetWidgetName() );
      
      int i;
      std::stringstream ss;
      const char *size = app->Script("grid size %s",
                                     this->GridFrame1->GetWidgetName());
      // std::cout << size << std::endl;
      ss << size;
      int rows, cols;
      ss >> cols;
      ss >> rows;
      // std::cout << "Cleaning (1) " << rows << ", " << cols << std::endl;

      for (i=0; i < rows; ++i)
        {
        app->Script("grid rowconfigure %s %d -weight 0 -minsize 0 -pad 0",
                    this->GridFrame1->GetWidgetName(), i);
        }
      for (i=0; i < cols; ++i)
        {
        app->Script("grid columnconfigure %s %d -weight 0 -minsize 0 -pad 0",
                    this->GridFrame1->GetWidgetName(), i);
        }
      }
    if (this->GridFrame2)
      {
      // don't want to resort to this line but it seems to be needed
      // to clear out the slaves
      app->Script("catch {grid forget [grid slaves %s]}",
                  this->GridFrame2->GetWidgetName() );

      int i;
      std::stringstream ss;
      const char *size = app->Script("grid size %s",
                                     this->GridFrame2->GetWidgetName());
      // std::cout << size << std::endl;
      ss << size;
      int rows, cols;
      ss >> cols;
      ss >> rows;
      // std::cout << "Cleaning (2) " << rows << ", " << cols << std::endl;

      for (i=0; i < rows; ++i)
        {
        app->Script("grid rowconfigure %s %d -weight 0 -minsize 0 -pad 0",
                    this->GridFrame2->GetWidgetName(), i);
        }
      for (i=0; i < cols; ++i)
        {
        app->Script("grid columnconfigure %s %d -weight 0 -minsize 0 -pad 0",
                    this->GridFrame2->GetWidgetName(), i);
        }
      }

    // use these lines to print out any hold overs from ungridding.
    // Size should be "0 0" and there should be no widgets listed
//      app->Script("puts \"After forgetting size (1): [grid size %s] : [grid slaves %s]\"", this->GridFrame1->GetWidgetName ( ) , this->GridFrame1->GetWidgetName ( ) );
//      app->Script("puts \"After forgetting size (2): [grid size %s] : [grid slaves %s]\"", this->GridFrame2->GetWidgetName ( ) , this->GridFrame2->GetWidgetName ( ) );
    }
    
  

  // Since I can't find a way to re-title this main page titled "View",
  // we make sure it's visible, and then 'hide' it only when we want to
  // show tabs that say things other than "View".
  this->MainSlicerWindow->GetViewNotebook()->ShowPage ( "View");
  this->PackMainViewer ( arrangementType, whichSlice );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackConventionalView ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkMRMLScene *scene = this->GetMRMLScene();
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode();
    if ( layout == NULL )
      {
      return;
      }

    vtkSlicerSliceGUI *g = NULL;

    // Note that the top panel is Frame2 of the SecondarySplitFrame
    // and the bottom panel is Frame1 of the SecondarySplitFrame
    //
    // The frame we create in the top frame is GridFrame1 while
    // the frame we create in the bottom frame is GridFrame2.
    //
    // So be careful with the sense of 1 and 2.
    
    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);

    // Show the bottom panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(1);

    // Don't use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
    
    // Pack
    vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
    if (viewer_widget)
      {
      viewer_widget->PackWidget(this->MainSlicerWindow->GetViewFrame() );
      }

    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame2->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid columnconfigure %s 1 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid columnconfigure %s 2 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );
      
    //--- red slice viewer
    g = this->SlicesGUI->GetSliceGUI("Red");
    g->GridGUI( this->GridFrame2, 0, 0 );

    //--- yellow slice viewer
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    g->GridGUI( this->GridFrame2, 0, 1 );

    //--- green slice viewer
    g = this->SlicesGUI->GetSliceGUI("Green");
    g->GridGUI( this->GridFrame2, 0, 2 );

    
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();

    // finally, modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility(1);
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutConventionalView)
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutConventionalView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackOneUp3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkMRMLScene *scene = this->GetMRMLScene();
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }

    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);
    
    // Hide the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);

    // Don't use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );      

    // Pack
    vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
    if (viewer_widget)
      {
      viewer_widget->PackWidget(this->MainSlicerWindow->GetViewFrame() );
      }
    
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    g->PackGUI(this->MainSlicerWindow->GetSecondaryPanelFrame());
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    g->PackGUI(this->MainSlicerWindow->GetSecondaryPanelFrame());
    g = this->SlicesGUI->GetSliceGUI("Green");
    g->PackGUI(this->MainSlicerWindow->GetSecondaryPanelFrame());
    
    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 0 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutOneUp3DView )
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutOneUp3DView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackOneUpSliceView ( const char * whichSlice )
{
  if ( this->GetApplication() != NULL && whichSlice != NULL)
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }

    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);
    
    // Hide the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);

    // Don't use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );

    // Pack
    vtkSlicerSliceGUI *g;
    int cur = layout->GetViewArrangement();
    int newlayout = vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView;
    if ( !strcmp (whichSlice, "Red" ) )
      {
      g = this->SlicesGUI->GetSliceGUI("Red");
      g->PackGUI( this->MainSlicerWindow->GetViewFrame());
      newlayout = vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ;
      }
    else if ( !strcmp ( whichSlice, "Yellow" ) )
      {
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      g->PackGUI( this->MainSlicerWindow->GetViewFrame());
      newlayout = vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView ;
      }
    else if ( !strcmp ( whichSlice, "Green" ) )
      {
      g = this->SlicesGUI->GetSliceGUI("Green");
      g->PackGUI( this->MainSlicerWindow->GetViewFrame());
      newlayout = vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ;
      }
    
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
   
    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 0 );
    if ( cur != newlayout )
      {
      layout->SetViewArrangement ( newlayout );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackFourUpView ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode();
    if ( layout == NULL )
      {
      return;
      }

    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);
    
    // Hide the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);

    // Don't use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );

    // Use this frame in MainSlicerWindow's ViewFrame to grid in the various viewers.
    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame1->GetWidgetName ( ) );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid rowconfigure %s 1 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid columnconfigure %s 1 -weight 1", this->GridFrame1->GetWidgetName() );
    
    vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
    if (viewer_widget)
      {
      viewer_widget->GridWidget ( this->GridFrame1, 0, 1 );
      }

    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    g->GridGUI( this->GetGridFrame1(), 0, 0 );
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    g->GridGUI( this->GetGridFrame1(), 1, 0 );
    g = this->SlicesGUI->GetSliceGUI("Green");
    g->GridGUI( this->GetGridFrame1(), 1, 1 );

    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
   
    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 0 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutFourUpView )
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutFourUpView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackTabbed3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }

    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);

    // Hide the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);

    // Use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
    
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    g->PackGUI( this->MainSlicerWindow->GetSecondaryPanelFrame( ));
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    g->PackGUI( this->MainSlicerWindow->GetSecondaryPanelFrame( ));
    g = this->SlicesGUI->GetSliceGUI("Green");
    g->PackGUI( this->MainSlicerWindow->GetSecondaryPanelFrame( ));
    
    // don't know how to change the title of this one,
    // so just hide it in this configuration, and expose
    // it again when the view configuration changes.
    this->MainSlicerWindow->GetViewNotebook()->HidePage ( "View");
    
    // Add a page for the each view

    int nb_viewer_widgets = this->GetNumberOfViewerWidgets();
    int active_id = -1;
    for (int i = 0; i < nb_viewer_widgets; ++i)
      {
      vtkSlicerViewerWidget *viewer_widget = this->GetNthViewerWidget(i);
      if (viewer_widget)
        {
        int id = this->MainSlicerWindow->GetViewNotebook()->AddPage(
          viewer_widget->GetViewNode()->GetName(), 
          NULL, NULL, this->ViewerPageTag );
        viewer_widget->PackWidget(
          this->MainSlicerWindow->GetViewNotebook()->GetFrame(id));
        if (viewer_widget->GetViewNode()->GetActive())
          {
          active_id = id;
          }
        }
      }
    if (active_id >= 0)
      {
      this->MainSlicerWindow->GetViewNotebook()->RaisePage(active_id);
      }
    
    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 0 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutTabbed3DView );
      }
    layout->DisableModifiedEventOff();
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
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }
    
    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);
    
    // Hide the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);

    // Use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 1 );
    
    this->MainSlicerWindow->GetViewNotebook()->AddPage("Red slice", NULL, NULL, this->ViewerPageTag );
    
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    
    g->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Red slice") );
    this->MainSlicerWindow->GetViewNotebook()->AddPage("Yellow slice", NULL, NULL, this->ViewerPageTag );
    
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    g->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Yellow slice") );
    this->MainSlicerWindow->GetViewNotebook()->AddPage("Green slice", NULL, NULL, this->ViewerPageTag );
    
    g = this->SlicesGUI->GetSliceGUI("Green");
    g->PackGUI ( this->MainSlicerWindow->GetViewNotebook( )->GetFrame ("Green slice") );
    
    // don't know how to change the title of this one,
    // so just hide it in this configuration, and expose
    // it again when the view configuration changes.
    this->MainSlicerWindow->GetViewNotebook()->HidePage ( "View");
     
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
 
    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 0 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackCompareView()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    double x, y, z;
    
    // Show the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(1);
    
    // Show the secondary panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(1);

    // Don't use tabs
    this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );

   // setup the layout for Frame1
    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame1->GetWidgetName ( ) );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid columnconfigure %s 0 -weight 1 -uniform 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid columnconfigure %s 1 -weight 1 -uniform 1", this->GridFrame1->GetWidgetName() );
    
    //--- CompareView puts the Red Slice GUI and 3D Viewer widget side by
    //--- side in a top row. Then, the requested compare view rows and cols
    //--- are arrayed in a grid beneath these two.
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    x = g->GetSliceNode()->GetFieldOfView()[0];
    y = g->GetSliceNode()->GetFieldOfView()[1];
    z = g->GetSliceNode()->GetFieldOfView()[2];
    g->GetSliceNode()->SetFieldOfView(x, y, z);
    g->GetSliceNode()->UpdateMatrices();
    
    //--TODO: when Compare view gets added into the vtkMRMLLayoutNode,
    vtkSlicerViewerWidget *viewer_widget = this->GetActiveViewerWidget();
    if (viewer_widget)
      {
      viewer_widget->GridWidget ( this->GridFrame1, 0, 1);
      }
    g->GridGUI ( this->GetGridFrame1( ), 0, 0 );

    // insert a number of new main slice viewers according to user's input
    char buf[20];
    for ( int i = 0; i < layout->GetNumberOfCompareViewRows(); i++)
      {
      sprintf(buf, "Compare%d", i);
      this->AddMainSliceGUI(buf);

      //--- Configure the lightbox niside each viewer by
      //--- and triggering the SliceControllerWidget's event path.
      int numRows = layout->GetNumberOfCompareViewLightboxRows();
      int numColumns = layout->GetNumberOfCompareViewLightboxColumns();
      vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI(buf);
      if (g != NULL)
        {
        //--- go thru node to set lightbox rows and columns
        if ( g->GetLogic() != NULL )
          {
          if ( g->GetLogic()->GetSliceNode() != NULL )
            {
            g->GetLogic()->GetSliceNode()->SetLayoutGrid ( numRows, numColumns );
            }
          }
        }
      }
    
    // configure the new layout
    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame2->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );
    
    char *layoutname = NULL;
    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    int ncount = 0;
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        g = this->SlicesGUI->GetFirstSliceGUI();
        layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        }
      else
        {
        g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        }
      
      if ( strcmp(layoutname, "Red") == 0 ||
           strcmp(layoutname, "Yellow") == 0 ||
           strcmp(layoutname, "Green") == 0)
        {
        g->GetSliceNode()->SetSliceVisible(0);
        continue;
        }
      else
        {
        g->GridGUI( this->GetGridFrame2( ), ncount, 0 );
        g->GetSliceViewer()->SetWidth(geom->GetDefaultSliceGUIFrameWidth());
        this->Script ("grid rowconfigure %s %d -weight 1", this->GridFrame2->GetWidgetName(), ncount );

        if (g->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID() == 0)
          {
          // no volume assigned. use the background of Red viewer for
          // first compare viewer, the foreground of Red Viewer for
          // second compare viewer, and rest like the first
          vtkSlicerSliceGUI *red = this->SlicesGUI->GetSliceGUI("Red");
          if ((ncount == 0 || ncount > 2) && red && red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID())
            {
            g->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID( red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID());
            }
          else if (ncount == 1 && red && red->GetLogic()->GetSliceCompositeNode()->GetForegroundVolumeID())
            {
            g->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID( red->GetLogic()->GetSliceCompositeNode()->GetForegroundVolumeID());
            }
          }
        
        ncount++;
        //--- if more compare viewers were created previously,
        //--- but fewer are requested in this layout change,
        //--- then we display only a subset of those already created.
        if ( ncount == layout->GetNumberOfCompareViewRows() )
          {
          break;
          }
        }
      }

    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();

    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 1 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutCompareView )
      {
      layout->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutCompareView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackConventionalView()
{
  // Unpack the 3D viewer widget 
  // (we don't know if it is the active widget or not)
  if (this->MainSlicerWindow->GetViewFrame())
    {
    this->MainSlicerWindow->GetViewFrame()->UnpackChildren();
    }
  
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UngridGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Green");
      if (g)
        {
          g->UngridGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      if (g)
        {
          g->UngridGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackOneUp3DView()
{
  // Unpack the 3D viewer widget 
  // (we don't know if it is the active widget or not)
  if (this->MainSlicerWindow->GetViewFrame())
    {
    this->MainSlicerWindow->GetViewFrame()->UnpackChildren();
    }
  
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Green");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      if (g)
        {
          g->UnpackGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackOneUpSliceView()
{
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      if (this->CurrentLayout == vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
          || this->CurrentLayout == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView)
        {
          g = this->SlicesGUI->GetSliceGUI("Red");
        }
      else if (this->CurrentLayout == vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
        {
          g = this->SlicesGUI->GetSliceGUI("Green");
        }
      else if (this->CurrentLayout == vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView)
        {
          g = this->SlicesGUI->GetSliceGUI("Yellow");
        }

      if (g)
        {
          g->UnpackGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackFourUpView()
{
  // Unpack the 3D viewer widget 
  // (we don't know if it is the active widget or not)
  this->GridFrame1->UnpackChildren();
  
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UngridGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Green");
      if (g)
        {
          g->UngridGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      if (g)
        {
          g->UngridGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackTabbed3DView()
{
  this->MainSlicerWindow->GetViewNotebook()->RemovePagesMatchingTag ( this->ViewerPageTag );
  
  //  if (this->ViewerWidget)
    {
    //    this->ViewerWidget->UnpackWidget();
    }
  
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Green");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      if (g)
        {
          g->UnpackGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackTabbedSliceView()
{
  this->MainSlicerWindow->GetViewNotebook()->RemovePage( "Red slice" );
  this->MainSlicerWindow->GetViewNotebook()->RemovePage( "Yellow slice" );
  this->MainSlicerWindow->GetViewNotebook()->RemovePage( "Green slice" );

  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Green");
      if (g)
        {
          g->UnpackGUI();
        }
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      if (g)
        {
          g->UnpackGUI();
        }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackCompareView()
{
  // Unpack the 3D viewer widget 
  // (we don't know if it is the active widget or not)
  this->GridFrame1->UnpackChildren();
  
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UngridGUI();
        }

    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    char *layoutname = NULL;
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        g = this->SlicesGUI->GetFirstSliceGUI();
        layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        }
      else
        {
        g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        }
      
      if ( strncmp(layoutname, "Compare", 7) == 0 )
        {
          g->UngridGUI();
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackLightboxView()
{
  // nothing implemented for this layout (pack or unpack)
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMainSliceGUI(const char *layoutName)
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }
    // if get "compare0" is NULL, add it
    //---wjpTEST >
    //--- somehow GetSliceLogic isn't returning good value.
    vtkSlicerSliceLogic *sliceLogic = this->GetApplicationLogic()->GetSliceLogic(layoutName);
    if (sliceLogic == NULL)
      {
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
      events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

      sliceLogic = vtkSlicerSliceLogic::New ( );
      this->GetApplicationLogic()->AddSliceLogic(layoutName, sliceLogic);
      sliceLogic->SetName(layoutName);
      
      sliceLogic->SetMRMLScene( this->MRMLScene );
      sliceLogic->ProcessLogicEvents();
      sliceLogic->ProcessMRMLEvents(this->MRMLScene, vtkCommand::ModifiedEvent, NULL);
      sliceLogic->SetAndObserveMRMLSceneEvents( this->GetMRMLScene(), events );
      
      events->Delete();
      }
    
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI(layoutName);
    if (g == NULL)
      {
      g = vtkSlicerSliceGUI::New( );
      this->SlicesGUI->AddSliceGUI(layoutName, g);
      g->SetApplication(app);
      g->SetApplicationLogic(this->ApplicationLogic);
      g->BuildGUI ( this->MainSlicerWindow->GetMainSplitFrame(), color->SliceGUIOrange );
      //g->GetSliceController()->GetSliceNode()->SetOrientationToSagittal();
      g->AddGUIObservers();
      g->SetAndObserveMRMLScene(this->MRMLScene);
      g->SetAndObserveModuleLogic(sliceLogic);
      
      // need to check the delete section to prevent memory leak
      g->GetSliceController()->AddObserver(vtkSlicerSliceControllerWidget::ExpandEvent, 
                                           (vtkCommand *)this->GUICallbackCommand);
      g->GetSliceController()->AddObserver(vtkSlicerSliceControllerWidget::ShrinkEvent, 
                                           (vtkCommand *)this->GUICallbackCommand);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackLightboxView ( )
{

/*
    if ( this->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
      vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
      vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
      if ( layout == NULL )
        {
        return;
        }

      this->MainSlicerWindow->GetViewNotebook()->SetAlwaysShowTabs ( 0 );
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutLightboxView );
      }
*/
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureMainSliceViewers ( )
{
  if (this->SlicesGUI)
    {
    vtkSlicerSliceGUI *g = NULL;
    char *layoutname = NULL;
    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        g = this->SlicesGUI->GetFirstSliceGUI();
        layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        }
      else
        {
        g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        }
      
      switch (layoutname[0])
        {
        case 'R':
          g->GetSliceController()->GetSliceNode()->SetOrientationToAxial();
          break;
        case 'Y':
          g->GetSliceController()->GetSliceNode()->SetOrientationToSagittal();
          break;
        case 'G':
          g->GetSliceController()->GetSliceNode()->SetOrientationToCoronal();
          break;
        default:
          g->GetSliceController()->GetSliceNode()->SetOrientationToSagittal();
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddMainSliceViewerObservers ( )
{
  if (this->SlicesGUI)
    {
    vtkSlicerSliceGUI *g = NULL;
    char *layoutname = NULL;
    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        g = this->SlicesGUI->GetFirstSliceGUI();
        layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        }
      else
        {
        g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        }
      
      g->AddGUIObservers();
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::RemoveMainSliceViewerObservers ( )
{
  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      char *layoutname = NULL;
      int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          g = this->SlicesGUI->GetFirstSliceGUI();
          layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
          }
        else
          {
          g = this->SlicesGUI->GetNextSliceGUI(layoutname);
          layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
          }
        
        g->RemoveGUIObservers();
        }
    }
}

// to do: the three vtkSlicerSliceLogic pointers should be changed with a 
// pointer to a vtkSlicerSliceLogic map.
// current implementation only takes 3 for red, yellow, and green
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetAndObserveMainSliceLogic ( vtkSlicerSliceLogic *l0,
                                                            vtkSlicerSliceLogic *l1,
                                                            vtkSlicerSliceLogic *l2 )
{
        //if (this->SlicesGUI)
        //{
        //      vtkSlicerSliceGUI *g = NULL;
        //      char *layoutname = NULL;
        //      int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
        //      for (int i = 0; i < nSliceGUI; i++)
        //      {
        //              if (i == 0)
        //              {
        //                      g = this->SlicesGUI->GetFirstSliceGUI();
        //                      layoutname = this->SlicesGUI->GetFirstSliceGUILayoutName();
        //              }
        //              else
        //              {
        //                      g = this->SlicesGUI->GetNextSliceGUI(layoutname);
        //                      layoutname = this->SlicesGUI->GetNextSliceGUILayoutName(layoutname);
        //              }

        //              g->RemoveGUIObservers();
        //      }
        //}
  
  vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
  g->SetAndObserveModuleLogic(l0);
  g = this->SlicesGUI->GetSliceGUI("Yellow");
  g->SetAndObserveModuleLogic(l1);
  g = this->SlicesGUI->GetSliceGUI("Green");
  g->SetAndObserveModuleLogic(l2);
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

//    this->Script ("pack %s -side left  -expand 1 -fill both -padx 0 -pady 0", 
//    this->DefaultSlice0Frame->GetWidgetName( ) );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUIFrames ( )
{
  if ( this->GetApplication() != NULL ) {
    // pointers for convenience
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    
    if ( this->MainSlicerWindow != NULL ) {
      
      this->MainSlicerWindow->GetMainPanelFrame()->SetWidth ( geom->GetDefaultGUIPanelWidth() );
      this->MainSlicerWindow->GetMainPanelFrame()->SetHeight ( geom->GetDefaultGUIPanelHeight() );
      this->MainSlicerWindow->GetMainPanelFrame()->SetReliefToSunken();
      
      this->TopFrame->SetParent ( this->MainSlicerWindow->GetMainPanelFrame ( ) );
      this->TopFrame->Create ( );
      this->TopFrame->SetHeight ( geom->GetDefaultTopFrameHeight ( ) );
      
      this->LogoFrame->SetParent ( this->TopFrame );
      this->LogoFrame->Create( );
      this->LogoFrame->SetHeight ( geom->GetDefaultTopFrameHeight ( ) );            
      
      this->DropShadowFrame->SetParent ( this->MainSlicerWindow->GetMainPanelFrame() );
      this->DropShadowFrame->Create ( );
      // why is the theme not setting this???
      this->DropShadowFrame->SetBackgroundColor ( 0.9, 0.9, 1.0);
      
      this->SlicesControlFrame->SetParent ( this->DropShadowFrame );
      this->SlicesControlFrame->Create( );
      this->SlicesControlFrame->ExpandFrame ( );
      this->SlicesControlFrame->SetLabelText ( "Manipulate Slice Views");
      this->SlicesControlFrame->GetFrame()->SetHeight ( geom->GetDefaultSlicesControlFrameHeight ( ) );
      
      this->ViewControlFrame->SetParent ( this->DropShadowFrame );
      this->ViewControlFrame->Create( );
      this->ViewControlFrame->ExpandFrame ( );
      this->ViewControlFrame->SetLabelText ( "Manipulate 3D View" );
      this->ViewControlFrame->GetFrame()->SetHeight (geom->GetDefaultViewControlFrameHeight ( ) );
      
      
      app->Script ( "pack %s -side top -fill x -padx 1 -pady 1", this->TopFrame->GetWidgetName() );
      app->Script ( "pack %s -side left -expand 1 -fill x -padx 1 -pady 1", this->LogoFrame->GetWidgetName() );
      app->Script ( "pack %s -side bottom -expand n -fill x -padx 1 -ipady 1 -pady 0", this->DropShadowFrame->GetWidgetName() );
      app->Script ( "pack %s -side bottom -expand n -fill x -padx 0 -ipady 5 -pady 2", this->ViewControlFrame->GetWidgetName() );
      app->Script ( "pack %s -side bottom -expand n -fill x -padx 0 -ipady 5 -pady 1", this->SlicesControlFrame->GetWidgetName() );
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureRemoteIOSettings()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
    if ( scene != NULL )
      {
      //--- update CacheManager
      vtkCacheManager *cm = scene->GetCacheManager();
      if ( cm != NULL )
        {
        if ( strcmp (cm->GetRemoteCacheDirectory(), app->GetRemoteCacheDirectory() ))
          {
          cm->SetRemoteCacheDirectory (app->GetRemoteCacheDirectory() );
          }
        if ( cm->GetEnableForceRedownload() != app->GetEnableForceRedownload() )
          {
          cm->SetEnableForceRedownload (app->GetEnableForceRedownload() );
          }
//          if ( cm->GetEnableRemoteCacheOverwriting () != app->GetEnableRemoteCacheOverwriting() )
//          {
//          cm->SetEnableRemoteCacheOverwriting (app->GetEnableRemoteCacheOverwriting() );
//          }
        if ( cm->GetRemoteCacheLimit() != app->GetRemoteCacheLimit() )
          {
          cm->SetRemoteCacheLimit (app->GetRemoteCacheLimit() );
          }
        if ( cm->GetRemoteCacheFreeBufferSize() != app->GetRemoteCacheFreeBufferSize() )
          {
          cm->SetRemoteCacheFreeBufferSize (app->GetRemoteCacheFreeBufferSize() );
          }
        }
      //---- update DataIOManager
      vtkDataIOManager *dm = scene->GetDataIOManager();
      if ( dm != NULL )
        {
        if ( dm->GetEnableAsynchronousIO() != app->GetEnableAsynchronousIO() )
          {
          dm->SetEnableAsynchronousIO (app->GetEnableAsynchronousIO() );
          }
        }

      //---- update application settings interface if required...
      if ( this->GetMainSlicerWindow() != NULL )
        {
        if ( this->GetMainSlicerWindow()->GetApplicationSettingsInterface() != NULL )
          {
          (vtkSlicerApplicationSettingsInterface::SafeDownCast (this->GetMainSlicerWindow()->GetApplicationSettingsInterface()))->UpdateRemoteIOSettings();
          }
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateRemoteIOConfigurationForRegistry()
{

  vtkMRMLScene *scene = this->GetMRMLScene();
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
    if ( scene != NULL )
      {
      vtkCacheManager *cm = scene->GetCacheManager();
      if ( cm != NULL )
        {
        app->SetRemoteCacheDirectory (cm->GetRemoteCacheDirectory() );
        app->SetEnableForceRedownload (cm->GetEnableForceRedownload() );
        //app->SetEnableRemoteCacheOverwriting (cm->GetEnableRemoteCacheOverwriting() );
        app->SetRemoteCacheLimit (cm->GetRemoteCacheLimit() );
        app->SetRemoteCacheFreeBufferSize (cm->GetRemoteCacheFreeBufferSize() );
        }
      vtkDataIOManager *dm = scene->GetDataIOManager();
      if ( dm != NULL )
        {
        app->SetEnableAsynchronousIO (dm->GetEnableAsynchronousIO() );
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::MainSplitFrameConfigureCallback(int width, int height)
{
  // std::cout << "MainSplitFrameConfigureCallback" << std::endl;
  this->GUILayoutNode->SetMainPanelSize( this->MainSlicerWindow->GetMainSplitFrame()->GetFrame1Size() );
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SecondarySplitFrameConfigureCallback(int width, int height)
{
  // std::cout << "SecondarySplitFrameConfigureCallback" << std::endl;
  // Disable modified event to avoid a crash when dragging split frame
  // on a layout that only uses the top frame
  this->GUILayoutNode->DisableModifiedEventOn();
  this->GUILayoutNode->SetSecondaryPanelSize( this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1Size() );
  this->GUILayoutNode->DisableModifiedEventOff();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetIconImage (vtkKWIcon *icon, vtkImageData *image)
{
  int *dims = image->GetDimensions();
  int nComps = image->GetNumberOfScalarComponents();
  icon->SetImage( 
    static_cast <const unsigned char *> (image->GetScalarPointer()),
    dims[0], dims[1], nComps,
    dims[0] * dims[1] * nComps, vtkKWIcon::ImageOptionFlipVertical);
}

