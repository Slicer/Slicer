/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#include "vtkSlicerConfigure.h" /* Slicer_USE_* */

#ifdef Slicer_USE_QT

#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModulePanel.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulePanel.h"

// Qt includes
#include <QDebug>
#include <QRect>
#endif

#include <vtksys/SystemTools.hxx>
#include <itksys/SystemTools.hxx>

// VTK includes
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

// KWWidget includes
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
#include "vtkKWProgressGauge.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWSimpleEntryDialog.h"

// vtkSlicer includes
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
#include "vtkCacheManager.h"

#include "vtkSlicerFiducialListWidget.h"
#include "vtkSlicerROIViewerWidget.h"

#include "vtkHTTPHandler.h"
#include "vtkURIHandler.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkURIHandler.h"
#include "vtkHTTPHandler.h"

#ifdef Slicer_USE_PYTHON
#include "slicerPython.h"
#endif

// STD includes
#include <sstream>
#include <string>

// VTKSYS includes
#include <vtksys/stl/vector>
#include <vtksys/stl/string>
#include <vtksys/stl/map>
#include <vtksys/ios/fstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision$");
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

  typedef vtksys_stl::vector<vtkSlicerFiducialListWidget*> FiducialListWidgetContainerType;
  FiducialListWidgetContainerType FiducialListWidgets;
};

//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  )
{
  this->Internals = new vtkSlicerApplicationGUIInternals;

  this->MRMLScene = NULL;
  this->Built = false;
  this->DataCount = 0;

  this->UpdatingMain3DViewers = 0;

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
  this->PythonResult = NULL;
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

    // force the external progress helper to exit if it's running
    this->Script("if {[info exists extprog_fp]} {puts $extprog_fp exit; flush $extprog_fp}");

    this->SetApplication(NULL);
    this->SetApplicationLogic ( NULL );
    this->SetAndObserveMRMLScene (NULL);
    delete this->Internals;
    this->DataCount = 0;
}

vtkSlicerApplication*
vtkSlicerApplicationGUI::GetSlicerApplication()
{
  return vtkSlicerApplication::SafeDownCast(this->GetApplication());
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
  //-- do some null pointer checking.
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ProcessLoadSceneCommand: Got NULL MRMLScene." );
    return;
    }
  if ( this->MainSlicerWindow == NULL )
    {
    vtkErrorMacro ( "ProcessLoadSceneCommand: Got NULL SlicerWindow." );
    return;
    }
  if ( this->LoadSceneDialog == NULL )
    {
    vtkErrorMacro ( "ProcessLoadSceneCommand: Got NULL LoadSceneDialog." );
    return;
    }
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
#else
  if ( !this->LoadSceneDialog->IsCreated() )
    {
    this->LoadSceneDialog->SetParent ( this->MainSlicerWindow );
    this->LoadSceneDialog->SetMasterWindow( this->MainSlicerWindow );
    this->LoadSceneDialog->Create ( );
    this->LoadSceneDialog->SetFileTypes("{ {Scenes} {.mrml .xml .xcat} } { {MRML Scene} {.mrml} } { {Slicer2 Scene} {.xml} } { {Xcede Catalog} {.xcat} } { {All} {.*} }");
    }

  this->LoadSceneDialog->RetrieveLastPathFromRegistry("OpenPath");

  this->LoadSceneDialog->Invoke();
  // If a file has been selected for loading...
  const char *fileName = this->LoadSceneDialog->GetFileName();
  vtkKWProgressDialog *progressDialog = vtkKWProgressDialog::New();
  progressDialog->SetParent( this->MainSlicerWindow );
  progressDialog->SetMasterWindow( this->MainSlicerWindow );
  progressDialog->SetDisplayPositionToMasterWindowCenter();
  progressDialog->Create();
  progressDialog->ModalOn();

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
      dialog->SetMasterWindow( this->MainSlicerWindow );
      dialog->ModalOn();
      dialog->Invoke();
      dialog->Delete();
      }
    else
      {
      // Legacy scene did not save any view or camera: create one
      if (this->GetActiveViewerWidget() == NULL)
        {
        this->OnViewNodeNeeded();
        }
      }
    if (this->GetMRMLScene()->GetLastLoadedVersion() &&
        this->GetSlicerApplication()->GetSvnRevision() &&
        strcmp(this->GetMRMLScene()->GetLastLoadedVersion(),
               this->GetSlicerApplication()->GetSvnRevision()) > 0 )
      {
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent (  this->MainSlicerWindow );
      dialog->SetStyleToMessage();
      std::string msg = std::string("Warning: scene file ") + std::string( fileName) +
        std::string(" has version ") + std::string( this->GetMRMLScene()->GetLastLoadedVersion()) +
        std::string(" greater than Slicer3 version ") + std::string(this->GetSlicerApplication()->GetSvnRevision());
      dialog->SetText(msg.c_str());
      dialog->Create ( );
      dialog->SetMasterWindow( this->MainSlicerWindow );
      dialog->ModalOn();
      dialog->Invoke();
      dialog->Delete();
      }

    }

  //--- The scene may have a new interaction node which
  //--- specifies a Mouse Mode -- but no InteractionModeChangedEvent
  //--- will have been invoked for the GUI to capture.
  //--- So we invoke the event here after the scene is finished loading.
  if ( this->GetApplicationLogic() )
    {
    if ( this->GetApplicationLogic()->GetInteractionNode() )
      {
      this->GetApplicationLogic()->GetInteractionNode()->InvokeEvent (
                                                                      vtkMRMLInteractionNode::InteractionModeChangedEvent );
      }
    }

  progressDialog->SetParent(NULL);
  progressDialog->Delete();
#endif

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
  //-- do some null pointer checking.
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ProcessImportSceneCommand: Got NULL MRMLScene." );
    return;
    }
  if ( this->LoadSceneDialog == NULL )
    {
    vtkErrorMacro ( "ProcessImportSceneCommand: Got NULL LoadSceneDialog." );
    return;
    }
  if ( this->MainSlicerWindow == NULL )
    {
    vtkErrorMacro ( "ProcessImportSceneCommand: Got NULL SlicerWindow." );
    return;
    }
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openAddSceneDialog();
#else
  if ( !this->LoadSceneDialog->IsCreated() )
    {
    this->LoadSceneDialog->SetParent ( this->MainSlicerWindow );
    this->LoadSceneDialog->SetMasterWindow( this->MainSlicerWindow );
    this->LoadSceneDialog->Create ( );
    this->LoadSceneDialog->SetFileTypes("{ {Scenes} {.mrml .xml .xcat} } { {MRML Scene} {.mrml} } { {Slicer2 Scene} {.xml} } { {Xcede Catalog} {.xcat} } { {All} {.*} }");
    }

  this->LoadSceneDialog->RetrieveLastPathFromRegistry( "OpenPath");

  vtkKWProgressDialog *progressDialog = vtkKWProgressDialog::New();
  progressDialog->SetParent( this->MainSlicerWindow );
  progressDialog->SetMasterWindow( this->MainSlicerWindow );
  progressDialog->Create();
  progressDialog->ModalOn();

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
      dialog->SetMasterWindow( this->MainSlicerWindow );
      dialog->ModalOn();
      dialog->Invoke();
      dialog->Delete();
      }
    }
  progressDialog->SetParent(NULL);
  progressDialog->Delete();
#endif
  return;
}


//--- Note: add new methods for new sample data
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadDTIBrain()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/0/01/DTI-Brain.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadCTChest()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/3/31/CT-chest.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadCTACardio()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/0/00/CTA-cardio.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}
 
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadMRIHead()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/4/43/MR-head.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadRegistration1()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_1.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessDownloadRegistration2()
{
  std::string uri_String = "http://www.slicer.org/slicerWiki/images/e/e3/RegLib_C01_2.nrrd";
  this->DownloadSampleVolume (uri_String.c_str() );
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::DownloadSampleVolume(const char *uri)
{
  
  if ( uri == NULL )
    {
    vtkErrorMacro ( "Got NULL uri." );
    return;
    }
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "Got NULL MRML scene." );
    return;
    }
  if (this->GetSlicerApplication() == NULL )
    {
    vtkErrorMacro ( "Got NULL Slicer Application." );
    return;
    }
  if ( this->GetApplicationLogic() == NULL )
    {
    vtkErrorMacro ( "Got NULL Slicer Application Logic." );
    return;
    }
  if ( this->GetMRMLScene()->GetCacheManager()==NULL )
    {
    vtkErrorMacro ( "Got NULL CacheManager." );
      return;
    }
  if ( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory() == NULL )
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (  this->MainSlicerWindow );
    dialog->SetStyleToMessage();
    std::string msg = "Please set your Cache Directory to enable data download. This may be done through Slicer's application settings interface.";
    dialog->SetText(msg.c_str());
    dialog->Create ( );
    dialog->SetMasterWindow( this->MainSlicerWindow );
    dialog->ModalOn();
    dialog->Invoke();
    dialog->Delete();
    return;
    }
  std::string cacheDir = this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory();
  if ( cacheDir.empty() )
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (  this->MainSlicerWindow );
    dialog->SetStyleToMessage();
    std::string msg = "Please set your Cache Directory to enable data download. This may be done through Slicer's application settings interface.";
    dialog->SetText(msg.c_str());
    dialog->Create ( );
    dialog->SetMasterWindow( this->MainSlicerWindow );
    dialog->ModalOn();
    dialog->Invoke();
    dialog->Delete();
    return;
    }


  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());  
  std::string uri_String = uri;

  size_t slash = uri_String.find_last_of ( "/");
  if ( slash == std::string::npos )
    {
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (  this->MainSlicerWindow );
    dialog->SetStyleToMessage();
    std::string msg = "The uri string was poorly formed. Not able to download dataset.";
    dialog->SetText(msg.c_str());
    dialog->Create ( );
    dialog->SetMasterWindow( this->MainSlicerWindow );
    dialog->ModalOn();
    dialog->Invoke();
    dialog->Delete();
    return;
    }


  vtkSlicerModuleGUI *vgui = this->GetSlicerApplication()->GetModuleGUIByName("Volumes");
  if ( vgui )
    {
    const char *retval = this->GetSlicerApplication()->Script("set vlogic [$::slicer3::VolumesGUI GetLogic]");
    if (strcmp(retval, "0") != 0)
      {
      //--- Raise CacheAndRemoteDataIOGUI
      this->GetSlicerApplication()->Script ("$::slicer3::RemoteIOGUI DisplayManagerWindow");

      //---
      //--- Start volume download.
      //---
      std::string filename = vtksys::SystemTools::GetFilenameWithoutExtension ( uri_String );
      const char *retval2 = app->Script("[$::slicer3::VolumesGUI GetLogic] AddArchetypeVolume %s %s %d", uri_String.c_str(), filename.c_str(), 10 );
      if ( strcmp (retval2, "0") != 0)
        {
        //---
        //--- don't think we can
        //--- get the node back from tcl,
        //--- so find it by name
        //---
        int num = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLScalarVolumeNode");
        for ( int n=0; n <num; n++ )
          {
          vtkMRMLScalarVolumeNode *vnode = vtkMRMLScalarVolumeNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass (n,  "vtkMRMLScalarVolumeNode"));
          if ( vnode && vnode->GetName() != NULL )
            {
            if ( !(strcmp(vnode->GetName(), filename.c_str())) )
              {
              vtkMRMLScalarVolumeDisplayNode *dnode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(vnode->GetDisplayNode());
              if ( dnode )
                {
                dnode->SetAndObserveColorNodeID ( "vtkMRMLColorTableNodeGrey" );
                dnode->SetAutoWindowLevel ( 1 );
                dnode->SetAutoThreshold ( 1 );
                }
              if (this->GetApplicationLogic()->GetSelectionNode() != NULL )
                {
                this->GetApplicationLogic()->GetSelectionNode()->SetReferenceActiveVolumeID ( vnode->GetID() );
                }
              this->GetApplicationLogic()->PropagateVolumeSelection();
              break;
              }
            }
          }
        }
      else
        {
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent (  this->MainSlicerWindow );
        dialog->SetStyleToMessage();
        std::string msg = "Unable to access Volumes Logic to download dataset. No data downloaded.";
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->SetMasterWindow( this->MainSlicerWindow );
        dialog->ModalOn();
        dialog->Invoke();
        dialog->Delete();
        return;
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddDataCommand()
{
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openAddDataDialog();
#else
  this->GetApplication()->Script("::Loader::ShowDialog");
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddVolumeCommand()
{
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openAddVolumeDialog();
#else
  this->GetApplication()->Script("::LoadVolume::ShowDialog");
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddTransformCommand()
{
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openAddTransformDialog();
#else
  this->GetApplication()->Script("::LoadTransform::ShowDialog");
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessCloseSceneCommand()
{
  vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
  dialog->SetParent ( this->MainSlicerWindow );
  dialog->SetStyleToOkCancel();
  dialog->SetText("Are you sure you want to close the scene?");
  dialog->Create ( );
  dialog->SetMasterWindow( this->MainSlicerWindow );
  dialog->ModalOn();
  if (dialog->Invoke())
    {
    if (this->GetMRMLScene())
      {
      this->MRMLScene->Clear(false);
      this->OnViewNodeNeeded();
      }
    }
  dialog->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessMouseModeToggleCommand()
{
  vtkSlicerToolbarGUI *tgui = this->GetApplicationToolbar();
  if ( tgui == NULL )
    {
    vtkErrorMacro("ProcessMouseModeToggleCommand: no ToolbarGUI. can't toggle mouse mode with keypress.");
    return;
    }

  //TODO
  if ( this->GetApplicationLogic() == NULL )
    {
    vtkErrorMacro("ProcessMouseModeToggleCommand: no ApplicationLogic. can't toggle mouse mode with keypress.");
    return;
    }
  if ( this->GetApplicationLogic()->GetInteractionNode() == NULL )
    {
    vtkErrorMacro("ProcessMouseModeToggleCommand: can't find ApplicationLogic's Interaction Node. unable to toggle mouse mode with keypress.");
    return;
    }
  
  //--- Toggle Place mode from keyboard command.
  vtkMRMLInteractionNode *inode = this->GetApplicationLogic()->GetInteractionNode();
  int mode = inode->GetCurrentInteractionMode();
  if ( mode == vtkMRMLInteractionNode::Place )
    {
    inode->NormalizeAllMouseModes();
    inode->SetLastInteractionMode ( inode->GetCurrentInteractionMode() );
    inode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
    }
  else
    {
    inode->NormalizeAllMouseModes();
    inode->SetLastInteractionMode ( inode->GetCurrentInteractionMode() );
    inode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::Place );
    inode->SetPlaceModePersistence ( 1 );
    }
  
}


//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ProcessAddRulerCommand()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if (!app ||
      app->GetModuleGUIByName("Measurements") == NULL)
    {
    vtkErrorMacro("ProcessAddRulerCommand: no Measurements module, can't make a new ruler!");
    return;
    }
  vtkKWSimpleEntryDialog *dialog = vtkKWSimpleEntryDialog::New();
  dialog->SetParent ( this->MainSlicerWindow );
  dialog->SetText("Name the new ruler:");
  dialog->Create ( );
  dialog->SetMasterWindow( this->MainSlicerWindow );
  dialog->ModalOn();
  dialog->SetStyleToOkCancel();
  if (dialog->Invoke())
    {
    std::string str = dialog->GetEntry()->GetWidget()->GetValue();
    const char* retval;
    if (str.compare("") == 0)
      {
      retval = app->Script("[$::slicer3::MeasurementsGUI GetLogic] NewRulerBetweenFiducials \"\"");
      }
    else
      {
      retval = app->Script("[$::slicer3::MeasurementsGUI GetLogic] NewRulerBetweenFiducials %s", str.c_str());
      }
    if (strcmp(retval, "0") == 0)
      {
      // failed to make a ruler, give some feedback
      vtkErrorMacro("Ruler making failed, need to have two fiducials in a list to base it on.");
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->MainSlicerWindow );
      message->SetStyleToMessage();
      message->SetDialogName("WarningNoFiducials");
      message->SetText ("This option can only make a ruler from 2 fiducials, please add a couple of fiducials and then try again");
      message->Create();
      message->Invoke();
      message->Delete();
      }
    }
  dialog->Delete();
}

//---------------------------------------------------------------------------
const char* vtkSlicerApplicationGUI::GetCurrentLayoutStringName ( )
{
  if ( this->GetApplication() != NULL )
    {
    if ( this->GetGUILayoutNode() != NULL )
      {
      int layout = this->GetGUILayoutNode()->GetViewArrangement ();

      if ( layout == vtkMRMLLayoutNode::SlicerLayoutConventionalView)
        {
        return ( "Conventional layout" );
        }
      else if ( layout == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView )
        {
        return ( "Side-by-side Compare layout" );
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
      else if (layout == vtkMRMLLayoutNode::SlicerLayoutDual3DView )
        {
        return ( "Dual 3D layout" );
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
void vtkSlicerApplicationGUI::CustomizeStatusBarLayout()
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );

  // Move error log icon over to the left for MacOS
  // which positions every window's resize hardware right over the
  // default error icon position.
    if (this->MainSlicerWindow->GetTrayFrame() &&
        this->MainSlicerWindow->GetTrayFrame()->IsCreated() &&
        this->MainSlicerWindow->GetTrayFramePosition() ==
        vtkKWWindowBase::TrayFramePositionStatusFrame)
      {
      app->Script ( "%s configure -padx 15",
                    this->MainSlicerWindow->GetTrayFrame()->GetWidgetName() );
      app->Script ( "%s configure -background white",
                    this->MainSlicerWindow->GetTrayFrame()->GetWidgetName() );
      //--- if TrayImageError (a label) is made public and exposed thru the KWW API,
      //--- we can use this. Until that, we will let it be centered in the Tray Frame,
      //--- and its image/icon centered too.
//      app->Script ( "%s configure -anchor w",
//                    this->MainSlicerWindow->GetTrayImageError()->GetWidgetName() );

      }
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
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer (vtkMRMLLayoutNode::SlicerLayoutInitialView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutInitialView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutConventionalView &&
      this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutConventionalView )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer (vtkMRMLLayoutNode::SlicerLayoutConventionalView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutConventionalView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutDual3DView &&
      this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutDual3DView )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer (vtkMRMLLayoutNode::SlicerLayoutDual3DView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutDual3DView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUp3DView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUp3DView )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUp3DView, NULL);
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUp3DView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutFourUpView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutFourUpView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutFourUpView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutFourUpView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutTabbed3DView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutTabbed3DView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutTabbed3DView );
    //TEST
    this->ApplicationToolbar->ResumeViewRockOrSpin(mode);
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView, "Yellow");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView &&
            this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView, "Green");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView &&
             this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );
    }
  else if ( target == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView &&
             this->GetCurrentLayout()!= vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView)
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView, "Red");
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView );
    }
  else if ( (target == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView ) )
    {
    // TO DO
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView, NULL );
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView);
    }
  else if ( (target == vtkMRMLLayoutNode::SlicerLayoutCompareView) )
    {
#ifndef TOOLBAR_DEBUG
    mode = this->ApplicationToolbar->StopViewRockOrSpin();
#endif
    this->RepackMainViewer ( vtkMRMLLayoutNode::SlicerLayoutCompareView, NULL);
    this->SetCurrentLayout ( vtkMRMLLayoutNode::SlicerLayoutCompareView );
    }
  else if ( (target == vtkMRMLLayoutNode::SlicerLayoutNone) )
    {
    this->UnpackMainViewer();
    this->SetCurrentLayout ( target == vtkMRMLLayoutNode::SlicerLayoutNone );
    }

  // update the collapse state of the slice controllers
  if (this->SlicesGUI)
    {
    vtkSlicerSliceGUI *g = NULL;
    const char *layoutname = NULL;
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
     
      if (this->GUILayoutNode->GetCollapseSliceControllers())
        {
        g->GetSliceController()->Shrink();
        }
      else
        {
        g->GetSliceController()->Expand();
        }
      }
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

    if ( this->GetApplication() != NULL ) 
      {
      vtkSlicerApplication *app =(vtkSlicerApplication *)this->GetApplication();
      layout->SetNumberOfCompareViewRows( app->GetApplicationLayoutCompareViewRows() );
      layout->SetNumberOfCompareViewColumns( app->GetApplicationLayoutCompareViewColumns() );
      layout->SetNumberOfCompareViewLightboxRows( app->GetApplicationLayoutLightboxRows() );
      layout->SetNumberOfCompareViewLightboxColumns( app->GetApplicationLayoutLightboxColumns() );
      }
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
#ifdef Slicer_USE_QT
  qSlicerApplication::application()->ioManager()->openSaveDataDialog();
#else
  this->SaveDataWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
  this->SaveDataWidget->AddObserver ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataWidget->SetVersion(this->GetSlicerApplication()->GetSvnRevision());
  this->SaveDataWidget->Invoke();

  this->SaveDataWidget->RemoveObservers ( vtkSlicerMRMLSaveDataWidget::DataSavedEvent,  (vtkCommand *)this->GUICallbackCommand );
  return;
#endif
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
    this->ModulesWizardDialog->SetParent( this->MainSlicerWindow );
    this->ModulesWizardDialog->SetMasterWindow( this->MainSlicerWindow );
    this->ModulesWizardDialog->SetApplication( this->GetApplication() );
    this->ModulesWizardDialog->Create();
    this->ModulesWizardDialog->ModalOn();
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
  this->MainSlicerWindow->SetBinding("<Configure>", this, "ConfigureCallback %W");
  this->MainSlicerWindow->SetBinding("<Unmap>", this, "UnMapCallback %W");
  this->MainSlicerWindow->SetBinding("<Map>", this, "MapCallback %W");

#ifdef Slicer_USE_QT
  this->MainSlicerWindow->AddObserver(vtkKWTopLevel::SlaveDisplayEvent, (vtkCommand*)this->GUICallbackCommand);
  this->MainSlicerWindow->AddObserver(vtkKWTopLevel::SlaveWithdrawEvent, (vtkCommand*)this->GUICallbackCommand);
#endif
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
            const char *layoutname = NULL;
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
  this->MainSlicerWindow->RemoveBinding("<Configure>", this, "ConfigureCallback %W");
  this->MainSlicerWindow->RemoveBinding("<UnMap>", this, "UnMapCallback %W");
  this->MainSlicerWindow->RemoveBinding("<Map>", this, "MapCallback %W");

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
    const char *layoutname = NULL;
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

  vtkKWLoadSaveDialog::SafeDownCast(caller);
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
  vtkSlicerMRMLSaveDataWidget *saveDataWidget = vtkSlicerMRMLSaveDataWidget::SafeDownCast(caller);

  // catch changes to ApplicationSettings, and update the ApplicationSettingsInterface
  if ( event == vtkCommand::ModifiedEvent && vtkSlicerApplication::SafeDownCast(caller) == app )
    {
    this->MainSlicerWindow->GetApplicationSettingsInterface()->Update();
    }

  // Event observer is added in Slicer3.cxx
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
      int nb_fid_viewer_widgets = this->GetNumberOfFiducialListWidgets();
      for (int i = 0; i < nb_fid_viewer_widgets; ++i)
        {
        vtkSlicerFiducialListWidget *fidlist_widget = this->GetNthFiducialListWidget(i);
        if (fidlist_widget &&
            fidlist_widget->GetViewNode() &&
            fidlist_widget->GetViewNode()->GetName() &&
            !strcmp(page_title, fidlist_widget->GetViewNode()->GetName()))
          {
          fidlist_widget->GetViewNode()->SetActive(1);
          break;
          }
        }
      }
    }

  if (saveDataWidget == this->SaveDataWidget && event == vtkSlicerMRMLSaveDataWidget::DataSavedEvent)
    {
    }
#ifdef Slicer_USE_QT

  if (event == vtkKWTopLevel::SlaveDisplayEvent)
    {
    qDebug() << "vtkSlicerApplicationGUI: display";
    qSlicerApplication::application()->setTopLevelWidgetsVisible(false);
    }
  else if (event == vtkKWTopLevel::SlaveWithdrawEvent)
    {
    qDebug() << "vtkSlicerApplicationGUI: withdraw";
    qSlicerApplication::application()->setTopLevelWidgetsVisible(true);
    }
#endif
  if (caller)
    {
    if (event == vtkSlicerSliceControllerWidget::ExpandEvent)
      {
      this->GUILayoutNode->SetCollapseSliceControllers(0);
      }
    else if (event == vtkSlicerSliceControllerWidget::ShrinkEvent)
      {
      this->GUILayoutNode->SetCollapseSliceControllers(1);
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
void vtkSlicerApplicationGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                                   unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
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
#ifndef TOOLBAR_DEBUG
    this->ApplicationToolbar->UpdateLayoutMenu();
#endif
    this->UpdateLayout();
    if (ln->GetSelectedModule( ))
      {
      this->SelectModule ( ln->GetSelectedModule( ) );
      }
    }

  if (event == vtkMRMLViewNode::ActiveModifiedEvent)
    {
    this->UpdateActiveViewerWidgetDependencies(this->GetActiveViewerWidget());
    }

  if (scene != NULL &&
      scene == this->MRMLScene &&
      event == vtkCommand::ModifiedEvent )
    {
    // std::cout << "Scene modified" << std::endl;
#ifndef TOOLBAR_DEBUG
    if(this->ApplicationToolbar)
      {
      this->ApplicationToolbar->UpdateLayoutMenu();
      }
#endif
    // do not update the layout on every scene modified. only update
    // the layout when the layout node is modified (first case) or
    // when switching to a new layout node (third case)
    }
  else if (scene != NULL &&
           scene == this->MRMLScene &&
           event == vtkMRMLScene::SceneAboutToBeClosedEvent )
    {
    this->SceneClosing = true;
    // scene closing, let's try to release all the dependencies
    this->UpdateActiveViewerWidgetDependencies(NULL);
    }
  else if (scene != NULL &&
           scene == this->MRMLScene &&
           event == vtkMRMLScene::SceneClosedEvent )
    {
    // is the scene closing?
    this->SceneClosing = true;
    //-- todo: is this right?
    //    this->SetAndObserveGUILayoutNode ( NULL );
    // scene closed, let's update the layout
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
  else if (event == vtkMRMLViewNode::VisibilityEvent)
    {
    // a viewer may need to be added or removed from the screen
    this->UpdateMain3DViewers();
    }
  else if ( scene != NULL &&
            scene == this->MRMLScene &&
            event == vtkMRMLScene::ImportProgressFeedbackEvent )
    {
    this->UpdateLoadStatusText();
    }
  else if ( scene != NULL &&
            scene == this->MRMLScene &&
            event == vtkMRMLScene::SaveProgressFeedbackEvent )
    {
    this->UpdateSaveStatusText();
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
#ifndef TOOLBAR_DEBUG
      this->ApplicationToolbar->UpdateLayoutMenu();
#endif
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
void vtkSlicerApplicationGUI::UpdateLoadStatusText()
{
  vtkSlicerWindow *win = this->GetMainSlicerWindow();
  const char *upText = "Adding data";

  if ( win != NULL )
    {
    std::string txt = win->GetStatusText ();
    if ( txt.find(upText) != std::string::npos )
      {
      txt.append ( "..." );
      win->SetStatusText ( txt.c_str() );
      }
    else
      {
      win->SetStatusText ( upText );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateSaveStatusText()
{
  vtkSlicerWindow *win = this->GetMainSlicerWindow();
  const char *upText = "Saving data";

  if ( win != NULL )
    {
    std::string txt = win->GetStatusText ();
    if ( txt.find(upText) != std::string::npos )
      {
      txt.append ( "..." );
      win->SetStatusText ( txt.c_str() );
      this->Script ( "update idletasks" );
      }
    else
      {
      win->SetStatusText ( upText );
      this->Script ( "update idletasks" );
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SelectModuleForNode ( vtkMRMLNode *node )
{
  const char *moduleName = NULL;
  if (node->IsA("vtkMRMLVolumeNode"))
    {
    moduleName = "Volumes";
    }
  else if (node->IsA("vtkMRMLFiberBundleNode"))
    {
    moduleName = "FiberBundles";
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
  else if (node->IsA("vtkMRMLColorNode"))
    {
    moduleName = "Color";
    }
  else if (node->IsA("vtkMRMLCameraNode"))
    {
    // disable this for now, as swapping views when select a new view from the
    //View drop down in the Cameras gui isn't working
    //moduleName = "Cameras";
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

    if ( this->GetApplication() != NULL ) 
      {
      vtkSlicerApplication *app =(vtkSlicerApplication *)this->GetApplication();
      layout->SetNumberOfCompareViewRows( app->GetApplicationLayoutCompareViewRows() );
      layout->SetNumberOfCompareViewColumns( app->GetApplicationLayoutCompareViewColumns() );
      layout->SetNumberOfCompareViewLightboxRows( app->GetApplicationLayoutLightboxRows() );
      layout->SetNumberOfCompareViewLightboxColumns( app->GetApplicationLayoutLightboxColumns() );
      // view arrangement will be set later
      }
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

  // Observe an event that updates the status text during loading of data.

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::ImportProgressFeedbackEvent);
  events->InsertNextValue(vtkMRMLScene::SaveProgressFeedbackEvent);
  events->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue( vtkMRMLScene::NodeRemovedEvent );
  events->InsertNextValue( vtkMRMLScene::SceneClosedEvent );
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  this->SetAndObserveMRMLSceneEvents (this->MRMLScene, events );
  events->Delete();

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
#ifndef TOOLBAR_DEBUG
  this->ApplicationToolbar->SetLayoutMenubuttonValueToLayout ( app->GetApplicationLayoutType() );
#endif
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

  //---
  //--- Load sample data
  //---
  vtkKWMenu *cm = vtkKWMenu::New();
  cm->SetParent ( this->GetMainSlicerWindow()->GetFileMenu() );
  cm->Create();
  //--- add all instances of sample data here
  cm->AddCommand ( "MRI Head", this, "ProcessDownloadMRIHead" );
  cm->AddCommand ( "DTI Brain", this, "ProcessDownloadDTIBrain" );
  cm->AddCommand ( "CT Chest", this, "ProcessDownloadCTChest" );
  cm->AddCommand ( "CT Angiogram (Cardio)", this, "ProcessDownloadCTACardio" );
  cm->AddCommand ( "MRI Brain (Meningioma) Time 1", this, "ProcessDownloadRegistration1" );
  cm->AddCommand ( "MRI Brain (Meningioma) Time 2", this, "ProcessDownloadRegistration2" );
  i = this->GetMainSlicerWindow()->GetFileMenu()->InsertCascade (
                                                              this->GetMainSlicerWindow()->GetFileMenuInsertPosition(),
                                                              "Download Sample Data", cm );
  cm->Delete();


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


  // commenting out Undo and Redo options for now: 03/03/2009
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

#ifdef Slicer_USE_PYTHON
  i = this->MainSlicerWindow->GetWindowMenu()->AddCommand ( "Python Interactor", NULL, "$::slicer3::ApplicationGUI PythonConsole" );
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

  // make a new measurements ruler between the last two fiducials on the
  // currently active list, only works if the measurements module is loaded.
  i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "New Measurement Between Fiducials...", this, "ProcessAddRulerCommand");
  this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+M");
  this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);
  
  // Toggle among mouse modes.
  i = this->MainSlicerWindow->GetEditMenu()->AddCommand ( "Toggle 'Persistent Place' Interaction Mode...", this, "ProcessMouseModeToggleCommand");
  this->MainSlicerWindow->GetEditMenu()->SetItemAccelerator ( i, "Ctrl+I");
  this->MainSlicerWindow->GetEditMenu()->SetBindingForItemAccelerator ( i, this->MainSlicerWindow);

  //
  // View Menu
  //

  this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                                                             this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                                             "Cache & Remote I/O Manager", NULL, "$::slicer3::RemoteIOGUI DisplayManagerWindow");

  i = this->GetMainSlicerWindow()->GetViewMenu()->InsertCommand (
                                                                 this->GetMainSlicerWindow()->GetViewMenuInsertPosition(),
                                                                 "Extension Manager", this, "ShowModulesWizard");
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


#endif
  }
  this->Built = true;
  this->CustomizeStatusBarLayout();
//  this->UpdateLayout();
#ifndef TOOLBAR_DEBUG
//  this->ApplicationToolbar->UpdateLayoutMenu();
#endif

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

#ifdef Slicer_USE_PYTHON
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
    PyObject *exception, *v, *tb;
    PyObject *exception_s, *v_s, *tb_s;

    PyErr_Fetch(&exception, &v, &tb);
    if (exception == NULL)
      return;
    PyErr_NormalizeException(&exception, &v, &tb);
    if (exception == NULL)
      return;
    
    exception_s = PyObject_Str(exception);
    v_s = PyObject_Str(v);
    tb_s = PyObject_Str(tb);
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", "Failed to startup python interpreter" );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(exception_s) );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(v_s) );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(tb_s) );
    Py_DECREF ( exception_s );
    Py_DECREF ( v_s );
    Py_DECREF ( tb_s );
    Py_DECREF ( exception );
    Py_DECREF ( v );
    if ( tb )
      {
      Py_DECREF ( tb );
      }
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
void vtkSlicerApplicationGUI::PythonCommand ( const char *cmd )
{

#ifdef Slicer_USE_PYTHON
  PyObject* d =
    (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary());
  if ( d == NULL )
    {
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", "Failed to startup python interpreter (command): dictionary null" );
    return;
    }

  // Note: cmdString is not currently used - calls with cmd directly
  std::string cmdString =  std::string ( "import sys;\n" );
              cmdString += std::string ( "try:\n" );
              cmdString += std::string ( "  ") + std::string( cmd ) + std::string ( ";\n" );
              cmdString += std::string ( "except Exception, e:\n" );
              cmdString += std::string ( "  print 'Failed to run command ', e\n" );
              cmdString += std::string ( "sys.stdout.flush();\n" );
              cmdString += std::string ( "sys.stderr.flush();\n" );

  PyObject* v = PyRun_String ( cmd, Py_file_input, d, d);


  if (v == NULL)
    {
    PyObject *exception, *v, *tb;
    PyObject *exception_s, *v_s, *tb_s;

    PyErr_Fetch(&exception, &v, &tb);
    if (exception == NULL)
      return;
    PyErr_NormalizeException(&exception, &v, &tb);
    if (exception == NULL)
      return;
    
    exception_s = PyObject_Str(exception);
    v_s = PyObject_Str(v);
    tb_s = PyObject_Str(tb);
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", "Python Fail" );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(exception_s) );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(v_s) );
    vtkSlicerApplication::GetInstance()->RequestDisplayMessage ( "Error", PyString_AS_STRING(tb_s) );
    Py_DECREF ( exception_s );
    Py_DECREF ( v_s );
    Py_DECREF ( tb_s );
    Py_DECREF ( exception );
    Py_DECREF ( v );
    if ( tb )
      {
      Py_DECREF ( tb );
      }

    return;
    }
  else
    {
    PyObject *v_s;
    v_s = PyObject_Str(v);
    this->SetPythonResult(PyString_AS_STRING(v_s));
    Py_DECREF(v_s);
    }

  Py_DECREF ( v );
  if (Py_FlushLine())
    {
    PyErr_Clear();
    }
#else
  (void)(cmd); // To avoid 'unused variable warning'
  vtkErrorMacro("Python is not available in this build.");
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
    app->OpenLink ("http://www.slicer.org/slicerWiki/index.php/Slicer3.4:Training" );
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
      const char *layoutname = NULL;
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
    this->MainSlicerWindow->Display ( );
//     int w = this->MainSlicerWindow->GetWidth ( );
//     int h = this->MainSlicerWindow->GetHeight ( );
//     int vh = app->GetDefaultGeometry()->GetDefault3DViewerHeight();
//     int sh = app->GetDefaultGeometry()->GetDefaultSliceGUIFrameHeight();
//     int sfh = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame1Size();
//     int sf2h = this->MainSlicerWindow->GetSecondarySplitFrame()->GetFrame2Size();

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
    this->GridFrame1->SetParent ( this->MainSlicerWindow->GetViewFrame ( ) );
    this->GridFrame1->Create ( );
    this->GridFrame2->SetParent ( this->MainSlicerWindow->GetSecondaryPanelFrame ( ) );
    this->GridFrame2->Create ( );
    this->CreateMainSliceViewers ( );
    this->UpdateMain3DViewers ( );
    this->OnViewNodeNeeded();
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
  if (this->GetApplication() == NULL || !this->MRMLScene || this->UpdatingMain3DViewers)
    {
    return;
    }
  this->UpdatingMain3DViewers = 1;

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  //vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );

  // Create 3D viewer for the view nodes

  vtksys_stl::map<vtkMRMLViewNode*, int> view_nodes;

  int nb_added = 0;
  int nb_fidlist_added = 0;
  vtkMRMLViewNode *node = NULL;
  int n, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLViewNode");
  for (n = 0; n < nnodes; n++)
    {
    node = vtkMRMLViewNode::SafeDownCast (
      this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLViewNode"));
    if (node && node->GetVisibility())
      {
      view_nodes[node] = 1;
      if (!this->GetViewerWidgetForNode(node))
        {
        vtkSlicerViewerWidget *viewer_widget = vtkSlicerViewerWidget::New();
        viewer_widget->SetApplication(app);
        viewer_widget->SetParent(this->MainSlicerWindow);
        viewer_widget->SetAndObserveViewNode(node);
        viewer_widget->SetMRMLScene(this->MRMLScene);
        viewer_widget->Create();
        viewer_widget->UpdateFromMRML();
        viewer_widget->SetApplicationLogic(this->GetApplicationLogic());
        this->Internals->ViewerWidgets.push_back(viewer_widget);
        node->InvokeEvent(
          vtkMRMLViewNode::GraphicalResourcesCreatedEvent, viewer_widget);
        nb_added++;
        }
      if (!this->GetFiducialListWidgetForNode(node))
        {
        vtkSlicerFiducialListWidget *fidlist_widget = vtkSlicerFiducialListWidget::New();
        fidlist_widget->SetApplication(app);
        fidlist_widget->SetParent(this->MainSlicerWindow);
        fidlist_widget->SetAndObserveViewNode(node);
        fidlist_widget->SetMRMLScene(this->MRMLScene);
        fidlist_widget->Create();
        vtkSlicerViewerWidget *active_viewer = this->GetViewerWidgetForNode(node);
        fidlist_widget->SetViewerWidget(active_viewer);
        fidlist_widget->SetInteractorStyle(active_viewer ? vtkSlicerViewerInteractorStyle::SafeDownCast(active_viewer->GetMainViewer()->GetRenderWindowInteractor()->GetInteractorStyle()) : NULL);
        fidlist_widget->UpdateFromMRML();
//        fidlist_widget->SetApplicationLogic(this->GetApplicationLogic());
        this->Internals->FiducialListWidgets.push_back(fidlist_widget);
        node->InvokeEvent(
          vtkMRMLViewNode::GraphicalResourcesCreatedEvent, fidlist_widget);
        nb_fidlist_added++;
        }
      }
    }

  // Remove 3D viewers that have no nodes
  // First pass, remove them from the pool so that they are not found
  // by GetActiveViewerWidget; they still exist so that their
  // reference count can go down as they get released by the fiducials or
  // ROI below. The second pass will delete them later in this method.

  vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType
    view_widget_to_delete;
  int done;
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
        view_widget_to_delete.push_back(viewer_widget);
        viewer_widget->RemoveMRMLObservers();
        this->Internals->ViewerWidgets.erase(it);
        done = 0;
        break;
        }
      }
    } while (!done);

  // clean up the fiducial list widgets
  vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType
    fidlist_widget_to_delete;
  int fid_done; 
  do
    {
    fid_done = 1;
    vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator it =
      this->Internals->FiducialListWidgets.begin();
    vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator end =
      this->Internals->FiducialListWidgets.end();
    for (; it != end; ++it)
      {
      vtkSlicerFiducialListWidget *fidlist_widget = (*it);
      if (fidlist_widget &&
          view_nodes.find(fidlist_widget->GetViewNode()) == view_nodes.end())
        {
        fidlist_widget_to_delete.push_back(fidlist_widget);
        fidlist_widget->RemoveMRMLObservers();
        this->Internals->FiducialListWidgets.erase(it);
        fid_done = 0;
        break;
        }
      }
    } while (!fid_done);
  
  // Add the roi widget

  if (!this->ROIViewerWidget)
    {
    this->ROIViewerWidget = vtkSlicerROIViewerWidget::New();
    this->ROIViewerWidget->SetApplication( app );
    this->ROIViewerWidget->SetMRMLScene(this->MRMLScene);
    this->ROIViewerWidget->Create();
    }

  // Update the dependencies. Order is important, leave it here, so that
  // reference counts can go down (viewer widgets get partly release here).
  this->UpdateActiveViewerWidgetDependencies(this->GetActiveViewerWidget());

  
  // Second pass delete them

  // remove fiducial widget with no nodes (do this before the viewer widgets!)
  vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator fid_it =
    fidlist_widget_to_delete.begin();
  vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator fid_end =
    fidlist_widget_to_delete.end();
  for (; fid_it != fid_end; ++fid_it)
    {
    vtkSlicerFiducialListWidget *fidlist_widget = (*fid_it);
//    fidlist_widget->SetApplicationLogic(NULL);
    fidlist_widget->SetParent(NULL);
    fidlist_widget->Delete();
    }

  // Remove 3D viewers that have no nodes
  vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator it =
    view_widget_to_delete.begin();
  vtkSlicerApplicationGUIInternals::ViewerWidgetContainerType::iterator end =
    view_widget_to_delete.end();
  for (; it != end; ++it)
    {
    vtkSlicerViewerWidget *viewer_widget = (*it);
    viewer_widget->SetApplicationLogic(NULL);
    viewer_widget->SetParent(NULL);
    viewer_widget->Delete();
    }

 this->UpdatingMain3DViewers = 0;

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::OnViewNodeNeeded()
{
  if (this->GetApplication() == NULL || !this->MRMLScene)
    {
    return;
    }

  // No visible 3D view node, let's add one for convenience
  int nnodes = GetNumberOfVisibleViewNodes();
  if (nnodes<1)
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
  view_node->AddObserver(vtkMRMLViewNode::VisibilityEvent,
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
  view_node->RemoveObservers(vtkMRMLViewNode::VisibilityEvent,
                             this->MRMLCallbackCommand );

  this->UpdateMain3DViewers();
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateActiveViewerWidgetDependencies(
  vtkSlicerViewerWidget *active_viewer)
{
  this->ROIViewerWidget->SetMainViewerWidget(active_viewer);
  this->ROIViewerWidget->UpdateFromMRML();

  // TODO this should be handled in a generic way for all modules.
  vtkSlicerModuleGUI *FEMeshModule = this->GetSlicerApplication()->GetModuleGUIByName("IA_FEMesh");
  if (FEMeshModule)
    {
    FEMeshModule->SetActiveViewer(active_viewer);
    }

  vtkSlicerModuleGUI *MeasurementsModule = this->GetSlicerApplication()->GetModuleGUIByName("Measurements");
  if (MeasurementsModule)
    {
    MeasurementsModule->SetActiveViewer(active_viewer);
    }

  vtkSlicerModuleGUI *ColorModule = this->GetSlicerApplication()->GetModuleGUIByName("Color");
  if (ColorModule)
    {
    ColorModule->SetActiveViewer(active_viewer);
    }

  
#ifndef VIEWCONTROL_DEBUG
  vtkSlicerViewControlGUI *vcGUI = this->GetViewControlGUI ( );
  if (vcGUI != NULL)
    {
    if (active_viewer != NULL)
      {
      vcGUI->SetViewNode( active_viewer->GetViewNode());
      }
    else
      {
      vcGUI->SetViewNode(NULL);
      }
    }
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
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    if ( layout == NULL )
      {
      return;
      }

    // Destroy fiducial list
    vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator fid_it =
      this->Internals->FiducialListWidgets.begin();
    vtkSlicerApplicationGUIInternals::FiducialListWidgetContainerType::iterator fid_end =
      this->Internals->FiducialListWidgets.end();
    for (; fid_it != fid_end; ++fid_it)
      {
      vtkSlicerFiducialListWidget *fidlist_widget = (*fid_it);
      if (fidlist_widget)
        {
        fidlist_widget->RemoveMRMLObservers();
//        fidlist_widget->SetApplicationLogic(NULL);
        fidlist_widget->SetParent(NULL);
        fidlist_widget->Delete();
        }
      }
    this->Internals->FiducialListWidgets.clear();
    
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
    if (viewer_widget && viewer_widget->GetViewNode()->GetActive() && viewer_widget->GetViewNode()->GetVisibility())
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

//---------------------------------------------------------------------------
int vtkSlicerApplicationGUI::GetNumberOfFiducialListWidgets()
{
  return (int)this->Internals->FiducialListWidgets.size();
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget* vtkSlicerApplicationGUI::GetNthFiducialListWidget(int idx)
{
  if (idx < 0 || idx >= this->GetNumberOfFiducialListWidgets())
    {
    return NULL;
    }

  return this->Internals->FiducialListWidgets[idx];
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget* vtkSlicerApplicationGUI::GetFiducialListWidgetForNode(
  vtkMRMLViewNode *node)
{
  int nb_fidlist_widgets = this->GetNumberOfFiducialListWidgets();
  for (int i = 0; i < nb_fidlist_widgets; ++i)
    {
    vtkSlicerFiducialListWidget *fidlist_widget = this->GetNthFiducialListWidget(i);
    if (fidlist_widget && fidlist_widget->GetViewNode() == node)
      {
      return fidlist_widget;
      }
    }

  return NULL;
}

//---------------------------------------------------------------------------
vtkSlicerFiducialListWidget* vtkSlicerApplicationGUI::GetActiveFiducialListWidget()
{
  int nb_fidlist_widgets = this->GetNumberOfFiducialListWidgets();
  for (int i = 0; i < nb_fidlist_widgets; ++i)
    {
    vtkSlicerFiducialListWidget *fidlist_widget = this->GetNthFiducialListWidget(i);
    if (fidlist_widget && fidlist_widget->GetViewNode()->GetActive() && fidlist_widget->GetViewNode()->GetVisibility())
      {
      return fidlist_widget;
      }
    }

  // no active found, alright, use first one, if any
  // Legacy support, the active flag on the vtkMRMLViewNode was saved, but not
  // used. Sadly, it was saved as "false", which means that now that this flag
  // is supported, snapshots start disabling the view! Try to work around this.
  return this->GetNthFiducialListWidget(0);
}

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
        case vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView:
          this->PackSideBySideLightboxView();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutCompareView:
          this->PackCompareView();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutDual3DView:
          this->PackDual3DView();
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
        case vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView:
          this->UnpackSideBySideLightboxView();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutCompareView:
          this->UnpackCompareView();
          break;
        case vtkMRMLLayoutNode::SlicerLayoutDual3DView:
          this->UnpackDual3DView();
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
    if (g) g->GridGUI( this->GridFrame2, 0, 0 );

    //--- yellow slice viewer
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    if (g) g->GridGUI( this->GridFrame2, 0, 1 );

    //--- green slice viewer
    g = this->SlicesGUI->GetSliceGUI("Green");
    if (g) g->GridGUI( this->GridFrame2, 0, 2 );


#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

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
void vtkSlicerApplicationGUI::PackDual3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
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

    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame1->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid columnconfigure %s 1 -weight 1", this->GridFrame1->GetWidgetName() );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame1->GetWidgetName() );


    // Do we need to make another 3D viewer node?
    if (GetNumberOfVisibleViewNodes() < 2)
      {
      // Need another view node.  When the view node is added to the
      // scene, a viewer widget will be constructed automatically
      vtkMRMLViewNode *second = vtkMRMLViewNode::New();
      this->MRMLScene->AddNode(second);
      second->Delete();
      this->OnViewNodeAdded(second);
      }

    // Pack the 3D viewers
    for (size_t i=0; i < (this->Internals->ViewerWidgets.size() > 2 ? 2 : this->Internals->ViewerWidgets.size()); i++)
      {
      vtkSlicerViewerWidget *viewer_widget
        = this->Internals->ViewerWidgets[i];
      if (viewer_widget)
        {
        viewer_widget->GridWidget(this->GridFrame1, 0, i);
        }
      }


    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame2->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid columnconfigure %s 1 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid columnconfigure %s 2 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );

    //--- red slice viewer
    g = this->SlicesGUI->GetSliceGUI("Red");
    if (g) g->GridGUI( this->GridFrame2, 0, 0 );

    //--- yellow slice viewer
    g = this->SlicesGUI->GetSliceGUI("Yellow");
    if (g) g->GridGUI( this->GridFrame2, 0, 1 );

    //--- green slice viewer
    g = this->SlicesGUI->GetSliceGUI("Green");
    if (g) g->GridGUI( this->GridFrame2, 0, 2 );


#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

    // finally, modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility(1);
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutDual3DView)
      {
      layout->SetViewArrangement ( vtkMRMLLayoutNode::SlicerLayoutDual3DView );
      }
    layout->DisableModifiedEventOff();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackOneUp3DView ( )
{
  if ( this->GetApplication() != NULL )
    {
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

#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

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

#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

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

#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

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
void vtkSlicerApplicationGUI::PackSideBySideLightboxView()
{

    if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
    vtkMRMLLayoutNode *layout = this->GetGUILayoutNode ( );
    double x, y, z;

    // Hide the top panel
    this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame2Visibility(0);

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
    if (g->GetSliceNode())
      {
      x = g->GetSliceNode()->GetFieldOfView()[0];
      y = g->GetSliceNode()->GetFieldOfView()[1];
      z = g->GetSliceNode()->GetFieldOfView()[2];
      g->GetSliceNode()->SetFieldOfView(x, y, z);
      g->GetSliceNode()->UpdateMatrices();
      }

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
      vtkSlicerSliceGUI *g0 = this->SlicesGUI->GetSliceGUI(buf);
      if (g0 != NULL)
        {
        //--- go thru node to set lightbox rows and columns
        if ( g0->GetLogic() != NULL )
          {
          if ( g0->GetLogic()->GetSliceNode() != NULL )
            {
            g0->GetLogic()->GetSliceNode()->SetLayoutGrid ( numRows, numColumns );
            }
          }
        }
      }

    // configure the new layout
    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame2->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );
    this->Script ("grid rowconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );

    const char *layoutname = NULL;
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
        if (g->GetSliceNode())
          {
          g->GetSliceNode()->SetSliceVisible(0);
          }
        continue;
        }
      else
        {
        g->GridGUI( this->GetGridFrame2( ), 0, ncount );
        g->GetSliceViewer()->SetWidth(geom->GetDefaultSliceGUIFrameWidth());
        this->Script ("grid columnconfigure %s %d -weight 1", this->GridFrame2->GetWidgetName(), ncount );

        vtkMRMLSliceCompositeNode *compNode = g->GetLogic()->GetSliceCompositeNode();
        if (compNode && compNode->GetBackgroundVolumeID() == 0)
          {
          // no volume assigned. use the background of Red viewer for
          // first compare viewer, the foreground of Red Viewer for
          // bsecond compare viewer, and rest like the first
          vtkSlicerSliceGUI *red = this->SlicesGUI->GetSliceGUI("Red");
          if ((ncount == 0 || ncount > 2) && red && red->GetLogic() && red->GetLogic()->GetSliceCompositeNode() && red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID())
            {
            g->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID( red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID());
            }
          else if (ncount == 1 && red && red->GetLogic() && red->GetLogic()->GetSliceCompositeNode() && red->GetLogic()->GetSliceCompositeNode()->GetForegroundVolumeID())
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

#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

    // finally modify the layout node
    layout->DisableModifiedEventOn();
    layout->SetBottomPanelVisibility( 1 );
    int cur = layout->GetViewArrangement();
    if ( cur != vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView )
      {
      layout->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView );
      }
    layout->DisableModifiedEventOff();
    }

}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackSideBySideLightboxView()
{
  // Unpack the 3D viewer widget
  // (we don't know if it is the active widget or not)
  if ( this->GridFrame1 )
    {
    this->GridFrame1->UnpackChildren();
    }

  if (this->SlicesGUI)
    {
      vtkSlicerSliceGUI *g = NULL;
      g = this->SlicesGUI->GetSliceGUI("Red");
      if (g)
        {
          g->UngridGUI();
        }

    int nSliceGUI = this->SlicesGUI->GetNumberOfSliceGUI();
    const char *layoutname = NULL;
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::PackCompareView()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
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
    if (g->GetSliceNode())
      {
      x = g->GetSliceNode()->GetFieldOfView()[0];
      y = g->GetSliceNode()->GetFieldOfView()[1];
      z = g->GetSliceNode()->GetFieldOfView()[2];
      g->GetSliceNode()->SetFieldOfView(x, y, z);
      g->GetSliceNode()->UpdateMatrices();
      }

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
      vtkSlicerSliceGUI *g0 = this->SlicesGUI->GetSliceGUI(buf);
      if (g0 != NULL)
        {
        //--- go thru node to set lightbox rows and columns
        if ( g0->GetLogic() != NULL )
          {
          if ( g0->GetLogic()->GetSliceNode() != NULL )
            {
            g0->GetLogic()->GetSliceNode()->SetLayoutGrid ( numRows, numColumns );
            }
          }
        }
      }

    // configure the new layout
    this->Script ( "pack %s -side top -fill both -expand 1 -padx 0 -pady 0 ", this->GridFrame2->GetWidgetName ( ) );
    this->Script ("grid columnconfigure %s 0 -weight 1", this->GridFrame2->GetWidgetName() );

    const char *layoutname = NULL;
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
        if (g->GetSliceNode())
          {
          g->GetSliceNode()->SetSliceVisible(0);
          }
        continue;
        }
      else
        {
        g->GridGUI( this->GetGridFrame2( ), ncount, 0 );
        g->GetSliceViewer()->SetWidth(geom->GetDefaultSliceGUIFrameWidth());
        this->Script ("grid rowconfigure %s %d -weight 1", this->GridFrame2->GetWidgetName(), ncount );

        vtkMRMLSliceCompositeNode *compNode = g->GetLogic()->GetSliceCompositeNode();
        if (compNode && compNode->GetBackgroundVolumeID() == 0)
          {
          // no volume assigned. use the background of Red viewer for
          // first compare viewer, the foreground of Red Viewer for
          // bsecond compare viewer, and rest like the first
          vtkSlicerSliceGUI *red = this->SlicesGUI->GetSliceGUI("Red");
          if ((ncount == 0 || ncount > 2) && red && red->GetLogic() && red->GetLogic()->GetSliceCompositeNode() && red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID())
            {
            g->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID( red->GetLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID());
            }
          else if (ncount == 1 && red && red->GetLogic() && red->GetLogic()->GetSliceCompositeNode() && red->GetLogic()->GetSliceCompositeNode()->GetForegroundVolumeID())
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

#ifndef SLICESCONTROL_DEBUG
    this->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
#endif

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
    // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnpackDual3DView()
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
    // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
    // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
    const char *layoutname = NULL;
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
      // Hide the secondary panel
  if ( this->MainSlicerWindow )
    {
    if ( this->MainSlicerWindow->GetSecondarySplitFrame() )
      {
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Visibility(0);
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
    vtkMRMLSliceLogic *sliceLogic = this->GetApplicationLogic()->GetSliceLogic(layoutName);
    if (sliceLogic == NULL)
      {
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
      events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
      events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
      events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

      sliceLogic = vtkMRMLSliceLogic::New ( );
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
      // add the SliceGUI to the Slices module
      this->SlicesGUI->AddSliceGUI(layoutName, g);
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
    const char *layoutname = NULL;
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
    const char *layoutname = NULL;
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
      const char *layoutname = NULL;
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

// to do: the three vtkMRMLSliceLogic pointers should be changed with a
// pointer to a vtkMRMLSliceLogic map.
// current implementation only takes 3 for red, yellow, and green
//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetAndObserveMainSliceLogic ( vtkMRMLSliceLogic *l0,
                                                            vtkMRMLSliceLogic *l1,
                                                            vtkMRMLSliceLogic *l2 )
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

  if ( this->SlicesGUI )
    {
    vtkSlicerSliceGUI *g = this->SlicesGUI->GetSliceGUI("Red");
    if ( g )
      {
      g->SetAndObserveModuleLogic(l0);
      g = this->SlicesGUI->GetSliceGUI("Yellow");
      g->SetAndObserveModuleLogic(l1);
      g = this->SlicesGUI->GetSliceGUI("Green");
      g->SetAndObserveModuleLogic(l2);
      }
    }
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
      //--- for now, just turn this guy off. Expose again when curl doesn't step on itself and crash.
      vtkDataIOManager *dm = scene->GetDataIOManager();
      if ( dm != NULL )
        {
        dm->SetEnableAsynchronousIO ( 0 );
        /*
        if ( dm->GetEnableAsynchronousIO() != app->GetEnableAsynchronousIO() )
          {
          dm->SetEnableAsynchronousIO (app->GetEnableAsynchronousIO() );
          }
        */
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
void vtkSlicerApplicationGUI::MainSplitFrameConfigureCallback(int vtkNotUsed(width),
                                                              int vtkNotUsed(height))
{
  //std::cout << "MainSplitFrameConfigureCallback width:" << width
  //          << ", height:" << height << std::endl;
  this->GUILayoutNode->SetMainPanelSize( this->MainSlicerWindow->GetMainSplitFrame()->GetFrame1Size() );
}

//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SecondarySplitFrameConfigureCallback(int vtkNotUsed(width),
                                                                   int vtkNotUsed(height))
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

//-------------------------------------------------------------------------------------------------
//
void vtkSlicerApplicationGUI::SetExternalProgress(char *message, float progress)
{
  static int progress_initialized = 0;

  /* If a) you can not use getbuildtest.tcl (say, it doesn't support your
           compiler, nmake, or GNU make on Win32),
        b) you can not use the Slicer3.exe launcher (say, launch.tcl tries to
           read a file that is created by getbuildtest.tcl, or it crashes
           with a child killed 5936 on your platform, or can't be debugged),
        c) therefore you are calling slicer-real.exe directly,
     Then set TCL_DIR to see progress messages.
     Update: added catch() to avoid a stream the errors in the log dialog.
  */

  if ( !progress_initialized )
    {
    // look for mac/linux style or windows style name for wish version 8.4 or 8.5
    this->Script("catch {set wish_candidates [list $::env(TCL_DIR)/bin/wish8.4 $::env(TCL_DIR)/bin/wish84.exe $::env(TCL_DIR)/bin/wish8.5 $::env(TCL_DIR)/bin/wish85.exe]}");
    this->Script("catch {foreach wc $wish_candidates {if { [file exists $wc] } {set extprog_wish $wc} } }");
    this->Script("catch {set extprog_script $::env(Slicer_HOME)/lib/Slicer3/SlicerBaseGUI/Tcl/ExternalProgress.tcl}");
    this->Script("catch {set extprog_fp [open \"| $extprog_wish\" \"w\"]}");
    this->Script("catch {puts $extprog_fp \"source $extprog_script\"; flush $extprog_fp}");
    progress_initialized = 1;
    }

  const char *gauge = this->GetMainSlicerWindow()->GetProgressGauge()->GetWidgetName();
  const char *geometry = this->Script("winfo geometry %s", gauge);
  int w, h, x, y;
  sscanf(geometry, "%dx%d", &w, &h);
  const char *rootx = this->Script("string trim [winfo rootx %s]", gauge);
  sscanf(rootx, "%d", &x);
  const char *rooty = this->Script("string trim [winfo rooty %s]", gauge);
  sscanf(rooty, "%d", &y);
  char newGeometry[BUFSIZ];
  sprintf(newGeometry, "%dx%d+%d+%d", w, h, x, y);
  char progressString[BUFSIZ];
  sprintf(progressString, "%3.f", progress * 100);

  this->Script("catch {puts $extprog_fp \"progress_Window %s {%s} %s\"; flush $extprog_fp }",
                    newGeometry, message, progressString);
}

//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::SetCurrentQtModule(const char* moduleTitle)
{
#ifdef Slicer_USE_QT
  qSlicerModuleManager* moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);

  QString moduleName = moduleManager->moduleName(QLatin1String(moduleTitle));
  // Check if the corresponding QT module is loaded
  qSlicerAbstractCoreModule* module = moduleManager->module(moduleName);
  bool moduleRep = module ? module->widgetRepresentation() != 0 : false;

  qDebug() << "QT Module [" << QLatin1String(moduleTitle) << "] loaded:"
           << moduleManager->isLoaded(moduleName);
  qDebug() << "QT Module [" << QLatin1String(moduleTitle) << "] representation:" << moduleRep;

  // Set module panel visibility accordingly
  qSlicerApplication::application()->setTopLevelWidgetVisible(
    this->GetSlicerApplication()->modulePanel(), moduleRep);

  // Show module and update geometry if required
  if (moduleRep)
    {
    this->GetSlicerApplication()->modulePanel()->setModule(moduleName);
    this->ReposModulePanel();
    }
#endif
}

//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ReposModulePanel()
{
#ifdef Slicer_USE_QT
  if (!this->GetMainSlicerWindow() ||
      !this->GetMainSlicerWindow()->GetMainNotebook())
    {
    return;
    }
  vtkKWWidget* widget = this->GetMainSlicerWindow()->GetMainNotebook();
  int pos[2];
  int size[2];
  vtkKWTkUtilities::GetWidgetCoordinates(widget, &pos[0], &pos[1]);
  vtkKWTkUtilities::GetWidgetSize(widget, &size[0], &size[1]);
  Q_ASSERT(this->GetSlicerApplication());
  Q_ASSERT(this->GetSlicerApplication()->modulePanel());
  Q_ASSERT(qSlicerApplication::application()->moduleManager());
  this->GetSlicerApplication()->modulePanel()->setGeometry(QRect(pos[0], pos[1], size[0], size[1]));
#endif
}

//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ConfigureCallback(char* widgetName)
{
  // Make sure configure event related to the main window are considered
  if ((this->GetMainSlicerWindow() == 0) ||
      (this->GetMainSlicerWindow()->GetMainNotebook() == 0) ||
      ((strcmp(widgetName,this->GetMainSlicerWindow()->GetWidgetName()) != 0) &&
       (strcmp(widgetName,this->GetMainSlicerWindow()->GetMainNotebook()->GetWidgetName()) != 0)))
    {
    return;
    }

#ifdef Slicer_USE_QT
  //this->SetCurrentQtModuleVisible(true);
  this->ReposModulePanel();
#endif
}

//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UnMapCallback(char * widgetName)
{
  // Make sure configure event related to the main window are considered
  if (strcmp(widgetName,this->MainSlicerWindow->GetWidgetName()))
    {
    return;
    }
#ifdef Slicer_USE_QT
  //this->SetCurrentQtModuleVisible(false);
  qSlicerApplication::application()->setTopLevelWidgetsVisible(false);
#endif
}

//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::MapCallback(char * widgetName)
{
  // Make sure configure event related to the main window are considered
  if (strcmp(widgetName,this->MainSlicerWindow->GetWidgetName()))
    {
    return;
    }
#ifdef Slicer_USE_QT
  //this->SetCurrentQtModuleVisible(true);
  qSlicerApplication::application()->setTopLevelWidgetsVisible(true);
  this->ReposModulePanel();
#endif
}

int vtkSlicerApplicationGUI::GetNumberOfVisibleViewNodes()
{
  int numberOfVisibleNodes=0;
  vtkMRMLViewNode *node = NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLViewNode");    
  for (int n = 0; n < nnodes; n++)
    {
    node = vtkMRMLViewNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLViewNode"));
    if (node && node->GetVisibility())
      {
      numberOfVisibleNodes++;
      }
    }
  return numberOfVisibleNodes;
}


//-------------------------------------------------------------------------------------------------
void vtkSlicerApplicationGUI::ModifyAllWidgetLock(int lockFlag)
{
  //---
  //--- This method is called with the appropriate lockFlag
  //--- by the ToolbarGUI, when the vtkMRMLInteractionNode
  //--- invokes an InteractionModeChangedEvent.
  //---


  ///
  /// FIDUCIALS
  ///
  // iterate through the fiducial list widgets
  int nb_fid_viewer_widgets = this->GetNumberOfFiducialListWidgets();
  for (int i = 0; i < nb_fid_viewer_widgets; ++i)
    {
    vtkSlicerFiducialListWidget *fidlist_widget = this->GetNthFiducialListWidget(i);
    if (fidlist_widget)
      {
      fidlist_widget->ModifyAllWidgetLock(lockFlag);
      }
    }


  ///
  /// MEASUREMENTS
  ///
  if ( this->GetApplication() == NULL )
    {
    vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: Got NULL Application. Not modifying lock on some widgets." );
    return;
    }
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
  if ( app == NULL )
    {
    vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: Got NULL Slicer Application. Not modifying lock on some widgets." );
    return;
    }
  vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Measurements");

  if (m == NULL)
    {
    vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: Got NULL Measurements Modules. Not modifying lock on measurements widgets." );
    return;
    }
  app->Script("$::slicer3::MeasurementsGUI ModifyAllLock %d", lockFlag);



  ///
  /// Developers: INCLUDE OTHER PICKABLE WIDGETS HERE
  ///


  //--- now reset the cursor to default if we are not picking.
  if ( lockFlag )
    {

    vtkSlicerViewerWidget *active_viewer = this->GetActiveViewerWidget();
    if ( active_viewer == NULL )
      {
      vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: no active viewer widget. Not able to reset cursor." );
      return;
      }
    if ( active_viewer->GetMainViewer() == NULL )
      {
      vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: no main viewer. Not able to reset cursor." );
      return;
      }
    vtkRenderWindow *ren_win = active_viewer->GetMainViewer()->GetRenderWindow();
    if ( ren_win == NULL )
      {
      vtkWarningMacro ( "ApplicationGUI::ModifyAllWidgetLock: no render window. Not able to reset cursor.");
      return;
      }
    //--- if lockFlag == 1, then processEvents should be set to 0.
    //--- Trying this as a mechanism to give back
    //--- the default cursor, in case it is the picking cursor.
    //--- trick will be to get it to stick -- and not be reset to the picking cursor
    //--- by subsequent events.
    ren_win->SetCurrentCursor ( VTK_CURSOR_DEFAULT );
    }

}
