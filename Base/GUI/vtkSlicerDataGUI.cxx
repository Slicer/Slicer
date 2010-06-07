#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerWindow.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

//--- for loading.
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerColorDisplayWidget.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkMRMLModelNode.h"

#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTextWithHyperlinksWithScrollbars.h"
#include "vtkKWTkUtilities.h"

#include <string>

//#define LOADPANEL_DEBUG

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( )
{
  this->MRMLTreeWidget = vtkSlicerMRMLTreeWidget::New();

  this->LoadSceneButton = NULL;
  this->SceneInformationButton = NULL;
  this->SceneInformationFrame = NULL;
  this->SceneInformationOpen = 0;
  this->LoadDirectory = NULL;
  
  this->ImportSceneButton = NULL;
  this->ImportSceneInformationButton = NULL;
  this->ImportSceneInformationFrame = NULL;
  this->ImportSceneInformationOpen = 0;

  this->LoadVolumeButton = NULL;
  this->VolumeInformationButton = NULL;
  this->VolumeInformationFrame = NULL;
  this->VolumeInformationOpen = 0;
  
  this->LoadDicomVolumeButton = NULL;
  this->DicomInformationButton = NULL;
  this->DicomInformationFrame = NULL;
  this->DicomInformationOpen = 0;

  this->ModelInformationButton = NULL;
  this->ModelInformationFrame = NULL;
  this->LoadModelButton = NULL;
  this->ModelInformationOpen = 0;

  this->OverlayInformationButton = NULL;
  this->OverlayInformationFrame = NULL;
  this->LoadOverlayButton = NULL;
  this->OverlayInformationOpen = 0;

  this->TransformInformationButton = NULL;
  this->TransformInformationFrame = NULL;
  this->LoadTransformButton = NULL;
  this->TransformInformationOpen = 0;

  this->FiducialsInformationButton = NULL;
  this->FiducialsInformationFrame = NULL;
  this->LoadFiducialsButton = NULL;
  this->FiducialsInformationOpen = 0;

  this->DirectoryInformationButton = NULL;
  this->DirectoryInformationFrame = NULL;
  this->LoadDirectoryButton = NULL;
  this->DirectoryInformationOpen = 0;

  this->DataInformationButton = NULL;
  this->DataInformationFrame = NULL;
  this->LoadDataButton = NULL;
  this->DataInformationOpen = 0;

  this->ColorLUTInformationButton = NULL;
  this->ColorLUTInformationFrame = NULL;
  this->LoadColorLUTButton = NULL;
  this->ColorLUTInformationOpen = 0;

  this->FiberBundleInformationButton = NULL;
  this->FiberBundleInformationFrame = NULL;
  this->LoadFiberBundleButton = NULL;
  this->FiberBundleInformationOpen = 0;

  this->AddModelDialogButton = NULL;
  this->AddModelDirectoryDialogButton = NULL;
  this->AddModelWindow = NULL;

  this->ModelSelector = NULL;
  this->AddOverlayDialogButton = NULL;
  this->AddOverlayWindow = NULL;
  this->SelectedModelNode = NULL;
  
  this->AddFiducialDialog = NULL;
    
  this->AddColorLUTDialog = NULL;

  this->AddFiberBundleDialog = NULL;
  
  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( )
{
  if (this->MRMLTreeWidget)
    {
    this->MRMLTreeWidget->RemoveWidgetObservers ( );
    this->MRMLTreeWidget->SetParent (NULL );
    this->MRMLTreeWidget->Delete ( );
    }
  this->SetLoadDirectory ( NULL );
  
  if ( this->SceneInformationButton )
    {
    this->SceneInformationButton->SetParent ( NULL );
    this->SceneInformationButton->Delete();
    this->SceneInformationButton = NULL;
    }
  if ( this->SceneInformationFrame )
    {
    this->SceneInformationFrame->SetParent ( NULL );
    this->SceneInformationFrame->Delete();
    this->SceneInformationFrame = NULL;
    this->SceneInformationOpen = 0;
    }
  if ( this->LoadSceneButton )
    {
    this->LoadSceneButton->SetParent ( NULL );
    this->LoadSceneButton->Delete();
    this->LoadSceneButton = NULL;
    }

  if ( this->ImportSceneButton )
    {
    this->ImportSceneButton->SetParent (NULL);
    this->ImportSceneButton->Delete();
    this->ImportSceneButton = NULL;
    }
  if ( this->ImportSceneInformationButton )
    {
    this->ImportSceneInformationButton->SetParent ( NULL );
    this->ImportSceneInformationButton->Delete();
    this->ImportSceneInformationButton = NULL;    
    }
  if ( this->ImportSceneInformationFrame )
    {
    this->ImportSceneInformationFrame->SetParent ( NULL );
    this->ImportSceneInformationFrame->Delete();
    this->ImportSceneInformationFrame = NULL;
    this->ImportSceneInformationOpen = 0;    
    }

  if ( this->VolumeInformationButton )
    {
    this->VolumeInformationButton->SetParent ( NULL );
    this->VolumeInformationButton->Delete();
    this->VolumeInformationButton = NULL;
    }
  if ( this->VolumeInformationFrame )
    {
    this->VolumeInformationFrame->SetParent ( NULL );
    this->VolumeInformationFrame->Delete();
    this->VolumeInformationFrame = NULL;    
    this->VolumeInformationOpen = 0;
    }
  if ( this->LoadVolumeButton )
    {
    this->LoadVolumeButton->SetParent ( NULL);
    this->LoadVolumeButton->Delete();
    this->LoadVolumeButton = NULL;
    }

  if ( this->LoadDicomVolumeButton )
    {
    this->LoadDicomVolumeButton->SetParent ( NULL );
    this->LoadDicomVolumeButton->Delete();
    this->LoadDicomVolumeButton = NULL;
    }
  if ( this->DicomInformationButton )
    {
    this->DicomInformationButton->SetParent ( NULL );
    this->DicomInformationButton->Delete();
    this->DicomInformationButton = NULL;    
    }
  if ( this->DicomInformationFrame )
    {
    this->DicomInformationFrame->SetParent ( NULL );
    this->DicomInformationFrame->Delete();
    this->DicomInformationFrame = NULL;
    this->DicomInformationOpen = 0;    
    }

  if ( this->ModelInformationButton )
    {
    this->ModelInformationButton->SetParent (NULL );
    this->ModelInformationButton->Delete();
    this->ModelInformationButton = NULL;
    }
  if ( this->ModelInformationFrame )
    {
    this->ModelInformationFrame->SetParent ( NULL );
    this->ModelInformationFrame->Delete();
    this->ModelInformationFrame = NULL;    
    this->ModelInformationOpen = 0;
    }
  if ( this->LoadModelButton )
    {
    this->LoadModelButton->SetParent ( NULL );
    this->LoadModelButton->Delete();
    this->LoadModelButton = NULL;    
    }

  if ( this->OverlayInformationButton )
    {
    this->OverlayInformationButton->SetParent (NULL );
    this->OverlayInformationButton->Delete();
    this->OverlayInformationButton = NULL;
    }
  if ( this->OverlayInformationFrame )
    {
    this->OverlayInformationFrame->SetParent ( NULL );
    this->OverlayInformationFrame->Delete();
    this->OverlayInformationFrame = NULL;    
    this->OverlayInformationOpen = 0;
    }
  if ( this->LoadOverlayButton )
    {
    this->LoadOverlayButton->SetParent ( NULL );
    this->LoadOverlayButton->Delete();
    this->LoadOverlayButton = NULL;    
    }
  this->SelectedModelNode = NULL;
  
  if ( this->TransformInformationButton )
    {
    this->TransformInformationButton->SetParent (NULL );
    this->TransformInformationButton->Delete();
    this->TransformInformationButton = NULL;
    }
  if ( this->TransformInformationFrame )
    {
    this->TransformInformationFrame->SetParent ( NULL );
    this->TransformInformationFrame->Delete();
    this->TransformInformationFrame = NULL;    
    this->TransformInformationOpen = 0;
    }
  if ( this->LoadTransformButton )
    {
    this->LoadTransformButton->SetParent ( NULL );
    this->LoadTransformButton->Delete();
    this->LoadTransformButton = NULL;    
    }

  if ( this->FiducialsInformationButton )
    {
    this->FiducialsInformationButton->SetParent ( NULL );
    this->FiducialsInformationButton->Delete();
    this->FiducialsInformationButton = NULL;    
    }
  if ( this->FiducialsInformationFrame )
    {
    this->FiducialsInformationFrame->SetParent ( NULL );
    this->FiducialsInformationFrame->Delete();
    this->FiducialsInformationFrame = NULL;
    this->FiducialsInformationOpen = 0;
    }
  if ( this->LoadFiducialsButton )
    {
    this->LoadFiducialsButton->SetParent ( NULL );
    this->LoadFiducialsButton->Delete();
    this->LoadFiducialsButton = NULL;    
    }

  if ( this->DirectoryInformationButton )
    {
    this->DirectoryInformationButton->SetParent ( NULL );
    this->DirectoryInformationButton->Delete();
    this->DirectoryInformationButton = NULL;
    }
  if ( this->DirectoryInformationFrame )
    {
    this->DirectoryInformationFrame->SetParent ( NULL );
    this->DirectoryInformationFrame->Delete();
    this->DirectoryInformationFrame = NULL;    
    this->DirectoryInformationOpen = 0;
    }
  if ( this->LoadDirectoryButton )
    {
    this->LoadDirectoryButton->SetParent ( NULL);
    this->LoadDirectoryButton->Delete();
    this->LoadDirectoryButton = NULL;
    }

  if ( this->DataInformationButton )
    {
    this->DataInformationButton->SetParent ( NULL );
    this->DataInformationButton->Delete();
    this->DataInformationButton = NULL;
    }
  if ( this->DataInformationFrame )
    {
    this->DataInformationFrame->SetParent ( NULL );
    this->DataInformationFrame->Delete();
    this->DataInformationFrame = NULL;
    this->DataInformationOpen = 0;
    }
  if ( this->LoadDataButton )
    {
    this->LoadDataButton->SetParent ( NULL );
    this->LoadDataButton->Delete();
    this->LoadDataButton = NULL;
    }

  if (this->ColorLUTInformationButton )
    {
    this->ColorLUTInformationButton->SetParent ( NULL );
    this->ColorLUTInformationButton->Delete();
    this->ColorLUTInformationButton = NULL;    
    }
  if ( this->ColorLUTInformationFrame )
    {
    this->ColorLUTInformationFrame->SetParent ( NULL );
    this->ColorLUTInformationFrame->Delete();
    this->ColorLUTInformationFrame = NULL;
    this->ColorLUTInformationOpen = 0;
    }
  if ( this->LoadColorLUTButton )
    {
    this->LoadColorLUTButton->SetParent ( NULL );
    this->LoadColorLUTButton->Delete();
    this->LoadColorLUTButton = NULL;    
    }

  if ( this->FiberBundleInformationButton )
    {
    this->FiberBundleInformationButton->SetParent ( NULL );
    this->FiberBundleInformationButton->Delete();
    this->FiberBundleInformationButton = NULL;    
    }
  if ( this->FiberBundleInformationFrame )
    {
    this->FiberBundleInformationFrame->SetParent ( NULL );
    this->FiberBundleInformationFrame->Delete();
    this->FiberBundleInformationFrame = NULL;
    this->FiberBundleInformationOpen = 0;
    }
  if ( this->LoadFiberBundleButton )
    {
    this->LoadFiberBundleButton->SetParent ( NULL );
    this->LoadFiberBundleButton->Delete();
    this->LoadFiberBundleButton = NULL;    
    }

  //--- widgets in temporary raised windows.
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( this->AddModelDialogButton)
    {
    this->AddModelDialogButton->SetParent ( NULL );
    this->AddModelDialogButton->Delete();
    this->AddModelDialogButton = NULL;    
    }
  if ( this->AddModelDirectoryDialogButton)
    {
    this->AddModelDirectoryDialogButton->SetParent ( NULL );
    this->AddModelDirectoryDialogButton->Delete();
    this->AddModelDirectoryDialogButton = NULL;    
    }
  if ( this->AddModelWindow )
    {
    if ( app )
      {
      app->Script ( "grab release %s", this->AddModelWindow->GetWidgetName() );
      }
    this->AddModelWindow->Withdraw();
    this->AddModelWindow->Delete();
    }

  if ( this->AddOverlayDialogButton )
    {
    this->AddOverlayDialogButton->SetParent ( NULL );
    this->AddOverlayDialogButton->Delete();
    this->AddOverlayDialogButton = NULL;    
    }
  if ( this->ModelSelector)
    {
    this->ModelSelector->SetParent ( NULL );
    this->ModelSelector->Delete();
    this->ModelSelector = NULL;    
    }
  if ( this->AddOverlayWindow )
    {
    if ( app )
      {
      app->Script ( "grab release %s", this->AddOverlayWindow->GetWidgetName() );
      }
    this->AddOverlayWindow->Withdraw();
    this->AddOverlayWindow->Delete();
    }

  if ( this->AddFiducialDialog)
    {
    this->AddFiducialDialog->SetParent ( NULL );
    this->AddFiducialDialog->Delete();
    this->AddFiducialDialog = NULL;    
    }
  if ( this->AddColorLUTDialog)
    {
    this->AddColorLUTDialog->SetParent ( NULL );
    this->AddColorLUTDialog->Delete();
    this->AddColorLUTDialog = NULL;    
    }
  if ( this->AddFiberBundleDialog )
    {
    this->AddFiberBundleDialog->SetParent ( NULL );
    this->AddFiberBundleDialog->Delete();
    this->AddFiberBundleDialog = NULL;    
    }
  
  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }

  this->Built = false;
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDataGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::PackInformationFrame ( vtkKWFrame *f , vtkKWPushButton *b)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "PackInformationFrame: got Null SlicerApplication" );
    return;
    }

  if ( f->GetParent() != NULL )
    {
    this->Script ( "%s configure -height 100", f->GetParent()->GetWidgetName() );
    }
  this->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", f->GetWidgetName() );
  b->SetImageToIcon ( app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCloseIcon() );
  this->Script ( "update idletasks");
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::UnpackInformationFrame ( vtkKWFrame *f, vtkKWPushButton *b )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "UnpackInformationFrame: got Null SlicerApplication" );
    return;
    }

  this->Script ( "pack forget %s", f->GetWidgetName() );
  if ( f->GetParent() != NULL )
    {
    this->Script ( "%s configure -height 2", f->GetParent()->GetWidgetName() );
    }
  b->SetImageToIcon ( app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->Script ( "update idletasks");
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( )
{
  this->MRMLTreeWidget->RemoveObservers (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SceneInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadSceneButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ImportSceneButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ImportSceneInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->VolumeInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadVolumeButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ModelInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadModelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->OverlayInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadOverlayButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->TransformInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadTransformButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->FiducialsInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadFiducialsButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddFiducialDialog->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DirectoryInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadDirectoryButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ColorLUTInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadColorLUTButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddColorLUTDialog->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  this->FiberBundleInformationButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadFiberBundleButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddFiberBundleDialog->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( )
{
  this->MRMLTreeWidget->AddObserver (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MRMLTreeWidget->AddMRMLObservers();

  this->SceneInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadSceneButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ImportSceneButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ImportSceneInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->VolumeInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadVolumeButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ModelInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadModelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->OverlayInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadOverlayButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->TransformInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadTransformButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->FiducialsInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadFiducialsButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddFiducialDialog->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DirectoryInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadDirectoryButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ColorLUTInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadColorLUTButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddColorLUTDialog->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  this->FiberBundleInformationButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadFiberBundleButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AddFiberBundleDialog->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got Null SlicerApplicationGUI" );
    return;    
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow();
  if ( !win )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL Slicer Window" );
    return;
    }


  //--- node?
  if (caller == this->MRMLTreeWidget)
    {
    vtkMRMLNode *node = (vtkMRMLNode *)callData;
    if (node)
      {
      this->InvokeEvent(vtkSlicerModuleGUI::ModuleSelectedEvent, node);
      }
    }

  //--- node selector widget?
  //--- this is associated with adding overlays; temporary solution for centralized load.
  vtkSlicerNodeSelectorWidget *w = vtkSlicerNodeSelectorWidget::SafeDownCast ( caller );
  if (w != NULL && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
    {
    if ( w == this->ModelSelector )
      {
      this->SelectedModelNode = vtkMRMLModelNode::SafeDownCast (this->ModelSelector->GetSelected());
      if ( this->SelectedModelNode == NULL )
        {
        vtkErrorMacro ( "ProcessGUIEvents: Selected a NULL modelnode to receive an overlay." );
        }
      }
    }
  
  //--- a pushbutton?
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  if ( b != NULL && event == vtkKWPushButton::InvokedEvent )
    {
    if ( b == this->SceneInformationButton )
      {
      if ( this->SceneInformationOpen )
        {
        this->UnpackInformationFrame ( this->SceneInformationFrame, this->SceneInformationButton );
        this->SceneInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->SceneInformationFrame, this->SceneInformationButton );
        this->SceneInformationOpen = 1;        
        }
      }
    else if ( b == this->ImportSceneInformationButton )
      {
      if ( this->ImportSceneInformationOpen )
        {
        this->UnpackInformationFrame ( this->ImportSceneInformationFrame, this->ImportSceneInformationButton );
        this->ImportSceneInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->ImportSceneInformationFrame, this->ImportSceneInformationButton );
        this->ImportSceneInformationOpen = 1;        
        }
      }

    else if ( b == this->LoadSceneButton )
      {
      win->SetStatusText ( "Closing existing scene and opening selected scene file..." );
      app->Script ( "update idletasks" );
      appGUI->ProcessLoadSceneCommand();
      win->SetStatusText ( "" );
      app->Script ( "update idletasks" );
      }
    else if ( b == this->ImportSceneButton )
      {
      win->SetStatusText ( "Opening and adding data from selected scene file..." );
      app->Script ( "update idletasks" );
      appGUI->ProcessImportSceneCommand();
      win->SetStatusText ( "");
      app->Script ( "update idletasks" );
      }

    else if ( b == this->VolumeInformationButton )
      {
      if ( this->VolumeInformationOpen )
        {
        this->UnpackInformationFrame ( this->VolumeInformationFrame, this->VolumeInformationButton );
        this->VolumeInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->VolumeInformationFrame, this->VolumeInformationButton );
        this->VolumeInformationOpen = 1;        
        }
      }
    else if ( b == this->DicomInformationButton )
      {
      if ( this->DicomInformationOpen )
        {
        this->UnpackInformationFrame ( this->DicomInformationFrame, this->DicomInformationButton );
        this->DicomInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->DicomInformationFrame, this->DicomInformationButton );
        this->DicomInformationOpen = 1;        
        }
      }

    else if ( b == this->LoadDicomVolumeButton )
      {
      win->SetStatusText ( "Opening and adding volume file..." );
      app->Script ( "update idletasks" );
      appGUI->ProcessAddVolumeCommand();
      win->SetStatusText ( "");
      app->Script ( "update idletasks" );
      }
    else if ( b == this->LoadVolumeButton )
      {
      win->SetStatusText ( "Opening and adding volume file..." );
      app->Script ( "update idletasks" );
      appGUI->ProcessAddVolumeCommand();
      win->SetStatusText ( "");
      app->Script ( "update idletasks" );
      }

    else if ( b == this->ModelInformationButton )
      {
      if ( this->ModelInformationOpen )
        {
        this->UnpackInformationFrame ( this->ModelInformationFrame, this->ModelInformationButton );
        this->ModelInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->ModelInformationFrame, this->ModelInformationButton );
        this->ModelInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadModelButton )
      {
      this->RaiseAddModelWindow();
      }

    else if ( b == this->OverlayInformationButton )
      {
      if ( this->OverlayInformationOpen )
        {
        this->UnpackInformationFrame ( this->OverlayInformationFrame, this->OverlayInformationButton );
        this->OverlayInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->OverlayInformationFrame, this->OverlayInformationButton );
        this->OverlayInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadOverlayButton )
      {
      this->RaiseAddScalarOverlayWindow();
      }

    else if ( b == this->TransformInformationButton )
      {
      if ( this->TransformInformationOpen )
        {
        this->UnpackInformationFrame ( this->TransformInformationFrame, this->TransformInformationButton );
        this->TransformInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->TransformInformationFrame, this->TransformInformationButton );
        this->TransformInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadTransformButton )
      {
      appGUI->ProcessAddTransformCommand();      
      }

    else if ( b == this->FiducialsInformationButton )
      {
      if ( this->FiducialsInformationOpen )
        {
        this->UnpackInformationFrame ( this->FiducialsInformationFrame, this->FiducialsInformationButton );
        this->FiducialsInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->FiducialsInformationFrame, this->FiducialsInformationButton );
        this->FiducialsInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadFiducialsButton )
      {
      this->AddFiducialDialog->Invoke();
      }
    else if ( b == this->DirectoryInformationButton )
      {
      if ( this->DirectoryInformationOpen )
        {
        this->UnpackInformationFrame ( this->DirectoryInformationFrame, this->DirectoryInformationButton );
        this->DirectoryInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->DirectoryInformationFrame, this->DirectoryInformationButton );
        this->DirectoryInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadDirectoryButton )
      {
      appGUI->ProcessAddDataCommand ( );
      }

    else if ( b == this->DataInformationButton )
      {
      if ( this->DataInformationOpen )
        {
        this->UnpackInformationFrame ( this->DataInformationFrame, this->DataInformationButton );
        this->DataInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->DataInformationFrame, this->DataInformationButton );
        this->DataInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadDataButton )
      {
      }
    else if ( b == this->ColorLUTInformationButton )
      {
      if ( this->ColorLUTInformationOpen )
        {
        this->UnpackInformationFrame ( this->ColorLUTInformationFrame, this->ColorLUTInformationButton );
        this->ColorLUTInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->ColorLUTInformationFrame, this->ColorLUTInformationButton );
        this->ColorLUTInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadColorLUTButton )
      {
      this->AddColorLUTDialog->Invoke();
      }
    else if ( b == this->FiberBundleInformationButton )
      {
      if ( this->FiberBundleInformationOpen )
        {
        this->UnpackInformationFrame ( this->FiberBundleInformationFrame, this->FiberBundleInformationButton );
        this->FiberBundleInformationOpen = 0;
        }
      else
        {
        this->PackInformationFrame ( this->FiberBundleInformationFrame, this->FiberBundleInformationButton );
        this->FiberBundleInformationOpen = 1;        
        }
      }
    else if ( b == this->LoadFiberBundleButton )
      {
      this->AddFiberBundleDialog->Invoke();
      }
    }

  //--- or file browser dialog?
  //--- these are all pop-ups associated with adding data types. temporary solution for centralized load.
  vtkKWLoadSaveDialog *d = vtkKWLoadSaveDialog::SafeDownCast ( caller );
  if ( d != NULL &&  event == vtkKWTopLevel::WithdrawEvent )
    {
    if ( this->AddModelDialogButton != NULL && d == this->AddModelDialogButton->GetLoadSaveDialog() )
      {

      // If a file has been selected for loading...
      const char *fileName = this->AddModelDialogButton->GetFileName();
      if ( fileName ) 
        {
        win->SetStatusText ( "Reading and loading model file..." );
        app->Script ( "update idletasks" );

        //--- Get Models Logic
        vtkSlicerModelsGUI* modelsGUI = vtkSlicerModelsGUI::SafeDownCast ( app->GetModuleGUIByName ("Models"));
        if (modelsGUI != NULL )
          {
          vtkSlicerModelsLogic* modelLogic = modelsGUI->GetLogic();
          if ( modelLogic != NULL )
            {
            vtkMRMLModelNode *modelNode = modelLogic->AddModel( fileName );
            if ( modelNode == NULL ) 
              {
              vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
              if (this->UIPanel &&
                  this->UIPanel->GetPageWidget ( "Data" ))
                {
                dialog->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
                }
              else
                {
                if (modelsGUI->GetUIPanel() &&
                    modelsGUI->GetUIPanel()->GetPageWidget("Models"))
                  {
                  dialog->SetParent(modelsGUI->GetUIPanel()->GetPageWidget("Models"));
                  }
                else
                  {
                  // nothing obvious for the parent, so just set the
                  // application to try and avoid a kw widgets crash
                  dialog->SetApplication(app);
                  }
                }
              dialog->SetStyleToMessage();
              std::string msg = std::string("Unable to read model file ") + std::string(fileName);
              dialog->SetText(msg.c_str());
              dialog->Create ( );
              dialog->Invoke();
              dialog->Delete();
              vtkErrorMacro("Unable to read model file " << fileName);
              }
            else
              {
              win->SetStatusText ( "" );
              app->Script ( "update idletasks" );
              this->AddModelDialogButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
              const vtksys_stl::string fname(fileName);
              vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
              // set it to be the active model
              // set the display model
              this->SelectedModelNode = modelNode;
              if ( this->ModelSelector )
                {
                this->ModelSelector->SetSelected(this->SelectedModelNode);
                }
              }
            }
          }
        if ( this->AddModelDialogButton->GetText() )
          {
          this->AddModelDialogButton->SetText("");
          }
        //--- Withdraw and destroy the AddModelWindow
        this->WithdrawAddModelWindow();
        }
      return;
      }

    else if ( this->AddModelDirectoryDialogButton != NULL && d == this->AddModelDirectoryDialogButton->GetLoadSaveDialog() )
      {
      // If a file has been selected for loading...
      const char *fileName = this->AddModelDirectoryDialogButton->GetFileName();
      if ( fileName ) 
        {
        //--- Get Models Logic
        vtkSlicerModelsGUI* modelsGUI = vtkSlicerModelsGUI::SafeDownCast ( app->GetModuleGUIByName ("Models"));
        if (modelsGUI != NULL )
          {
          vtkSlicerModelsLogic* modelLogic = modelsGUI->GetLogic();
          if ( modelLogic != NULL )
            {
            vtkKWMessageDialog *dialog0 = vtkKWMessageDialog::New();
            vtkKWWidget *parentWidget = NULL;
            if (this->UIPanel &&
                this->UIPanel->GetPageWidget ( "Data" ))
              {
              parentWidget = this->UIPanel->GetPageWidget ( "Data" );
              }
            else
              {
              if (modelsGUI->GetUIPanel() &&
                  modelsGUI->GetUIPanel()->GetPageWidget("Models"))
                {
                parentWidget = modelsGUI->GetUIPanel()->GetPageWidget("Models");
                }
              }
            if (parentWidget != NULL)
              {
              dialog0->SetParent ( parentWidget );
              }
            else
              {
              // nothing obvious for the parent, so just set the
              // application
              dialog0->SetApplication(app);
              }
            dialog0->SetStyleToMessage();
            std::string msg0 = std::string("Reading *.vtk from models directory ") + std::string(fileName);
            dialog0->SetText(msg0.c_str());
            dialog0->Create ( );
            dialog0->Invoke();
            dialog0->Delete();

            win->SetStatusText ( "Reading and loading files..." );
            app->Script ( "update idletasks" );
            if (modelLogic->AddModels( fileName, ".vtk") == 0)
              {
              vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
              if (parentWidget != NULL)
                {
                dialog->SetParent ( parentWidget );
                }
              else
                {
                // nothing obvious for the parent, so just set the
                // application
                dialog->SetApplication(app);
                }
              dialog->SetStyleToMessage();
              std::string msg = std::string("Unable to read all models from directory ") + std::string(fileName);
              dialog->SetText(msg.c_str());
              dialog->Create ( );
              dialog->Invoke();
              dialog->Delete();
              vtkErrorMacro("ProcessGUIEvents: unable to read all models from directory " << fileName);
              }
            else
              {
              this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
              vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
              if (parentWidget != NULL)
                {
                dialog->SetParent ( parentWidget );
                }
              else
                {
                // nothing obvious for the parent, so just set the
                // application
                dialog->SetApplication(app);
                }
              dialog->SetStyleToMessage();
              dialog->SetText("Done reading models...");
              dialog->Create ( );
              dialog->Invoke();
              dialog->Delete();
              }
            }
          }
        //--- Withdraw and destroy the AddModelWindow
        if ( this->AddModelDirectoryDialogButton->GetText() )
          {
          this->AddModelDirectoryDialogButton->SetText("");
          }
        this->WithdrawAddModelWindow();
        }

      win->SetStatusText ( "" );
      app->Script ( "update idletasks" );
      return;      
      }    
    else if ( this->AddOverlayDialogButton != NULL && d == this->AddOverlayDialogButton->GetLoadSaveDialog() )
      {
      // If a scalar file has been selected for loading...
      const char *fileName = this->AddOverlayDialogButton->GetFileName();
      if ( fileName ) 
        {
        vtkSlicerModelsGUI* modelsGUI = vtkSlicerModelsGUI::SafeDownCast ( app->GetModuleGUIByName ("Models"));
        if ( this->SelectedModelNode == NULL )
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          if (this->UIPanel &&
              this->UIPanel->GetPageWidget ( "Data" ))
            {
            dialog->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
            }
          else
            {
            if (modelsGUI &&
                modelsGUI->GetUIPanel() &&
                modelsGUI->GetUIPanel()->GetPageWidget("Models"))
              {
              dialog->SetParent(modelsGUI->GetUIPanel()->GetPageWidget("Models"));
              }
            else
              {
              // nothing obvious for the parent, so just set the
              // application to try and avoid a kw widgets crash
              dialog->SetApplication(app);
              }
            }
          dialog->SetStyleToMessage();
          std::string msg = std::string("Please select a model to which the scalar overlay will be applied before selecting the overlay.");
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          return;
          }
        if (modelsGUI != NULL )
          {
          vtkSlicerModelsLogic* modelLogic = modelsGUI->GetLogic();
          if ( modelLogic != NULL )
            {
            vtkMRMLModelNode *modelNode = this->SelectedModelNode;
            if (modelNode != NULL)
              {
              vtkDebugMacro("vtkSlicerDataGUI: loading scalar for model " << modelNode->GetName());
              // load the scalars

              win->SetStatusText ( "Reading and loading scalar overlay..." );
              app->Script ( "update idletasks" );
      
              if (!modelLogic->AddScalar(fileName, modelNode))
                {
                vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
                if (this->UIPanel &&
                  this->UIPanel->GetPageWidget ( "Data" ))
                  {
                  dialog->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
                  }
                else
                  {
                  if (modelsGUI->GetUIPanel() &&
                      modelsGUI->GetUIPanel()->GetPageWidget("Models"))
                    {
                    dialog->SetParent(modelsGUI->GetUIPanel()->GetPageWidget("Models"));
                    }
                  else
                    {
                    // nothing obvious for the parent, so just set the
                    // application to try and avoid a kw widgets crash
                    dialog->SetApplication(app);
                    }
                  }                
                dialog->SetStyleToMessage();
                std::string msg = std::string("Unable to read scalars file ") + std::string(fileName);
                dialog->SetText(msg.c_str());
                dialog->Create ( );
                dialog->Invoke();
                dialog->Delete();
                vtkErrorMacro("Error loading scalar overlay file " << fileName);
                }
              else
                {
                this->AddOverlayDialogButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
                }
              }
            }
          }
        //--- Withdraw and destroy the AddOverlayWindow
        if ( this->AddOverlayDialogButton->GetText())
          {
          this->AddOverlayDialogButton->SetText ("");
          }
        this->WithdrawAddScalarOverlayWindow();
        }
      win->SetStatusText ( "" );
      app->Script ( "update idletasks" );
        return;      
      }
    else if ( this->AddFiducialDialog != NULL && d == this->AddFiducialDialog )
      {
      //---- THIS CHUNK COMES FROM Base/GUI/Tcl/Loader.tcl
      const char *fileName = this->AddFiducialDialog->GetFileName();
      if ( fileName )
        {
        vtkSlicerFiducialsGUI* fidGUI = vtkSlicerFiducialsGUI::SafeDownCast ( app->GetModuleGUIByName ("Fiducials"));
        if (fidGUI != NULL )
          {
          vtkSlicerFiducialsLogic* fidLogic = fidGUI->GetLogic();
          if ( fidLogic != NULL )
            {
            win->SetStatusText ( "Reading and loading fiducial list file..." );
            app->Script ( "update idletasks" );
            vtkMRMLFiducialListNode *fnode = fidLogic->LoadFiducialList ( fileName );
            if ( fnode == NULL )
              {
                vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
                dialog->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
                dialog->SetStyleToMessage();
                std::string msg = std::string("Unable to read fiducial list file ") + std::string(fileName);
                dialog->SetText(msg.c_str());
                dialog->Create ( );
                dialog->Invoke();
                dialog->Delete();
                vtkErrorMacro("ProcessGUIEvents: unable to read file " << fileName);
              }
            }
          }
        this->AddFiducialDialog->SaveLastPathToRegistry("OpenPath");
        }
      win->SetStatusText ( "");
      app->Script ( "update idletasks" );
      return;
      }
    else if ( this->AddColorLUTDialog != NULL && d == this->AddColorLUTDialog )
      {
      //---- THIS CHUNK COMES FROM vtkSlicerColorGUI
      const char *fileName = this->AddColorLUTDialog->GetFileName();
      if ( fileName )
        {
        vtkSlicerColorGUI* colorGUI = vtkSlicerColorGUI::SafeDownCast ( app->GetModuleGUIByName ("Color"));
        if (colorGUI != NULL )
          {
          vtkSlicerColorLogic* colorLogic = colorGUI->GetLogic();
          if ( colorLogic != NULL )
            {
            win->SetStatusText ( "Reading and loading color LUT file..." );
            app->Script ( "update idletasks" );
            int retval = colorLogic->LoadColorFile(fileName);
            if (!retval)
              {
                vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
                dialog->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
                dialog->SetStyleToMessage();
                std::string msg = std::string("Unable to read color file ") + std::string(fileName);
                dialog->SetText(msg.c_str());
                dialog->Create ( );
                dialog->Invoke();
                dialog->Delete();
                vtkErrorMacro("ProcessGUIEvents: unable to read file " << fileName);
              }
            }
          }
        this->AddColorLUTDialog->SaveLastPathToRegistry("OpenPath");
        }
      win->SetStatusText ( "" );
      app->Script ( "update idletasks" );
      return;
      }
    else if ( this->AddFiberBundleDialog != NULL && d == this->AddFiberBundleDialog )
      {
      const char *fileName = this->AddFiberBundleDialog->GetFileName();
      if ( fileName )
        {
        win->SetStatusText ( "Reading and loading DTI fiber bundle file..." );
        app->Script ( "update idletasks" );
        this->Script ( "LoadFiberBundle %s", fileName );
        this->AddFiberBundleDialog->SaveLastPathToRegistry("OpenPath");
        }
      //---- THIS CHUNK COMES FROM vtkSlicerTractographyDisplayGUI
      //--- implemented in tcl.
      win->SetStatusText ( "" );
      app->Script ( "update idletasks" );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                            unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessMRMLEvents ( vtkObject * vtkNotUsed(caller),
                                           unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
{
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ReleaseModuleEventBindings ( )
{
  
}




//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Enter ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "Enter: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "Enter: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "Enter: got NULL MainSlicerWindow");
    return;
    }

  if ( this->Built == false )
    {
    win->SetStatusText ( "Building Interface for Data Module...." );
    app->Script ( "update idletasks" );
    this->BuildGUI();
    this->AddGUIObservers();
    this->Built = true;

    // temporary for the load interface.
    if ( this->ModelSelector != NULL )
      {
      this->ModelSelector->UnconditionalUpdateMenu();
      }
    // end temporary.
    }
  else
    {
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();

}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( )
{
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    this->ReleaseModuleEventBindings();
    }

  this->WithdrawAddModelWindow();
  this->WithdrawAddScalarOverlayWindow();

  this->DestroyAddModelWindow();
  this->DestroyAddScalarOverlayWindow();

}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::TearDownGUI ( )
{
  this->WithdrawAddModelWindow();
  this->WithdrawAddScalarOverlayWindow();

  this->DestroyAddModelWindow();
  this->DestroyAddScalarOverlayWindow();
  
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::BuildGUI ( )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildGUI: got NULL MainSlicerWindow");
    return;
    }
  win->SetStatusText ( "Building Interface for Data Module...." );

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Data", "Data", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Data Module displays and permits operations on the MRML tree, and creates and edits transformation hierarchies.\n The Load panels exposes options for loading data. Helpful comments can be opened by clicking on the \"information\" icons in each load panel. \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:Data-Documentation-3.6</a>\n";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC, and the Slicer Community. See <a>http://www.slicer.org</a> for details.\nThe Data module was contributed by Alex Yarmarkovich, Isomics Inc. with help from others at SPL, BWH (Ron Kikinis, Wendy Plesniak)";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Data" );
    this->BuildHelpAndAboutFrame ( page, help, about );

    this->NACLabel = vtkKWLabel::New();
    this->NACLabel->SetParent ( this->GetLogoFrame() );
    this->NACLabel->Create();
    this->NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    this->NAMICLabel = vtkKWLabel::New();
    this->NAMICLabel->SetParent ( this->GetLogoFrame() );
    this->NAMICLabel->Create();
    this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    this->NCIGTLabel = vtkKWLabel::New();
    this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
    this->NCIGTLabel->Create();
    this->NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    this->BIRNLabel = vtkKWLabel::New();
    this->BIRNLabel->SetParent ( this->GetLogoFrame() );
    this->BIRNLabel->Create();
    this->BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

    vtkKWLabel *CTSCLabel = vtkKWLabel::New();
    CTSCLabel->SetParent ( this->GetLogoFrame() );
    CTSCLabel->Create();
    CTSCLabel->SetImageToIcon (this->GetAcknowledgementIcons()->GetCTSCLogo() );

    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", this->NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky e", this->NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e",  this->BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky e",  this->NCIGTLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 2 -sticky w",  CTSCLabel->GetWidgetName());                  
    app->Script ( "grid columnconfigure %s 0 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", this->GetLogoFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 1", this->GetLogoFrame()->GetWidgetName() );

    CTSCLabel->Delete();
    
    // INSPECT FRAME
    vtkSlicerModuleCollapsibleFrame *displayModifyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayModifyFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    displayModifyFrame->Create ( );
    displayModifyFrame->ExpandFrame ( );
    displayModifyFrame->SetLabelText ("Display & modify scene");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayModifyFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());
    
    this->MRMLTreeWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->MRMLTreeWidget->SetParent ( displayModifyFrame->GetFrame() );
    this->MRMLTreeWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->MRMLTreeWidget->GetWidgetName(), displayModifyFrame->GetFrame()->GetWidgetName());

  // ---
  // load FRAME
  // ---
#ifndef LOADPANEL_DEBUG
  vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New();
  loadFrame->SetParent( page );
  loadFrame->Create();
  loadFrame->SetLabelText("Load & Add Scenes or Individual Datasets");
  loadFrame->ExpandFrame();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    loadFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());
  this->BuildScenePanel(loadFrame->GetFrame());
  // ---
  // general data FRAME
  // ---
  this->BuildGeneralPanel (loadFrame->GetFrame());

  // ---
  // volumes FRAME
  // ---
  this->BuildVolumePanel ( loadFrame->GetFrame() );

  // ---
  // models FRAME
  // ---
  this->BuildModelPanel ( loadFrame->GetFrame() );
  
  // ---
  // overlays FRAME
  // ---
  this->BuildOverlayPanel ( loadFrame->GetFrame() );

  // ---
  // transforms FRAME
  // ---
  this->BuildTransformPanel ( loadFrame->GetFrame() );

  // ---
  // fiducials FRAME
  // ---
  this->BuildFiducialPanel ( loadFrame->GetFrame() );

  // ---
  // colorLUT FRAME
  // ---
  this->BuildColorPanel ( loadFrame->GetFrame() );

  // ---
  // fibers FRAME
  // ---
  this->BuildFiberBundlePanel ( loadFrame->GetFrame() );

  // ---
  // clean up.
  // ---
  loadFrame->Delete();
#endif
  
  displayModifyFrame->Delete ( );
  win->SetStatusText ( "" );
  this->Built = true;
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildScenePanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildScenePanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildScenePanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f1 = vtkKWFrame::New();
  f1->SetParent ( parent );
  f1->Create();
  vtkKWFrame *f2 = vtkKWFrame::New();
  f2->SetParent ( parent );
  f2->Create();
  vtkKWFrame *f3 = vtkKWFrame::New();
  f3->SetParent ( parent );
  f3->Create();
  vtkKWFrame *f4 = vtkKWFrame::New();
  f4->SetParent ( parent );
  f4->Create();
  
  this->LoadSceneButton = vtkKWPushButton::New();
  this->LoadSceneButton->SetParent ( f1 );
  this->LoadSceneButton->Create();
  this->LoadSceneButton->SetBorderWidth ( 0 );
  this->LoadSceneButton->SetReliefToFlat();  
  this->LoadSceneButton->SetCompoundModeToLeft ();
  this->LoadSceneButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadSceneIcon() );
  this->LoadSceneButton->SetText ( " Load new scene (close current)" );
  this->LoadSceneButton->SetWidth ( 300 );
  this->LoadSceneButton->SetAnchorToWest();

  this->SceneInformationButton = vtkKWPushButton::New();
  this->SceneInformationButton->SetParent ( f1 );
  this->SceneInformationButton->Create();
  this->SceneInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->SceneInformationButton->SetBorderWidth ( 0 );
  this->SceneInformationButton->SetReliefToFlat();
  this->SceneInformationButton->SetBalloonHelpString ("Open help for loading (replacing existing) scenes.");

  this->SceneInformationFrame = vtkKWFrame::New();
  this->SceneInformationFrame->SetParent ( f2 );
  this->SceneInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *scenetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  scenetxt->SetParent ( this->SceneInformationFrame );
  scenetxt->Create();
  scenetxt->SetHorizontalScrollbarVisibility ( 0 );
  scenetxt->SetVerticalScrollbarVisibility ( 1) ;
  scenetxt->GetWidget()->SetReliefToGroove();
  scenetxt->GetWidget()->SetWrapToWord();
  scenetxt->GetWidget()->QuickFormattingOn();
//  scenetxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this option to close the existing scene and load a new MRML scene from a Slicer3.x file with extension \".mrml\", or a Slicer2.x file with extension \".xml\". Data collections in XNAT Catalog format (\".xcat\") or XNAT Archive format (\".xar\") may also be loaded. (Click the **Close** (x) icon to hide this message.)\n";
  scenetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  scenetxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", scenetxt->GetWidgetName() );  


  this->ImportSceneButton = vtkKWPushButton::New();
  this->ImportSceneButton->SetParent ( f3 );
  this->ImportSceneButton->Create();
  this->ImportSceneButton->SetBorderWidth ( 0 );
  this->ImportSceneButton->SetReliefToFlat();  
  this->ImportSceneButton->SetCompoundModeToLeft ();
  this->ImportSceneButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerImportSceneIcon() );
  this->ImportSceneButton->SetText (" Add a scene (to current)" );
  this->ImportSceneButton->SetWidth ( 300 );
  this->ImportSceneButton->SetAnchorToWest();

  this->ImportSceneInformationButton = vtkKWPushButton::New();
  this->ImportSceneInformationButton->SetParent ( f3 );
  this->ImportSceneInformationButton->Create();
  this->ImportSceneInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->ImportSceneInformationButton->SetBorderWidth ( 0 );
  this->ImportSceneInformationButton->SetReliefToFlat();
  this->ImportSceneInformationButton->SetBalloonHelpString ("Open help for importing (adding) scenes.");

  this->ImportSceneInformationFrame = vtkKWFrame::New();
  this->ImportSceneInformationFrame->SetParent ( f4 );
  this->ImportSceneInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *importtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  importtxt->SetParent ( this->ImportSceneInformationFrame );
  importtxt->Create();
  importtxt->SetHorizontalScrollbarVisibility ( 0 );
  importtxt->SetVerticalScrollbarVisibility ( 1) ;
  importtxt->GetWidget()->SetReliefToGroove();
  importtxt->GetWidget()->SetWrapToWord();
  importtxt->GetWidget()->QuickFormattingOn();
//  importtxt->GetWidget()->SetHeight( 12 );
  const char *t2 = "Use this option to add a new scene to the existing scene. Supported file formats include Slicer3.x MRML files (\".mrml\"), Slicer2.x MRML files (\".xml\"), data collections in XNAT Catalog format (\".xcat\") or XNAT Archive format (\".xar\").. (Click the **Close** (x) icon to hide this message.)\n";
  importtxt->SetText ( t2 );
  importtxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", importtxt->GetWidgetName() );    

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand n", f1->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand n", f2->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand n", f3->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand n", f4->GetWidgetName() );

  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadSceneButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->SceneInformationButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->ImportSceneButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->ImportSceneInformationButton->GetWidgetName() );

  
  scenetxt->Delete();
  importtxt->Delete();
  f1->Delete();
  f2->Delete();
  f3->Delete();
  f4->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildGeneralPanel (vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildGeneralPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGeneralPanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->DirectoryInformationButton = vtkKWPushButton::New();
  this->DirectoryInformationButton->SetParent ( f );
  this->DirectoryInformationButton->Create();
  this->DirectoryInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->DirectoryInformationButton->SetBorderWidth ( 0 );
  this->DirectoryInformationButton->SetReliefToFlat();
  this->DirectoryInformationButton->SetBalloonHelpString ("Open help for adding data or directory.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->DirectoryInformationFrame = vtkKWFrame::New();
  this->DirectoryInformationFrame->SetParent ( infoFrame );
  this->DirectoryInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *directorytxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  directorytxt->SetParent ( this->DirectoryInformationFrame );
  directorytxt->Create();
  directorytxt->SetHorizontalScrollbarVisibility ( 0 );
  directorytxt->SetVerticalScrollbarVisibility ( 1) ;
  directorytxt->GetWidget()->SetReliefToGroove();
  directorytxt->GetWidget()->SetWrapToWord();
  directorytxt->GetWidget()->QuickFormattingOn();
//  directorytxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this general load option to add all files in a selected directory to the existing scene, or an individual data set of any supported type. (Click the **Close** (x) icon to hide this message.)\n";  
  directorytxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  directorytxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", directorytxt->GetWidgetName() );  

  this->LoadDirectoryButton = vtkKWPushButton::New();
  this->LoadDirectoryButton->SetParent ( f );
  this->LoadDirectoryButton->Create();
  this->LoadDirectoryButton->SetBorderWidth ( 0 );
  this->LoadDirectoryButton->SetReliefToFlat();  
  this->LoadDirectoryButton->SetCompoundModeToLeft ();
  this->LoadDirectoryButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadDirectoryIcon() );
  this->LoadDirectoryButton->SetText (" Add data or a data directory" );
  this->LoadDirectoryButton->SetWidth ( 300 );
  this->LoadDirectoryButton->SetAnchorToWest();

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadDirectoryButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->DirectoryInformationButton->GetWidgetName() );


  directorytxt->Delete();
  f->Delete();
  infoFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildVolumePanel ( vtkKWFrame *parent  )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildVolumePanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildVolumePanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f1 = vtkKWFrame::New();
  f1->SetParent ( parent );
  f1->Create();
  vtkKWFrame *f2 = vtkKWFrame::New();
  f2->SetParent ( parent );
  f2->Create();

  this->VolumeInformationButton = vtkKWPushButton::New();
  this->VolumeInformationButton->SetParent ( f1);
  this->VolumeInformationButton->Create();
  this->VolumeInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->VolumeInformationButton->SetBorderWidth ( 0 );
  this->VolumeInformationButton->SetReliefToFlat();
  this->VolumeInformationButton->SetBalloonHelpString ("Open help for adding volumes.");

  this->VolumeInformationFrame = vtkKWFrame::New();
  this->VolumeInformationFrame->SetParent ( f2 );
  this->VolumeInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *volumetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  volumetxt->SetParent ( this->VolumeInformationFrame );
  volumetxt->Create();
  volumetxt->SetHorizontalScrollbarVisibility ( 0 );
  volumetxt->SetVerticalScrollbarVisibility ( 1) ;
  volumetxt->GetWidget()->SetReliefToGroove();
  volumetxt->GetWidget()->SetWrapToWord();
  volumetxt->GetWidget()->QuickFormattingOn();
//  volumetxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this volume loading option to browse DICOM information, and to add volume image data in a variety of formats to the existing scene. (Click the **Close** (x) icon to hide this message.)\n";  
  volumetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  volumetxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", volumetxt->GetWidgetName() );    
  
  this->LoadVolumeButton = vtkKWPushButton::New();
  this->LoadVolumeButton->SetParent ( f1 );
  this->LoadVolumeButton->Create();
  this->LoadVolumeButton->SetBorderWidth ( 0 );
  this->LoadVolumeButton->SetReliefToFlat();  
  this->LoadVolumeButton->SetCompoundModeToLeft ();
  this->LoadVolumeButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadVolumeIcon() );
  this->LoadVolumeButton->SetText (" Add scalar and tensor volumes" );
  this->LoadVolumeButton->SetWidth ( 300 );
  this->LoadVolumeButton->SetAnchorToWest();

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f1->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f2->GetWidgetName() );  
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadVolumeButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->VolumeInformationButton->GetWidgetName() );

  volumetxt->Delete();
  f1->Delete();
  f2->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildModelPanel (vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildModelPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildModelPanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->ModelInformationButton = vtkKWPushButton::New();
  this->ModelInformationButton->SetParent ( f);
  this->ModelInformationButton->Create();
  this->ModelInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->ModelInformationButton->SetBorderWidth ( 0 );
  this->ModelInformationButton->SetReliefToFlat();
  this->ModelInformationButton->SetBalloonHelpString ("Open help for adding models.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->ModelInformationFrame = vtkKWFrame::New();
  this->ModelInformationFrame->SetParent ( infoFrame );
  this->ModelInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *modeltxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  modeltxt->SetParent ( this->ModelInformationFrame);
  modeltxt->Create();
  modeltxt->SetHorizontalScrollbarVisibility ( 0 );
  modeltxt->SetVerticalScrollbarVisibility ( 1) ;
  modeltxt->GetWidget()->SetReliefToGroove();
  modeltxt->GetWidget()->SetWrapToWord();
  modeltxt->GetWidget()->QuickFormattingOn();
//  modeltxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this model loading option to raise a dialog with options to add models to the current scene. (Click the **Close** (x) icon to hide this message.)\n";  
  modeltxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  modeltxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", modeltxt->GetWidgetName() );    

  this->LoadModelButton = vtkKWPushButton::New();
  this->LoadModelButton->SetParent ( f );
  this->LoadModelButton->Create();
  this->LoadModelButton->SetBorderWidth ( 0 );
  this->LoadModelButton->SetReliefToFlat();  
  this->LoadModelButton->SetCompoundModeToLeft ();
  this->LoadModelButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadModelIcon() );
  this->LoadModelButton->SetText (" Add 3D model or a model directory" );
  this->LoadModelButton->SetWidth ( 300 );
  this->LoadModelButton->SetAnchorToWest();
  
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadModelButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->ModelInformationButton->GetWidgetName() );

  modeltxt->Delete();
  f->Delete();
  infoFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildOverlayPanel (vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildOverlayPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildOverlayPanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->OverlayInformationButton = vtkKWPushButton::New();
  this->OverlayInformationButton->SetParent ( f);
  this->OverlayInformationButton->Create();
  this->OverlayInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->OverlayInformationButton->SetBorderWidth ( 0 );
  this->OverlayInformationButton->SetReliefToFlat();
  this->OverlayInformationButton->SetBalloonHelpString ("Open help for adding scalar overlays for models.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->OverlayInformationFrame = vtkKWFrame::New();
  this->OverlayInformationFrame->SetParent ( infoFrame );
  this->OverlayInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *modeltxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  modeltxt->SetParent ( this->OverlayInformationFrame);
  modeltxt->Create();
  modeltxt->SetHorizontalScrollbarVisibility ( 0 );
  modeltxt->SetVerticalScrollbarVisibility ( 1) ;
  modeltxt->GetWidget()->SetReliefToGroove();
  modeltxt->GetWidget()->SetWrapToWord();
  modeltxt->GetWidget()->QuickFormattingOn();
//  modeltxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this option to add a (FreeSurfer) scalar overlay to an existing model in the scene. (Click the **Close** (x) icon to hide this message.)\n";  
  modeltxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  modeltxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", modeltxt->GetWidgetName() );    

  this->LoadOverlayButton = vtkKWPushButton::New();
  this->LoadOverlayButton->SetParent ( f );
  this->LoadOverlayButton->Create();
  this->LoadOverlayButton->SetBorderWidth ( 0 );
  this->LoadOverlayButton->SetReliefToFlat();  
  this->LoadOverlayButton->SetCompoundModeToLeft ();
  this->LoadOverlayButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadScalarOverlayIcon() );
  this->LoadOverlayButton->SetText (" Add scalar overlay" );
  this->LoadOverlayButton->SetWidth ( 300 );
  this->LoadOverlayButton->SetAnchorToWest();
  
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadOverlayButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->OverlayInformationButton->GetWidgetName() );

  modeltxt->Delete();
  f->Delete();
  infoFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildTransformPanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildTransformPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildTransformPanel: got Null SlicerApplicationGUI" );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->TransformInformationButton = vtkKWPushButton::New();
  this->TransformInformationButton->SetParent ( f );
  this->TransformInformationButton->Create();
  this->TransformInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->TransformInformationButton->SetBorderWidth ( 0 );
  this->TransformInformationButton->SetReliefToFlat();
  this->TransformInformationButton->SetBalloonHelpString ("Open help for adding transforms.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->TransformInformationFrame = vtkKWFrame::New();
  this->TransformInformationFrame->SetParent ( infoFrame );
  this->TransformInformationFrame->Create();
  vtkKWTextWithHyperlinksWithScrollbars *transformtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );

  transformtxt->SetParent ( this->TransformInformationFrame );
  transformtxt->Create();
  transformtxt->SetHorizontalScrollbarVisibility ( 0 );
  transformtxt->SetVerticalScrollbarVisibility ( 1) ;
  transformtxt->GetWidget()->SetReliefToGroove();
  transformtxt->GetWidget()->SetWrapToWord();
  transformtxt->GetWidget()->QuickFormattingOn();
//  transformtxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this transform loading option to add a new transform initialized with an Identity matrix to the current scene. Then, use the MRML tree widget in the panel above to position the transform in the data hierarchy.. (Click the **Close** (x) icon to hide this message.)\n";  
  transformtxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  transformtxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", transformtxt->GetWidgetName() );  

  this->LoadTransformButton = vtkKWPushButton::New();
  this->LoadTransformButton->SetParent ( f );
  this->LoadTransformButton->Create();
  this->LoadTransformButton->SetBorderWidth ( 0 );
  this->LoadTransformButton->SetReliefToFlat();  
  this->LoadTransformButton->SetCompoundModeToLeft ();
  this->LoadTransformButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadTransformIcon() );
  this->LoadTransformButton->SetText (" Add transformation matrix" );
  this->LoadTransformButton->SetWidth ( 300 );
  this->LoadTransformButton->SetAnchorToWest();

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadTransformButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->TransformInformationButton->GetWidgetName() );

  transformtxt->Delete();
  f->Delete();
  infoFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI:: BuildFiducialPanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildFiducialPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildFiducialPanel: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildFiducialPanel: got NULL MainSlicerWindow");
    return;
    }


  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->FiducialsInformationButton = vtkKWPushButton::New();
  this->FiducialsInformationButton->SetParent ( f);
  this->FiducialsInformationButton->Create();
  this->FiducialsInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->FiducialsInformationButton->SetBorderWidth ( 0 );
  this->FiducialsInformationButton->SetReliefToFlat();
  this->FiducialsInformationButton->SetBalloonHelpString ("Open help for adding fiducial lists.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->FiducialsInformationFrame = vtkKWFrame::New();
  this->FiducialsInformationFrame->SetParent ( infoFrame );
  this->FiducialsInformationFrame->Create();
  vtkKWTextWithHyperlinksWithScrollbars *fiducialtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );

  fiducialtxt->SetParent ( this->FiducialsInformationFrame );
  fiducialtxt->Create();
  fiducialtxt->SetHorizontalScrollbarVisibility ( 0 );
  fiducialtxt->SetVerticalScrollbarVisibility ( 1) ;
  fiducialtxt->GetWidget()->SetReliefToGroove();
  fiducialtxt->GetWidget()->SetWrapToWord();
  fiducialtxt->GetWidget()->QuickFormattingOn();
//  fiducialtxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this option to add a fiducial list from a file with extension (\".fcsv\") to the current scene.  (Click the **Close** (x) icon to hide this message.)\n";  
  fiducialtxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  fiducialtxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", fiducialtxt->GetWidgetName() );  

  this->LoadFiducialsButton = vtkKWPushButton::New();
  this->LoadFiducialsButton->SetParent ( f );
  this->LoadFiducialsButton->Create();
  this->LoadFiducialsButton->SetBorderWidth ( 0 );
  this->LoadFiducialsButton->SetReliefToFlat();  
  this->LoadFiducialsButton->SetCompoundModeToLeft ();
  this->LoadFiducialsButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadFiducialsIcon() );
  this->LoadFiducialsButton->SetText (" Add fiducial list" );
  this->LoadFiducialsButton->SetWidth ( 300 );
  this->LoadFiducialsButton->SetAnchorToWest();

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadFiducialsButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->FiducialsInformationButton->GetWidgetName() );

  //--- set up dialog to be invoked if pushbutton is clicked.

  this->AddFiducialDialog = vtkKWLoadSaveDialog::New();
  this->AddFiducialDialog->SetParent ( win );
  this->AddFiducialDialog->Create();
  this->AddFiducialDialog->SetTitle ( "Add Fiducial List" );
  this->AddFiducialDialog->SetFileTypes ( "{ {All} {.*} } { {Fiducial Lists} {.fcsv} }" );
  if ( this->GetLoadDirectory() == NULL )
    {
    this->AddFiducialDialog->RetrieveLastPathFromRegistry ("OpenPath");
    }
  else
    {
    this->AddFiducialDialog->SetLastPath ( this->GetLoadDirectory() );
    }

  fiducialtxt->Delete();
  f->Delete();
  infoFrame->Delete();
}


//---------------------------------------------------------------------------
  void vtkSlicerDataGUI::BuildColorPanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildColorPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildColorPanel: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildColorPanel: got NULL MainSlicerWindow");
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->ColorLUTInformationButton = vtkKWPushButton::New();
  this->ColorLUTInformationButton->SetParent ( f);
  this->ColorLUTInformationButton->Create();
  this->ColorLUTInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->ColorLUTInformationButton->SetBorderWidth ( 0 );
  this->ColorLUTInformationButton->SetReliefToFlat();
  this->ColorLUTInformationButton->SetBalloonHelpString ("Open help for adding color tables.");

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->ColorLUTInformationFrame = vtkKWFrame::New();
  this->ColorLUTInformationFrame->SetParent ( infoFrame );
  this->ColorLUTInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *colortxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  colortxt->SetParent ( this->ColorLUTInformationFrame );
  colortxt->Create();
  colortxt->SetHorizontalScrollbarVisibility ( 0 );
  colortxt->SetVerticalScrollbarVisibility ( 1) ;
  colortxt->GetWidget()->SetReliefToGroove();
  colortxt->GetWidget()->SetWrapToWord();
  colortxt->GetWidget()->QuickFormattingOn();
//  colortxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this option to add new **Color look-up tables** to the scene (files with extension \".txt\"). Click the **Add color table** button, and select a color LUT file (with extension .txt)  from the File Browser. (Click the **Close** (x) icon to hide this message.)\n";
  colortxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  colortxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", colortxt->GetWidgetName() );  

  this->LoadColorLUTButton = vtkKWPushButton::New();
  this->LoadColorLUTButton->SetParent ( f );
  this->LoadColorLUTButton->Create();
  this->LoadColorLUTButton->SetBorderWidth ( 0 );
  this->LoadColorLUTButton->SetReliefToFlat();  
  this->LoadColorLUTButton->SetCompoundModeToLeft ();
  this->LoadColorLUTButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadColorLUTIcon() );
  this->LoadColorLUTButton->SetText (" Add color table" );
  this->LoadColorLUTButton->SetWidth ( 300 );
  this->LoadColorLUTButton->SetAnchorToWest();

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadColorLUTButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->ColorLUTInformationButton->GetWidgetName() );


  //--- set up dialog to be invoked if pushbutton is clicked.
  this->AddColorLUTDialog = vtkKWLoadSaveDialog::New();
  this->AddColorLUTDialog->SetParent ( win );
  this->AddColorLUTDialog->Create();
  this->AddColorLUTDialog->SetTitle ( "Add Color Look-up Table" );
  this->AddColorLUTDialog->SetFileTypes ( "{ {All} {.*} } { {ColorLUT} {.txt} }" );
  if ( this->GetLoadDirectory() == NULL )
    {
    this->AddColorLUTDialog->RetrieveLastPathFromRegistry ("OpenPath");
    }
  else
    {
    this->AddColorLUTDialog->SetLastPath ( this->GetLoadDirectory() );
    }

  colortxt->Delete();
  f->Delete();
  infoFrame->Delete();
}


//---------------------------------------------------------------------------
  void vtkSlicerDataGUI::BuildFiberBundlePanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildFiberBundlelPanel: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildFiberBundlePanel: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildFiberBundlePanel: got NULL MainSlicerWindow");
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();

  this->FiberBundleInformationButton = vtkKWPushButton::New();
  this->FiberBundleInformationButton->SetParent ( f);
  this->FiberBundleInformationButton->Create();
  this->FiberBundleInformationButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerInformationIcon() );
  this->FiberBundleInformationButton->SetBorderWidth ( 0 );
  this->FiberBundleInformationButton->SetReliefToFlat();
  this->FiberBundleInformationButton->SetBalloonHelpString ("Open help for adding tractography fiber bundles.");

  this->LoadFiberBundleButton = vtkKWPushButton::New();
  this->LoadFiberBundleButton->SetParent ( f );
  this->LoadFiberBundleButton->Create();
  this->LoadFiberBundleButton->SetBorderWidth ( 0 );
  this->LoadFiberBundleButton->SetReliefToFlat();  
  this->LoadFiberBundleButton->SetCompoundModeToLeft ();
  this->LoadFiberBundleButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerLoadFiberBundleIcon() );
  this->LoadFiberBundleButton->SetText (" Add DTI fiber bundle" );
  this->LoadFiberBundleButton->SetWidth ( 300 );
  this->LoadFiberBundleButton->SetAnchorToWest();

  vtkKWFrame *infoFrame = vtkKWFrame::New();
  infoFrame->SetParent ( parent );
  infoFrame->Create();

  this->FiberBundleInformationFrame = vtkKWFrame::New();
  this->FiberBundleInformationFrame->SetParent ( infoFrame );
  this->FiberBundleInformationFrame->Create();

  vtkKWTextWithHyperlinksWithScrollbars *fibertxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  fibertxt->SetParent ( this->FiberBundleInformationFrame );
  fibertxt->Create();
  fibertxt->SetHorizontalScrollbarVisibility ( 0 );
  fibertxt->SetVerticalScrollbarVisibility ( 1) ;
  fibertxt->GetWidget()->SetReliefToGroove();
  fibertxt->GetWidget()->SetWrapToWord();
  fibertxt->GetWidget()->QuickFormattingOn();
//  fibertxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "Use this option to add new **Tractography Fiber Bundles** to the scene. Click the **Add fiber bundle** button, and select a fiber bundle file from the File Browser. (Click the **Close** (x) icon to hide this message.)\n";
  fibertxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  fibertxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -padx 2 -pady 2 -fill x -expand y", fibertxt->GetWidgetName() );

  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor w -padx 0 -pady 0 -fill x -expand y", infoFrame->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->LoadFiberBundleButton->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 2 -pady 4", this->FiberBundleInformationButton->GetWidgetName() );

  //--- set up dialog to be invoked if pushbutton is clicked.
  this->AddFiberBundleDialog = vtkKWLoadSaveDialog::New();
  this->AddFiberBundleDialog->SetParent ( win );
  this->AddFiberBundleDialog->Create();
  this->AddFiberBundleDialog->SetTitle ( "Add DTI Fiber Bundle Model" );
  this->AddFiberBundleDialog->SetFileTypes ( "{ {All} {.*} } { {Model} {.vtk} }" );  
  if ( this->GetLoadDirectory() == NULL )
    {
    this->AddFiberBundleDialog->RetrieveLastPathFromRegistry ("OpenPath");
    }
  else
    {
    this->AddFiberBundleDialog->SetLastPath ( this->GetLoadDirectory() );
    }

  fibertxt->Delete();
  f->Delete();
  infoFrame->Delete();
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::WithdrawAddModelWindow ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app && this->AddModelWindow )
    {
    app->Script ( "grab release %s", this->AddModelWindow->GetWidgetName() );
    }

  if ( this->AddModelDialogButton )
    {
    this->AddModelDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->AddModelDirectoryDialogButton )
    {
    this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->AddModelWindow )
    {
    this->AddModelWindow->Withdraw();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RaiseAddModelWindow ( )
{
  //--- create window if not already created.
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app == NULL )
    {
    vtkErrorMacro ( "RaiseAddModelWindow: got NULL SlicerApplication");
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI ( );
  if ( appGUI == NULL )
    {
    vtkErrorMacro ( "RaiseAddModelWindow: got NULL SlicerApplicationGUI");
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "RaiseAddModelWindow: got NULL MainSlicerWindow");
    return;
    }

  if ( this->LoadDirectory == NULL )
    {
    if ( app )
      {
      if ( app->GetTemporaryDirectory() )
        {
        this->SetLoadDirectory(app->GetTemporaryDirectory() );
        }
      }
    }
  if ( this->AddModelWindow == NULL )
    {
    //-- top level container.
    this->AddModelWindow = vtkKWTopLevel::New();
    this->AddModelWindow->SetMasterWindow (win );
    if ( this->LoadModelButton )
      {
      this->AddModelWindow->SetParent ( this->LoadModelButton);
      }
    
    this->AddModelWindow->SetApplication ( this->GetApplication() );
    this->AddModelWindow->Create();
    if ( this->GetLoadModelButton() )
      {
      int px, py;
      vtkKWTkUtilities::GetWidgetCoordinates( this->GetLoadModelButton(), &px, &py );
      this->AddModelWindow->SetPosition ( px + 10, py + 10 );
      }
    this->AddModelWindow->SetBorderWidth ( 1 );
    this->AddModelWindow->SetReliefToFlat();
    this->AddModelWindow->SetTitle ( "Add a 3D model");
    this->AddModelWindow->SetSize ( 250, 100 );
    this->AddModelWindow->Withdraw();
    this->AddModelWindow->SetDeleteWindowProtocolCommand ( this, "DestroyAddModelWindow");

    //--- Add model button
    vtkKWLabel *l0 = vtkKWLabel::New();
    l0->SetParent ( this->AddModelWindow );
    l0->Create();
    l0->SetText ( "Select model:" );

    this->AddModelDialogButton = vtkKWLoadSaveButton::New();
    this->AddModelDialogButton->SetParent ( this->AddModelWindow );
    this->AddModelDialogButton->Create();
    if ( this->GetLoadDirectory() == NULL )
      {
      this->AddModelDialogButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry ("OpenPath");
      const char *lastpath = this->AddModelDialogButton->GetLoadSaveDialog()->GetLastPath();
      if ( lastpath != NULL && !(strcmp(lastpath, "" )) )
        {
//        this->AddModelDialogButton->SetInitialFileName (lastpath);
        }
      }
    else
      {
      this->AddModelDialogButton->GetLoadSaveDialog()->SetLastPath ( this->GetLoadDirectory() );
//      this->AddModelDialogButton->SetInitialFileName ( this->GetLoadDirectory() );
      }
    this->AddModelDialogButton->TrimPathFromFileNameOff();
    this->AddModelDialogButton->SetMaximumFileNameLength (128 );
    this->AddModelDialogButton->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->AddModelDialogButton->GetLoadSaveDialog()->SetFileTypes(
                                                             "{ {model} {*.*} }");    
    this->AddModelDialogButton->SetBalloonHelpString ( "Select a 3D model from a pop-up file browser." );

    //--- Add model directory button
    vtkKWLabel *l1 = vtkKWLabel::New();
    l1->SetParent ( this->AddModelWindow );
    l1->Create();
    l1->SetText ( "Select model directory:" );
    this->AddModelDirectoryDialogButton = vtkKWLoadSaveButton::New();
    this->AddModelDirectoryDialogButton->SetParent ( this->AddModelWindow );
    this->AddModelDirectoryDialogButton->Create();
    if ( this->GetLoadDirectory() == NULL )
      {
      this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry ("OpenPath");
      const char *lastpath = this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->GetLastPath();
      if ( lastpath != NULL && !(strcmp(lastpath, "" )) )
        {
//        this->AddModelDirectoryDialogButton->SetInitialFileName (lastpath);
        }
      }
    else
      {
      this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->SetLastPath ( this->GetLoadDirectory() );
//      this->AddModelDirectoryDialogButton->SetInitialFileName ( this->GetLoadDirectory() );
      }
    this->AddModelDirectoryDialogButton->TrimPathFromFileNameOff();
    this->AddModelDirectoryDialogButton->SetMaximumFileNameLength (128 );
    this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->ChooseDirectoryOn();
    this->AddModelDirectoryDialogButton->SetBalloonHelpString ( "Select a directory from which 3D models (*.vtk) will be loaded." );

    this->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", l0->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1   -padx 2 -pady 2 -ipadx 2 -ipady 2 -sticky w", this->AddModelDialogButton->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e", l1->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 1   -padx 2 -pady 2 -ipadx 2 -ipady 2 -sticky w", this->AddModelDirectoryDialogButton->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 0 -weight 0", this->AddModelWindow->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 1", this->AddModelWindow->GetWidgetName() );
    
    l0->Delete();
    l1->Delete();
    }

  //--- add observers.
  this->AddModelDialogButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  // display
  this->AddModelWindow->DeIconify();
  this->AddModelWindow->Raise();
  if ( app )
    {
    app->Script ( "grab %s", this->AddModelWindow->GetWidgetName() );
    app->ProcessIdleTasks();
    }
  this->Script ( "update idletasks");

}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::WithdrawAddScalarOverlayWindow ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app && this->AddOverlayWindow )
    {
    app->Script ( "grab release %s", this->AddOverlayWindow->GetWidgetName() );
    }

  if ( this->AddOverlayDialogButton )
    {
    this->AddOverlayDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModelSelector )
    {
    this->ModelSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->AddOverlayWindow )
    {
    this->AddOverlayWindow->Withdraw();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RaiseAddScalarOverlayWindow ( )
{
  //--- create window if not already created.
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app == NULL )
    {
    vtkErrorMacro ( "RaiseAddScalarOverlayWindow: got NULL SlicerApplication");
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI ( );
  if ( appGUI == NULL )
    {
    vtkErrorMacro ( "RaiseAddScalarOverlayWindow: got NULL SlicerApplicationGUI");
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "RaiseAddScalarOverlayWindow: got NULL MainSlicerWindow");
    return;
    }

  if ( this->LoadDirectory == NULL )
    {
    if ( app )
      {
      if ( app->GetTemporaryDirectory() )
        {
        this->SetLoadDirectory(app->GetTemporaryDirectory() );
        }
      }
    }
  
    if ( this->AddOverlayWindow == NULL )
    {
    //-- top level container.
    this->AddOverlayWindow = vtkKWTopLevel::New();
    this->AddOverlayWindow->SetMasterWindow (win );
    this->AddOverlayWindow->SetApplication ( this->GetApplication() );
    this->AddOverlayWindow->Create();
    if ( this->GetLoadOverlayButton() )
      {
      int px, py;
      vtkKWTkUtilities::GetWidgetCoordinates( this->GetLoadOverlayButton(), &px, &py );
      this->AddOverlayWindow->SetPosition ( px + 10, py + 10 );
      }
    this->AddOverlayWindow->SetBorderWidth ( 1 );
    this->AddOverlayWindow->SetReliefToFlat();
    this->AddOverlayWindow->SetTitle ( "Add a scalar overlay to a 3D model");
    this->AddOverlayWindow->SetSize ( 380, 150 );
    this->AddOverlayWindow->Withdraw();
    this->AddOverlayWindow->SetDeleteWindowProtocolCommand ( this, "DestroyAddScalarOverlayWindow");

    //-- create node selector
    this->ModelSelector = vtkSlicerNodeSelectorWidget::New();
    this->ModelSelector->SetParent( this->AddOverlayWindow );
    this->ModelSelector->Create();
    this->ModelSelector->AddNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
    this->ModelSelector->SetChildClassesEnabled(0);
    this->ModelSelector->SetShowHidden (1);
    this->ModelSelector->SetMRMLScene(this->GetMRMLScene());
    this->ModelSelector->GetWidget()->GetWidget()->SetWidth (24 );
    this->ModelSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ModelSelector->SetBorderWidth(2);
    this->ModelSelector->SetPadX(2);
    this->ModelSelector->SetPadY(2);
    this->ModelSelector->SetLabelText( "Select model for overlay:");
    this->ModelSelector->UnconditionalUpdateMenu();
    this->ModelSelector->SetBalloonHelpString("Select a model (from the scene) to which the overlay will be applied.");

    //--- Add model button
    vtkKWLabel *l1 = vtkKWLabel::New();
    l1->SetParent ( this->AddOverlayWindow );
    l1->Create();
    l1->SetText ( "  Select a scalar overlay:" );
    this->AddOverlayDialogButton = vtkKWLoadSaveButton::New();
    this->AddOverlayDialogButton->SetParent ( this->AddOverlayWindow );
    this->AddOverlayDialogButton->Create();
    if ( this->GetLoadDirectory() == NULL )
      {
      this->AddOverlayDialogButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry ("OpenPath");
      const char *lastpath = this->AddOverlayDialogButton->GetLoadSaveDialog()->GetLastPath();
      if ( lastpath != NULL && !(strcmp(lastpath, "" )) )
        {
//        this->AddOverlayDialogButton->SetInitialFileName (lastpath);
        }
      }
    else
      {
      this->AddOverlayDialogButton->GetLoadSaveDialog()->SetLastPath ( this->GetLoadDirectory() );
//      this->AddOverlayDialogButton->SetInitialFileName ( this->GetLoadDirectory() );
      }
    this->AddOverlayDialogButton->TrimPathFromFileNameOff();
    this->AddOverlayDialogButton->SetMaximumFileNameLength (128 );
    this->AddOverlayDialogButton->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->AddOverlayDialogButton->GetLoadSaveDialog()->SetFileTypes("{ {All} {.*} } { {Thickness} {.thickness} } { {Curve} {.curv} } { {Average Curve} {.avg_curv} } { {Sulc} {.sulc} } { {Area} {.area} } { {W} {.w} } { {Parcellation Annotation} {.annot} } { {Volume} {.mgz .mgh} } { {Label} {.label} }");
    this->AddOverlayDialogButton->SetBalloonHelpString ( "Select a scalar overlay and apply it to the selected model." );

    this->Script ( "grid %s -row 0 -column 0 -columnspan 2  -padx 2 -pady 2 -ipadx 2 -ipady 2 -sticky w", this->ModelSelector->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w", l1->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 1   -padx 2 -pady 2 -ipadx 2 -ipady 2 -sticky w", this->AddOverlayDialogButton->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 0 -weight 0", this->AddOverlayWindow->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 1", this->AddOverlayWindow->GetWidgetName() );
    
    l1->Delete();
    }

  //--- add observers.
  this->AddOverlayDialogButton->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  // display
  this->AddOverlayWindow->DeIconify();
  this->AddOverlayWindow->Raise();
  if ( app )
    {
    app->Script ( "grab %s", this->AddOverlayWindow->GetWidgetName() );
    app->ProcessIdleTasks();
    }
  this->Script ( "update idletasks");

}
  

//---------------------------------------------------------------------------
  void vtkSlicerDataGUI::DestroyAddModelWindow ( )
{
  if ( !this->AddModelWindow )
    {
    return;
    }
  if ( ! (this->AddModelWindow->IsCreated()) )
    {
    vtkErrorMacro ( "DestroyAddModelWindow: AddModelWindow is not created." );
    return;
    }
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app )
    {
    app->Script ( "grab release %s", this->AddModelWindow->GetWidgetName() );
    }
  this->AddModelWindow->Withdraw();

  if ( this->AddModelDialogButton )
    {
    this->AddModelDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddModelDialogButton->SetParent ( NULL );
    this->AddModelDialogButton->Delete();
    this->AddModelDialogButton = NULL;    
    }
  if ( this->AddModelDirectoryDialogButton )
    {
    this->AddModelDirectoryDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddModelDirectoryDialogButton->SetParent ( NULL);
    this->AddModelDirectoryDialogButton->Delete();
    this->AddModelDirectoryDialogButton = NULL;    
    }
  if ( this->AddModelWindow )
    {
    this->AddModelWindow->Delete();
    this->AddModelWindow = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::DestroyAddScalarOverlayWindow ( )
{
  if ( !this->AddOverlayWindow )
    {
    return;
    }
  if ( ! (this->AddOverlayWindow->IsCreated()) )
    {
    vtkErrorMacro ( "DestroyAddOverlayWindow: AddOverlayWindow is not created." );
    return;
    }
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app )
    {
    app->Script ( "grab release %s", this->AddOverlayWindow->GetWidgetName() );
    }
  this->AddOverlayWindow->Withdraw();

  if ( this->ModelSelector  )
    {
    this->ModelSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelSelector->SetParent ( NULL );
    this->ModelSelector->Delete();
    this->ModelSelector = NULL;
    }

  if ( this->AddOverlayDialogButton )
    {
    this->AddOverlayDialogButton->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddOverlayDialogButton->SetParent (NULL);
    this->AddOverlayDialogButton->Delete();
    this->AddOverlayDialogButton = NULL;
    }

  if ( this->AddOverlayWindow )
    {
    this->AddOverlayWindow->Delete();
    this->AddOverlayWindow = NULL;
    }
}

