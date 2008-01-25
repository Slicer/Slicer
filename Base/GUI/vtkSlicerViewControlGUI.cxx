#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkActorCollection.h"
#include "vtkSlicerInteractorStyle.h"
#include "vtkMath.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerViewControlGUI.h"
#include "vtkSlicerWindow.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTopLevel.h"
#include "vtkKWFrame.h"
#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWRenderWidget.h"
#include "vtkSlicerViewControlIcons.h"

// uncomment in order to stub out the NavZoom widget.
//#define NAVZOOMWIDGET_DEBUG


#define DEGREES2RADIANS 0.0174532925
#define RADIANS2DEGREES 57.295779513
#define NUM_ZOOM_INCREMENTS 20.0


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewControlGUI );
vtkCxxRevisionMacro ( vtkSlicerViewControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::vtkSlicerViewControlGUI ( )
{
  
  this->NavigationRenderPending = 0;
  this->ZoomRenderPending = 0;
  this->EntryUpdatePending = 0;
  this->SceneClosing = false;
  this->ProcessingMRMLEvent = 0;
  this->RockCount = 0;
  this->NavigationZoomWidgetWid = 150;
  this->NavigationZoomWidgetHit = 80;

  this->SliceMagnification = 10.0;
  this->SliceInteracting = 0;

  this->SlicerViewControlIcons = NULL;
  this->SpinButton = NULL;
  this->RockButton = NULL;
  this->OrthoButton = NULL;

  this->CenterButton = NULL;
  this->StereoButton = NULL;
  this->SelectViewButton = NULL;
  this->SelectCameraButton = NULL;
  this->LookFromButton = NULL;
  this->RotateAroundButton = NULL;
  this->ZoomEntry = NULL;
  this->VisibilityButton = NULL;

  //--- ui for the ViewControlFrame
  this->ViewAxisAIconButton = NULL;
  this->ViewAxisPIconButton = NULL;
  this->ViewAxisRIconButton = NULL;
  this->ViewAxisLIconButton = NULL;
  this->ViewAxisSIconButton = NULL;
  this->ViewAxisIIconButton = NULL;
  this->ViewAxisCenterIconButton = NULL;
  this->ViewAxisTopCornerIconButton = NULL;
  this->ViewAxisBottomCornerIconButton = NULL;

  // temporary thing until navzoom window is built.
  this->NavigationWidget = NULL;
  this->ZoomWidget = NULL;
  this->NavigationZoomFrame = NULL;

  this->SliceMagnifier = NULL;
  this->SliceMagnifierCursor = NULL;
  this->SliceMagnifierMapper = NULL;
  this->SliceMagnifierActor = NULL;

  this->FOVBox = NULL;
  this->FOVBoxMapper = NULL;
  this->FOVBoxActor = NULL;

  //Enable/Disable navigation window
  this->EnableDisableNavButton;
  
  this->ViewNode = NULL;
  this->RedSliceNode = NULL;
  this->YellowSliceNode = NULL;
  this->GreenSliceNode = NULL;
  this->RedSliceEvents = NULL;
  this->YellowSliceEvents = NULL;
  this->GreenSliceEvents = NULL;
  this->MainViewerEvents = NULL;

  this->SetAndObserveMRMLScene ( NULL );
  this->SetApplicationGUI ( NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::TearDownGUI ( )
{
  this->RemoveSliceEventObservers();
  this->RemoveMainViewerEventObservers();
  this->SetAndObserveMRMLScene ( NULL );
  this->SetApplicationGUI ( NULL);
  this->SetApplication ( NULL );
}


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::~vtkSlicerViewControlGUI ( )
{

  this->NavigationRenderPending = 0;
  this->ZoomRenderPending = 0;
  this->EntryUpdatePending = 0;
  this->SceneClosing = false;
  this->RockCount = 0;  
  this->SliceInteracting = 0;

  if ( this->SliceMagnifier )
    {
    this->SliceMagnifier->Delete();
    this->SliceMagnifier = NULL;
    }
  if ( this->SliceMagnifierCursor )
    {
    this->SliceMagnifierCursor->Delete();
    this->SliceMagnifierCursor = NULL;
    }
  if ( this->SliceMagnifierMapper)
    {
    this->SliceMagnifierMapper->Delete();
    this->SliceMagnifierMapper = NULL;
    }
  if ( this->SliceMagnifierActor)
    {
    this->SliceMagnifierActor->Delete();
    this->SliceMagnifierActor = NULL;
    }
  if ( this->SlicerViewControlIcons )
    {
    this->SlicerViewControlIcons->Delete ( );
    this->SlicerViewControlIcons = NULL;
    }
   //--- widgets from ViewControlFrame
  if ( this->SpinButton ) 
    {
    this->SpinButton->SetParent ( NULL );
    this->SpinButton->Delete();
    this->SpinButton = NULL;
    }
  if ( this->RockButton) 
    {
    this->RockButton->SetParent ( NULL );
    this->RockButton->Delete();
    this->RockButton = NULL;
    }
  if ( this->OrthoButton ) 
    {
    this->OrthoButton->SetParent ( NULL );
    this->OrthoButton->Delete();
    this->OrthoButton = NULL;
    }
  if ( this->CenterButton ) 
    {
    this->CenterButton->SetParent ( NULL );      
    this->CenterButton->Delete();
    this->CenterButton = NULL;
    }
  if ( this->SelectViewButton ) 
    {
    this->SelectViewButton->SetParent ( NULL );
    this->SelectViewButton->Delete();
    this->SelectViewButton = NULL;
    }
  if ( this->SelectCameraButton ) 
    {
    this->SelectCameraButton->SetParent ( NULL );
    this->SelectCameraButton->Delete();
    this->SelectCameraButton = NULL;
    }

  if ( this->StereoButton ) 
    {
    this->StereoButton->SetParent ( NULL );
    this->StereoButton->Delete();
    this->StereoButton = NULL;
    }
  if ( this->VisibilityButton ) 
    {
    this->VisibilityButton->SetParent ( NULL );
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }
  if ( this->LookFromButton )
    {
    this->LookFromButton->SetParent (NULL );
    this->LookFromButton->Delete ();
    this->LookFromButton = NULL;
    }
  if ( this->RotateAroundButton )
    {
    this->RotateAroundButton->SetParent ( NULL );
    this->RotateAroundButton->Delete ( );
    this->RotateAroundButton = NULL;
    }
  if ( this->ZoomEntry )
    {
    this->ZoomEntry->SetParent ( NULL );
    this->ZoomEntry->Delete ( );
    this->ZoomEntry = NULL;
    }
  if ( this->ViewAxisAIconButton ) 
    {
    this->ViewAxisAIconButton->SetParent ( NULL );      
    this->ViewAxisAIconButton->Delete ( );
    this->ViewAxisAIconButton = NULL;
    }
  if ( this->ViewAxisPIconButton ) 
    {
    this->ViewAxisPIconButton->SetParent ( NULL );
    this->ViewAxisPIconButton->Delete ( );
    this->ViewAxisPIconButton = NULL;
   }
  if ( this->ViewAxisRIconButton ) 
    {
    this->ViewAxisRIconButton->SetParent ( NULL );
    this->ViewAxisRIconButton->Delete ( );
    this->ViewAxisRIconButton = NULL;
    }
  if ( this->ViewAxisLIconButton ) 
    {
    this->ViewAxisLIconButton->SetParent ( NULL );
    this->ViewAxisLIconButton->Delete ( );
    this->ViewAxisLIconButton = NULL;
    }
  if ( this->ViewAxisSIconButton ) 
    {
    this->ViewAxisSIconButton->SetParent ( NULL );
    this->ViewAxisSIconButton->Delete ( );
    this->ViewAxisSIconButton = NULL;
    }
  if ( this->ViewAxisIIconButton ) 
    {
    this->ViewAxisIIconButton->SetParent ( NULL );
    this->ViewAxisIIconButton->Delete ( );
    this->ViewAxisIIconButton = NULL;
    }
  if ( this->ViewAxisCenterIconButton ) 
    {
    this->ViewAxisCenterIconButton->SetParent ( NULL );
    this->ViewAxisCenterIconButton->Delete ( );
    this->ViewAxisCenterIconButton = NULL;
    }
  if ( this->ViewAxisTopCornerIconButton ) 
    {
    this->ViewAxisTopCornerIconButton->SetParent ( NULL );
    this->ViewAxisTopCornerIconButton->Delete ( );
    this->ViewAxisTopCornerIconButton = NULL;
    }
  if ( this->ViewAxisBottomCornerIconButton ) 
    {
    this->ViewAxisBottomCornerIconButton->SetParent ( NULL );
    this->ViewAxisBottomCornerIconButton->Delete ( );
    this->ViewAxisBottomCornerIconButton = NULL;
    }
  if ( this->NavigationWidget )
    {
    this->NavigationWidget->SetParent ( NULL );
    this->NavigationWidget->Delete ();
    this->NavigationWidget = NULL;
    }
  if( this->EnableDisableNavButton)
  {
      this->EnableDisableNavButton->SetParent ( NULL );
      this->EnableDisableNavButton->Delete ();
      this->EnableDisableNavButton = NULL;
  }
  if ( this->ZoomWidget )
    {
    this->ZoomWidget->SetParent ( NULL );
    this->ZoomWidget->Delete ();
    this->ZoomWidget = NULL;
    }
  if ( this->NavigationZoomFrame )
    {
    this->NavigationZoomFrame->SetParent ( NULL );
    this->NavigationZoomFrame->Delete ();
    this->NavigationZoomFrame = NULL;
    }
  if ( this->FOVBox )
    {
    this->FOVBox->Delete();
    this->FOVBox = NULL;
    }
  if ( this->FOVBoxMapper )
    {
    this->FOVBoxMapper->Delete();
    this->FOVBoxMapper = NULL;
    }
  if (this->FOVBoxActor)
    {
    this->FOVBoxActor->Delete();
    this->FOVBoxActor = NULL;
    }

  vtkSetAndObserveMRMLNodeMacro ( this->ViewNode, NULL );
  vtkSetAndObserveMRMLNodeMacro ( this->RedSliceNode, NULL );
  vtkSetAndObserveMRMLNodeMacro ( this->GreenSliceNode, NULL );
  vtkSetAndObserveMRMLNodeMacro ( this->YellowSliceNode, NULL );
  this->RemoveSliceEventObservers();
  this->RemoveMainViewerEventObservers();
  this->SetRedSliceEvents(NULL);
  this->SetYellowSliceEvents(NULL);
  this->SetGreenSliceEvents(NULL);
  this->SetMainViewerEvents ( NULL );
  this->SetAndObserveMRMLScene ( NULL );
  this->SetApplicationGUI ( NULL );
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MakeViewControlRolloverBehavior ( )
{

  //--- configure and bind for rollover interaction
  //--- as there are no events on labels, we'll do this the old fashioned way.
  
  this->ViewAxisAIconButton->SetBorderWidth (0);
  this->ViewAxisAIconButton->SetBinding ( "<Enter>",  this, "EnterViewAxisACallback");
  this->ViewAxisAIconButton->SetBinding ( "<Leave>",  this, "LeaveViewAxisACallback");
  this->ViewAxisAIconButton->SetBinding ( "<Button-1>", this, "ViewControlACallback");
  this->Script ( "%s ListMethods", this->GetTclName() );

  this->ViewAxisPIconButton->SetBorderWidth (0);
  this->ViewAxisPIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisPCallback");
  this->ViewAxisPIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisPCallback");
  this->ViewAxisPIconButton->SetBinding ( "<Button-1>", this, "ViewControlPCallback");

  this->ViewAxisRIconButton->SetBorderWidth (0);
  this->ViewAxisRIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisRCallback");
  this->ViewAxisRIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisRCallback");
  this->ViewAxisRIconButton->SetBinding ( "<Button-1>", this, "ViewControlRCallback");
  
  this->ViewAxisLIconButton->SetBorderWidth (0);
  this->ViewAxisLIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisLCallback");
  this->ViewAxisLIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisLCallback");
  this->ViewAxisLIconButton->SetBinding ( "<Button-1>", this, "ViewControlLCallback");

  this->ViewAxisSIconButton->SetBorderWidth (0);
  this->ViewAxisSIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisSCallback");
  this->ViewAxisSIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisSCallback");
  this->ViewAxisSIconButton->SetBinding ( "<Button-1>", this, "ViewControlSCallback");
  
  this->ViewAxisIIconButton->SetBorderWidth (0);
  this->ViewAxisIIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisICallback");
  this->ViewAxisIIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisICallback");
  this->ViewAxisIIconButton->SetBinding ( "<Button-1>", this, "ViewControlICallback");
  
  this->ViewAxisCenterIconButton->SetBorderWidth (0);
  this->ViewAxisTopCornerIconButton->SetBorderWidth (0);
  this->ViewAxisBottomCornerIconButton->SetBorderWidth (0);
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  //TODO: need to print everything
  this->vtkObject::PrintSelf ( os, indent );

  // eventuall these get moved into the view node...
  os << indent << "SlicerViewControlGUI: " << this->GetClassName ( ) << "\n";

  // class widgets
  os << indent << "ViewAxisAIconButton: " << this->GetViewAxisAIconButton (  ) << "\n";
  os << indent << "ViewAxisPIconButton: " << this->GetViewAxisPIconButton (  ) << "\n";
  os << indent << "ViewAxisRIconButton: " << this->GetViewAxisRIconButton (  ) << "\n";
  os << indent << "ViewAxisLIconButton: " << this->GetViewAxisLIconButton (  ) << "\n";
  os << indent << "ViewAxisSIconButton: " << this->GetViewAxisSIconButton (  ) << "\n";
  os << indent << "ViewAxisIIconButton: " << this->GetViewAxisIIconButton (  ) << "\n";
  os << indent << "ViewAxisCenterIconButton: " << this->GetViewAxisCenterIconButton (  ) << "\n";
  os << indent << "ViewAxisTopCornerIconButton: " << this->GetViewAxisTopCornerIconButton (  ) << "\n";
  os << indent << "ViewAxisBottomCornerIconButton: " << this->GetViewAxisBottomCornerIconButton (  ) << "\n";
  os << indent << "SpinButton: " << this->GetSpinButton(  ) << "\n";
  os << indent << "RockButton: " << this->GetRockButton(  ) << "\n";
  os << indent << "OrthoButton: " << this->GetOrthoButton(  ) << "\n";
  os << indent << "LookFromButton: " << this->GetLookFromButton(  ) << "\n";  
  os << indent << "RotateAroundButton: " << this->GetRotateAroundButton(  ) << "\n";
  os << indent << "CenterButton: " << this->GetCenterButton(  ) << "\n";
  os << indent << "StereoButton: " << this->GetStereoButton(  ) << "\n";
  os << indent << "SelectViewButton: " << this->GetSelectViewButton(  ) << "\n";
  os << indent << "SelectCameraButton: " << this->GetSelectCameraButton(  ) << "\n";
  os << indent << "VisibilityButton: " << this->GetVisibilityButton(  ) << "\n";
  os << indent << "ZoomEntry: " << this->GetZoomEntry(  ) << "\n";
  os << indent << "SlicerViewControlIcons: " << this->GetSlicerViewControlIcons(  ) << "\n";
  os << indent << "ApplicationGUI: " << this->GetApplicationGUI(  ) << "\n";

  os << indent << "NavigationWidget: " << this->GetNavigationWidget(  ) << "\n";    
  os << indent << "ZoomWidget: " << this->GetZoomWidget(  ) << "\n";    
  os << indent << "NavigationZoomFrame: " << this->GetNavigationZoomFrame(  ) << "\n";

  os << indent << "SliceMagnifier: " << this->GetSliceMagnifier(  ) << "\n";
  os << indent << "SliceMagnifierCursor: " << this->GetSliceMagnifierCursor(  ) << "\n";    
  os << indent << "SliceMagnifierMapper: " << this->GetSliceMagnifierMapper(  ) << "\n";    
  os << indent << "SliceMagnifierActor: " << this->GetSliceMagnifierActor(  ) << "\n";    
  
  os << indent << "FOVBox: " << this->GetFOVBox(  ) << "\n";
  os << indent << "FOVBoxMapper: " << this->GetFOVBoxMapper(  ) << "\n";
  os << indent << "FOVBoxActor: " << this->GetFOVBoxActor(  ) << "\n";

  os << indent << "ViewNode: " << this->GetViewNode(  ) << "\n";    
  os << indent << "RedSliceNode: " << this->GetRedSliceNode(  ) << "\n";
  os << indent << "GreenSliceNode: " << this->GetGreenSliceNode(  ) << "\n";    
  os << indent << "YellowSliceNode: " << this->GetYellowSliceNode(  ) << "\n";    
  os << indent << "RedSliceEvents: " << this->GetRedSliceEvents(  ) << "\n";    
  os << indent << "GreenSliceEvents: " << this->GetGreenSliceEvents(  ) << "\n";    
  os << indent << "YellowSliceEvents: " << this->GetYellowSliceEvents(  ) << "\n";    
  os << indent << "MainViewerEvents: " << this->GetMainViewerEvents() << "\n";

  os << indent << "NavigationRenderPending: " << this->GetNavigationRenderPending(  ) << "\n";
  os << indent << "ZoomRenderPending: " << this->GetZoomRenderPending(  ) << "\n";
  os << indent << "EntryUpdatePending: " << this->GetEntryUpdatePending(  ) << "\n";  
  os << indent << "ProcessingMRMLEvent: " << this->GetProcessingMRMLEvent(  ) << "\n";  
  os << indent << "RockCount: " << this->GetRockCount(  ) << "\n";      
  os << indent << "NavigationZoomWidgetWid: " << this->GetNavigationZoomWidgetWid(  ) << "\n";    
  os << indent << "NavigationZoomWidgetHit: " << this->GetNavigationZoomWidgetHit(  ) << "\n";
  os << indent << "SliceMagnification: " << this->GetSliceMagnification(  ) << "\n";
  os << indent << "SliceInteracting: " << this->GetSliceInteracting(  ) << "\n";      
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveGUIObservers ( )
{
  // FILL IN
    this->LookFromButton->RemoveObservers (vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RotateAroundButton->RemoveObservers (vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SpinButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RockButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->OrthoButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->StereoButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->CenterButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectViewButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectCameraButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VisibilityButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ZoomEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );


}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddGUIObservers ( )
{
  // FILL IN
    this->LookFromButton->AddObserver (vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RotateAroundButton->AddObserver (vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SpinButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RockButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->OrthoButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->StereoButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->CenterButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectViewButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectCameraButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VisibilityButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ZoomEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddMainViewerEventObservers()
{
  
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->MainViewerEvents != NULL )
      {
      this->MainViewerEvents->AddObserver ( vtkCommand::EndInteractionEvent, this->GUICallbackCommand );
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveMainViewerEventObservers()
{
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->MainViewerEvents != NULL )
      {
      this->MainViewerEvents->RemoveObservers ( vtkCommand::EndInteractionEvent, this->GUICallbackCommand );
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateSliceGUIInteractorStyles ( )
{
  // get all views from the scene
  // and observe active view.
  if (this->SceneClosing)
    {
    return;
    }

  // Find current SliceGUIs; if there are none, do nothing.
  if ( ( this->GetApplicationGUI()->GetMainSliceGUI0() == NULL ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI1() == NULL ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI2() == NULL ))
    {
    return;
    }

  // If the interactor and these references are out of sync...
  if ( ( this->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->RedSliceEvents ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI1()->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->YellowSliceEvents ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI2()->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->GreenSliceEvents ) )
    {
    this->RemoveSliceEventObservers();
    this->SetRedSliceEvents(NULL );
    this->SetYellowSliceEvents(NULL );
    this->SetGreenSliceEvents(NULL );

    this->SetRedSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI0()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetYellowSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI1()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetGreenSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI2()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->AddSliceEventObservers();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateFromMRML()
{


  // called:
  // 1. whenever any new node is created or deleted
  // 2. when a new view or camera has been selected
  // Needs to remove old observers, put new
  // observers on the current camera and view,
  // repopulate the NavigationZoom widget's actors, etc.,
  // and rerender the NavigationZoom widget's view.

  this->UpdateViewFromMRML();
  this->UpdateSlicesFromMRML();
  this->RequestNavigationRender ( );
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateViewFromMRML()
{
    if (this->SceneClosing)
    {
    return;
    }

  vtkMRMLViewNode *node =  vtkMRMLViewNode::SafeDownCast (
       this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLViewNode"));

  if ( this->ViewNode != NULL && node != NULL && this->ViewNode != node)
    {
    // local ViewNode is out of sync with the scene
    vtkSetAndObserveMRMLNodeMacro ( this->ViewNode, NULL );
    }
  if ( this->ViewNode != NULL && this->MRMLScene->GetNodeByID(this->ViewNode->GetID()) == NULL)
    {
    // local node not in the scene
    vtkSetAndObserveMRMLNodeMacro ( this->ViewNode, NULL );
    }
  if ( this->ViewNode == NULL )
    {
    if ( node == NULL )
      {
      // no view in the scene and local
      // create an active camera
      node = vtkMRMLViewNode::New();
      this->MRMLScene->AddNode(node);
      node->Delete();
      }

    vtkIntArray  *events = vtkIntArray::New();
    events->InsertNextValue( vtkMRMLViewNode::AnimationModeEvent);
    events->InsertNextValue( vtkMRMLViewNode::RenderModeEvent); 
    events->InsertNextValue( vtkMRMLViewNode::StereoModeEvent);
    events->InsertNextValue( vtkMRMLViewNode::VisibilityEvent);
    events->InsertNextValue( vtkMRMLViewNode::BackgroundColorEvent);    
    vtkSetAndObserveMRMLNodeEventsMacro ( this->ViewNode, node, events );
    events->Delete();
    events = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateSlicesFromMRML()
{
  if (this->SceneClosing)
    {
    return;
    }

  // update Slice nodes
  vtkMRMLSliceNode *node= NULL;
  vtkMRMLSliceNode *nodeRed= NULL;
  vtkMRMLSliceNode *nodeGreen= NULL;
  vtkMRMLSliceNode *nodeYellow= NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (!strcmp(node->GetLayoutName(), "Red"))
      {
      nodeRed = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Green"))
      {
      nodeGreen = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Yellow"))
      {
      nodeYellow = node;
      }
    node = NULL;
    }

  // set and observe
  if (nodeRed != this->RedSliceNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->RedSliceNode, nodeRed);
    }
  if (nodeGreen != this->GreenSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->GreenSliceNode, nodeGreen);
   }
  if (nodeYellow != this->YellowSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->YellowSliceNode, nodeYellow);
   }

  // tidy up.
  nodeRed = NULL;
  nodeGreen = NULL;
  nodeYellow = NULL;
}





//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RequestNavigationRender()
{


#ifndef NAVZOOMWIDGET_DEBUG
  if(!this->EnableDisableNavButton->GetSelectedState())
  {
          this->NavigationWidget->RemoveAllViewProps();
          this->NavigationWidget->Render();
          return;
  }
  if (this->GetNavigationRenderPending())
    {
    return;
    }

  this->SetNavigationRenderPending(1);
  this->Script("after idle \"%s NavigationRender\"", this->GetTclName());
#endif
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RequestZoomRender()
{
#ifndef NAVZOOMWIDGET_DEBUG
  if (this->GetZoomRenderPending())
    {
    return;
    }

  this->SetZoomRenderPending(1);
  this->Script("after idle \"%s ZoomRender\"", this->GetTclName());
#endif
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ZoomRender()
{
#ifndef NAVZOOMWIDGET_DEBUG
  if ( this->SliceMagnifier->GetInput() != NULL )
    {
    this->ZoomWidget->Render();
    }
  this->SetZoomRenderPending(0);
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::NavigationRender()
{

#ifndef  NAVZOOMWIDGET_DEBUG
  this->UpdateNavigationWidgetViewActors ( );
  this->ConfigureNavigationWidgetRender ( );
  this->NavigationWidget->Render();
  this->SetNavigationRenderPending(0);
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ResetNavigationCamera ( )
{
  double center[3];
  double distance;
  double vn[3], *vup;
  double bounds[6];
  double viewAngle = 20.0;
  
  vtkRenderer *ren = this->NavigationWidget->GetRenderer();
  vtkCamera *cam = ren->GetActiveCamera();
  
  ren->ComputeVisiblePropBounds( bounds );
  if (!vtkMath::AreBoundsInitialized(bounds))
    {
    vtkDebugMacro( << "Cannot reset camera!" );
    }
  else
    {
    ren->InvokeEvent(vtkCommand::ResetCameraEvent, ren);

    if ( cam != NULL )
      {
      cam->GetViewPlaneNormal(vn);
      }
    else
      {
      vtkErrorMacro(<< "Trying to reset non-existant camera");
      return;
      }

    center[0] = (bounds[0] + bounds[1])/2.0;
    center[1] = (bounds[2] + bounds[3])/2.0;
    center[2] = (bounds[4] + bounds[5])/2.0;

    double w1 = bounds[1] - bounds[0];
    double w2 = bounds[3] - bounds[2];
    double w3 = bounds[5] - bounds[4];
    w1 *= w1;
    w2 *= w2;
    w3 *= w3;
    double radius = w1 + w2 + w3;

    // If we have just a single point, pick a radius of 1.0
    radius = (radius==0)?(1.0):(radius);

    // compute the radius of the enclosing sphere
    radius = sqrt(radius)*0.5;

    // default so that the bounding sphere fits within the view fustrum
    // compute the distance from the intersection of the view frustum with the
    // bounding sphere. Basically in 2D draw a circle representing the bounding
    // sphere in 2D then draw a horizontal line going out from the center of
    // the circle. That is the camera view. Then draw a line from the camera
    // position to the point where it intersects the circle. (it will be tangent
    // to the circle at this point, this is important, only go to the tangent
    // point, do not draw all the way to the view plane). Then draw the radius
    // from the tangent point to the center of the circle. You will note that
    // this forms a right triangle with one side being the radius, another being
    // the target distance for the camera, then just find the target dist using
    // a sin.
    distance = radius/sin(viewAngle*vtkMath::Pi()/360.0);

    // check view-up vector against view plane normal
    vup = cam->GetViewUp();
    if ( fabs(vtkMath::Dot(vup,vn)) > 0.999 )
      {
      vtkWarningMacro(<<"Resetting view-up since view plane normal is parallel");
      cam->SetViewUp(-vup[2], vup[0], vup[1]);
      }

    // update the camera
    cam->SetFocalPoint(center[0],center[1],center[2]);
    cam->SetPosition(center[0]+distance*vn[0],
                     center[1]+distance*vn[1],
                     center[2]+distance*vn[2]);

    ren->ResetCameraClippingRange( bounds );

    // setup default parallel scale
    cam->SetParallelScale(radius);
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  // Right now this class contains state variables that will be moved
  // to a vtkMRMLViewNode in the next iteration.


  if ( this->GetApplicationGUI() != NULL )
    {
    //Check for abort during rendering of navigation widget
    if(caller==this->NavigationWidget->GetRenderWindow()&&event==vtkCommand::AbortCheckEvent)
      {
      this->CheckAbort();
      return;
      }
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( appGUI->GetApplication() );
    if ( app != NULL )
      {
      vtkKWCheckButton *b = vtkKWCheckButton::SafeDownCast ( caller );
      vtkKWRadioButton *r = vtkKWRadioButton::SafeDownCast ( caller );
      vtkKWPushButton *p = vtkKWPushButton::SafeDownCast ( caller );
      vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
      vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
      vtkKWScale *s = vtkKWScale::SafeDownCast ( caller );
      vtkSlicerInteractorStyle *istyle = vtkSlicerInteractorStyle::SafeDownCast ( caller );
      vtkSlicerViewerInteractorStyle *vstyle = vtkSlicerViewerInteractorStyle::SafeDownCast ( caller );

      // if user has requested a larger or smaller font,
      // execute this.
      if (0)
        {
        this->ReconfigureGUIFonts();
        }

      // has interaction occured in the slice viewers?
      if ( istyle == this->RedSliceEvents || istyle == this->YellowSliceEvents || istyle == this->GreenSliceEvents)
        {
#ifndef NAVZOOMWIDGET_DEBUG
        this->SliceViewMagnify( event, istyle );
#endif
        }
#ifndef NAVZOOMWIDGET_DEBUG
      // has interaction occured in the main viewer?
      if (vstyle == this->MainViewerEvents )
        {
        this->RequestNavigationRender();
        }
#endif
      
      double val;
      // Zoom entry
      if ( e == this->ZoomEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        val = this->ZoomEntry->GetWidget()->GetValueAsDouble();
        val = val/100.0;
        if ( val > 0.0)
          {
          this->MainViewZoom ( val );
          }
        }
      // Make requested changes to the ViewNode      
      // save state for undo
      if ( m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->VisibilityButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->SelectViewButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||           
           m == this->SelectCameraButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           p == this->CenterButton && event == vtkKWPushButton::InvokedEvent ||                      
           p == this->OrthoButton && event == vtkKWPushButton::InvokedEvent ||                      
           b == this->SpinButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           b == this->RockButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           r == this->RotateAroundButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           r == this->LookFromButton && event == vtkKWCheckButton::SelectedStateChangedEvent)
        {
        vtkMRMLViewNode *vn = this->GetActiveView();
        if ( vn != NULL )
          {
          appGUI->GetMRMLScene()->SaveStateForUndo( vn );
          if ( m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            if ( !strcmp (this->StereoButton->GetValue(), "No stereo"))
              {
              vn->SetStereoType( vtkMRMLViewNode::NoStereo );
              }
            else if (!strcmp (this->StereoButton->GetValue(), "Red/Blue"))
              {
              vn->SetStereoType( vtkMRMLViewNode::RedBlue );
              }
            else if (!strcmp (this->StereoButton->GetValue(), "Anaglyph"))
              {
              vn->SetStereoType( vtkMRMLViewNode::Anaglyph );
              }
            else if (!strcmp (this->StereoButton->GetValue(), "CrystalEyes"))
              {
              vn->SetStereoType( vtkMRMLViewNode::CrystalEyes );
              }
            else if (!strcmp (this->StereoButton->GetValue(), "Interlaced"))
              {
              vn->SetStereoType( vtkMRMLViewNode::Interlaced );            
              }
            }
          else if ( m == this->VisibilityButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // Get all menu items
            if ( vn->GetFiducialsVisible() != m->GetItemSelectedState("Fiducial points"))
              {
//                this->SetMRMLFiducialPointVisibility (m->GetItemSelectedState("Fiducial points"));
              }
            if ( vn->GetFiducialLabelsVisible() !=m->GetItemSelectedState("Fiducial labels"))
              {
//                this->SetMRMLFiducialLabelVisibility (m->GetItemSelectedState("Fiducial labels"));
              }
            if ( vn->GetBoxVisible() !=m->GetItemSelectedState ("3D cube"))
              {
              vn->SetBoxVisible ( m->GetItemSelectedState ("3D cube"));
              this->RequestNavigationRender();
              }
            if ( vn->GetAxisLabelsVisible() != m->GetItemSelectedState ("3D axis labels"))
              {
              vn->SetAxisLabelsVisible (m->GetItemSelectedState ("3D axis labels"));
              this->RequestNavigationRender();
              }
            if ( m->GetItemSelectedState ("Light blue background" ) == 1 )
              {
              vn->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
              this->RequestNavigationRender();
              }
            else if ( m->GetItemSelectedState ("Black background" ) == 1 )
              {
              vn->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->Black );
              this->RequestNavigationRender();
              }
            else if ( m->GetItemSelectedState ("White background" ) == 1 )
              {
              vn->SetBackgroundColor (app->GetSlicerTheme()->GetSlicerColors()->White );
              this->RequestNavigationRender();
              }            
            }
          else if ( m == this->SelectViewButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            }
          else if ( m == this->SelectCameraButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            }
      
          if ( (p == this->CenterButton) && (event == vtkKWPushButton::InvokedEvent ) )
            {
            this->MainViewResetFocalPoint ( );
            this->RequestNavigationRender();
            }

          // toggle the Ortho/Perspective rendering state
          if ( (p == this->OrthoButton) && (event == vtkKWPushButton::InvokedEvent ) && vn )
            {
            if ( vn->GetRenderMode() == vtkMRMLViewNode::Orthographic )
              {
              vn->SetRenderMode (vtkMRMLViewNode::Perspective);
              }
            else if ( vn->GetRenderMode() == vtkMRMLViewNode::Perspective )
              {
              vn->SetRenderMode(vtkMRMLViewNode::Orthographic );
              }
            }

          //--- turn View Spin and Rocking on and off
          if ( (b == this->SpinButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent) && vn )
            {
            // toggle the Spin 
            if ( vn->GetAnimationMode() != vtkMRMLViewNode::Spin  && this->SpinButton->GetSelectedState() == 1 )
              {
              vn->SetAnimationMode ( vtkMRMLViewNode::Spin );
              }
            else if ( vn->GetAnimationMode() == vtkMRMLViewNode::Spin && this->SpinButton->GetSelectedState() == 0 )
              {
              vn->SetAnimationMode( vtkMRMLViewNode::Off );
              }
            }

          if ( (b == this->RockButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent) && vn )
            {
            // toggle the Rock 
            if ( vn->GetAnimationMode() != vtkMRMLViewNode::Rock && this->RockButton->GetSelectedState() == 1 )
              {
              vn->SetAnimationMode( vtkMRMLViewNode::Rock );
              }
            else if (vn->GetAnimationMode() == vtkMRMLViewNode::Rock && this->RockButton->GetSelectedState() == 0 )
              {
              vn->SetAnimationMode ( vtkMRMLViewNode::Off );
              }
            }

          //--- automatic camera control mode: switch 'rotate around axis' or 'look from direction'
          if (( r == this->RotateAroundButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent)  &&
              ( vn->GetViewAxisMode() == vtkMRMLViewNode::LookFrom) )
            {
            vn->SetViewAxisMode ( vtkMRMLViewNode::RotateAround );
            }

          if (( r == this->LookFromButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent ) &&
              (vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround) )
            {
            vn->SetViewAxisMode( vtkMRMLViewNode::LookFrom );
            }
          }
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewZoom(double factor )
{

#ifndef NAVZOOMWIDGET_DEBUG
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
  vtkRenderer *ren = NULL;
  vtkCamera *cam = NULL;

  if ( !appGUI )
    {
    return;
    }
    ren = appGUI->GetViewerWidget()->GetMainViewer()->GetRenderer();    

  if ( !ren )
    {
    return;
    }
  cam = ren->GetActiveCamera();

  if ( !cam )
    {
    return;
    }

  if (cam->GetParallelProjection())
    {
    cam->SetParallelScale(cam->GetParallelScale() / factor);
    }
  else
    {
    cam->Dolly(factor);
    ren->ResetCameraClippingRange();
    ren->UpdateLightsGeometryToFollowCamera();
    }
  appGUI->GetViewerWidget()->GetMainViewer()->Render();
  this->RequestNavigationRender();

  cam = NULL;
  ren = NULL;
  appGUI = NULL;
#endif
  
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SetMRMLFiducialPointVisibility ( int state)
{
  
  // Right now, fiducial visibility is stored in three places.
  // TODO: agree on where to keep it, and whether to override it.
/*
  if ( this->ApplicationGUI  )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      p->GetMRMLScene()->SaveStateForUndo( vn );
      // update ViewNode
      vn->SetFiducialsVisible (state );
      }
    }
*/
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SetMRMLFiducialLabelVisibility ( int state)
{

  // Right now, fiducial visibility is stored in three places.
  // TODO: agree on where to keep it, and whether to override it.
/*
  if ( this->ApplicationGUI  )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      // update ViewNode
      vn->SetFiducialLabelsVisible (state);
      }
    }
*/
}





//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::CreateFieldOfViewBoxActor ( )
{
#ifndef NAVZOOMWIDGET_DEBUG
  this->FOVBox = vtkOutlineSource::New();
  this->FOVBoxMapper = vtkPolyDataMapper::New();
  this->FOVBoxMapper->SetInput( this->FOVBox->GetOutput() );
  this->FOVBoxActor = vtkFollower::New();

  this->FOVBoxMapper->Update();
   
  this->FOVBoxActor->SetMapper( this->FOVBoxMapper );
  this->FOVBoxActor->SetPickable(0);
  this->FOVBoxActor->SetDragable(0);
  this->FOVBoxActor->SetVisibility(1);
  this->FOVBoxActor->SetScale(1.0, 1.0, 1.0);
  this->FOVBoxActor->GetProperty()->SetColor( 0.1, 0.45, 0.1 );
  this->FOVBoxActor->GetProperty()->SetLineWidth (2.0);
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::InitializeNavigationWidgetCamera ( )
{

#ifndef NAVZOOMWIDGET_DEBUG
  double camPos[3];
  double focalPoint[3];

  if ( this->GetApplicationGUI() != NULL )
    {
    vtkCamera *cam;
    vtkCamera *navcam;
    vtkRenderer *ren;
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    ren = p->GetViewerWidget()->GetMainViewer()->GetRenderer();
    cam = ren->GetActiveCamera();
    cam->GetPosition (camPos );
    cam->GetFocalPoint ( focalPoint );

    navcam = this->NavigationWidget->GetRenderer()->GetActiveCamera();
    navcam->SetPosition ( camPos );
    navcam->SetFocalPoint ( focalPoint );
    navcam->ComputeViewPlaneNormal();
    navcam->SetViewUp( cam->GetViewUp() );
    this->FOVBoxActor->SetCamera (navcam);
    this->FOVBox->SetBoxTypeToOriented ( );
    }
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ConfigureNavigationWidgetRender ( )
{
  
#ifndef NAVZOOMWIDGET_DEBUG
  // Scale the FOVBox actor to show the
  // MainViewer's window on the scene.
  if ( this->GetApplicationGUI() != NULL )
    {
        //Add an observer for check abort events
        if(!this->NavigationWidget->GetRenderWindow()->HasObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->GUICallbackCommand))
        {
            this->NavigationWidget->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent,(vtkCommand*)this->GUICallbackCommand);
        }

    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    // 3DViewer's renderer and its camera
    vtkRenderer *ren = p->GetViewerWidget()->GetMainViewer()->GetRenderer();
    vtkCamera *cam = ren->GetActiveCamera();
    
    // 3DViewer's camera configuration
    double *focalPoint = cam->GetFocalPoint ( );
    double *camPos= cam->GetPosition ( );
    double *vpn = cam->GetViewPlaneNormal ();
    double thetaV = (cam->GetViewAngle()) / 2.0;

    // camera distance, and distance of FOVBox from focalPoint
    double camDist = cam->GetDistance ();
    double boxDist = camDist * 0.89;

    // configure navcam based on main renderer's camera
    vtkRenderer *navren = this->NavigationWidget->GetRenderer();
    vtkCamera *navcam = this->NavigationWidget->GetRenderer()->GetActiveCamera();

    if ( navcam != NULL )
      {
      // give navcam the same parameters as MainViewer's ActiveCamera
      navcam->SetPosition ( camPos );
      navcam->SetFocalPoint ( focalPoint );
      navcam->SetViewUp( cam->GetViewUp() );
      navcam->ComputeViewPlaneNormal ( );

      // compute FOVBox height & width to correspond 
      // to the main viewer's size and aspect ratio, in world-coordinates,
      // positioned just behind the near clipping plane, to make sure
      // nothing in the scene occludes it.
      double boxHalfHit;
      if ( cam->GetParallelProjection() )
        {
        boxHalfHit = cam->GetParallelScale();
        }
      else
        {
        boxHalfHit = (camDist) * tan ( thetaV * DEGREES2RADIANS);
        }

      // 3D MainViewer height and width for computing aspect
      int mainViewerWid = ren->GetRenderWindow()->GetSize()[0];
      int mainViewerHit = ren->GetRenderWindow()->GetSize()[1];
      // width of the FOVBox that represents MainViewer window.
      double boxHalfWid = boxHalfHit * (double)mainViewerWid / (double)mainViewerHit;

      // configure and position the FOVBox
      double data [24];
      data[0] = -1.0;
      data[1] = -1.0;
      data[2] = 0.0;
      data[3] = 1.0;
      data[4] = -1.0;
      data[5] = 0.0;
      data[6] = -1.0;
      data[7] = 1.0;
      data[8] = 0.0;
      data[9] = 1.0;
      data[10] = 1.0;
      data[11] = 0.0;

      data[12] = -1.0;
      data[13] = -1.0;
      data[14] = 0.0;
      data[15] = 1.0;
      data[16] = -1.0;
      data[17] = 0.0;
      data[18] = -1.0;
      data[19] = 1.0;
      data[20] = 0.0;
      data[21] = 1.0;
      data[22] = 1.0;
      data[23] = 0.0;
      this->FOVBox->SetCorners ( data );
      // Position and scale FOVBox very close to camera,
      // to prevent things in the scene from occluding it.
      this->FOVBoxActor->SetScale ( boxHalfWid, boxHalfHit, 1.0);
      this->FOVBoxActor->SetPosition (focalPoint[0]+ (vpn[0]*boxDist),
                                      focalPoint[1] + (vpn[1]*boxDist),
                                      focalPoint[2] + (vpn[2]*boxDist));
      this->ResetNavigationCamera();
      // put the focal point back into the center of
      // the scene without the FOVBox included,
      // since ResetNavigationCamera moved it.
      navcam->SetFocalPoint ( focalPoint );
      navren->ResetCameraClippingRange();
      navren->UpdateLightsGeometryToFollowCamera();
      }
      navren = NULL;
      navcam = NULL;
      p = NULL;
    }
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateNavigationWidgetViewActors ( )
{

#ifndef NAVZOOMWIDGET_DEBUG
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkRenderer *ren;
    vtkActorCollection *mainActors;
    vtkActor *mainActor;
    vtkActorCollection *navActors;
    vtkActor *newActor;
    vtkPolyDataMapper *newMapper;
    double bounds[6];
    double dimension;
    double x,y,z;
    double cutoff = 0.1;
    double cutoffDimension;

    // iterate thru NavigationWidget's actor collection,
    // remove item, delete actor, delete mapper.
    navActors = this->NavigationWidget->GetRenderer()->GetActors();
    
    if (navActors != NULL )
      {
      this->NavigationWidget->RemoveAllViewProps();
      navActors->RemoveAllItems();
      }

    // get estimate of Main Viewer's visible scene max dimension;
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    ren = p->GetViewerWidget()->GetMainViewer()->GetRenderer();
    ren->ComputeVisiblePropBounds( bounds );
    x = bounds[1] - bounds[0];
    y = bounds[3] - bounds[2];
    z = bounds[5] - bounds[4];
    dimension = x*x + y*y + z*z;
    cutoffDimension = cutoff * dimension;

    // Get actor collection from the main viewer's renderer
    mainActors = ren->GetActors();    
    if (mainActors != NULL )
      {
      // add the little FOV box to NavigationWidget's actors
      this->NavigationWidget->GetRenderer()->AddViewProp( this->FOVBoxActor);
      mainActors->InitTraversal();
      mainActor = mainActors->GetNextActor();
      while (mainActor != NULL )
        {
        // get the bbox of this actor
        mainActor->GetBounds ( bounds );
        // check to see if it's big enough to include in the scene...
        x = bounds[1] - bounds[0];
        y = bounds[3] - bounds[2];
        z = bounds[5] - bounds[4];
        dimension = x*x + y*y + z*z;

        // add a copy of the actor to NavigationWidgets's renderer
        // only if it's big enough to count (don't bother with tiny
        // and don't bother with invisible stuff)
        int vis = mainActor->GetVisibility();
        if ( dimension > cutoffDimension && vis )
          {
          // ---new: create new actor, mapper, deep copy, add it.
          newMapper = vtkPolyDataMapper::New();
          newMapper->ShallowCopy (mainActor->GetMapper() );
          newMapper->SetInput ( vtkPolyData::SafeDownCast(mainActor->GetMapper()->GetInput()) );

          newActor = vtkActor::New();
          newActor->ShallowCopy (mainActor );
          newActor->SetMapper ( newMapper );
          newMapper->Delete();
          
          this->NavigationWidget->GetRenderer()->AddActor( newActor );
          newActor->Delete();
          }
        mainActor = mainActors->GetNextActor();
        }
      }
    }
#endif
}





//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewSetStereo ( )
{
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( )); 
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      int s = vn->GetStereoType();
      switch ( s )
        {
        case vtkMRMLViewNode::NoStereo:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOff ( );
          break;
        case vtkMRMLViewNode::RedBlue:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetStereoTypeToRedBlue();
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOn ( );
          break;
        case vtkMRMLViewNode::Anaglyph:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetStereoTypeToAnaglyph();
          //p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetAnaglyphColorSaturation(0.1);
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOn ( );
          break;
        case vtkMRMLViewNode::CrystalEyes:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetStereoTypeToCrystalEyes();
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOn ( );
          break;
        case vtkMRMLViewNode::Interlaced:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->SetStereoTypeToInterlaced();
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOn ( );
          break;
        default:
          p->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->StereoRenderOff ( );
          break;
        }
      p->GetViewerWidget()->GetMainViewer()->Render();
      }
    } 
}


void vtkSlicerViewControlGUI::DeviceCoordinatesToXYZ(vtkSlicerSliceGUI *sgui, int x, int y, int xyz[3] )
{
  vtkMRMLSliceNode *snode = sgui->GetSliceNode();
  
  vtkRenderWindowInteractor *iren
    = sgui->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
  int *windowSize = iren->GetRenderWindow()->GetSize();

  double tx = x / (double) windowSize[0];
  double ty = (windowSize[1] - y) / (double) windowSize[1];
  
  vtkRenderer *ren = iren->FindPokedRenderer(x, y);
  int *origin = ren->GetOrigin();
  
  xyz[0] = x - origin[0];
  xyz[1] = y - origin[1];
  xyz[2] = static_cast<int> ( floor(ty*snode->GetLayoutGridRows())
                              *snode->GetLayoutGridColumns()
                              + floor(tx*snode->GetLayoutGridColumns()) );
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SliceViewMagnify(int event, vtkSlicerInteractorStyle *istyle )
{
  int x, y;
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    
    if ( istyle == this->RedSliceEvents )
      {
      if ( appGUI->GetMainSliceGUI0()->GetLogic() != NULL )
        {
        if (appGUI->GetMainSliceGUI0()->GetLogic()->GetImageData() != NULL )
          {
          if (event == vtkCommand::EnterEvent )
            {
            // configure zoom
            x = this->RedSliceEvents->GetLastPos ()[0];
            y = this->RedSliceEvents->GetLastPos ()[1];

            // check that the event position is in the window
            int *windowSize =
              appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow()->GetSize();
            if ( x >= 0 && y >= 0 && x < windowSize[0] && y < windowSize[1] )
              {
              int xyz[3];
              this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI0(),
                                           x, y, xyz);

              this->SliceMagnifier->SetX ( xyz[0] );
              this->SliceMagnifier->SetY ( xyz[1] );
              this->SliceMagnifier->SetZ ( xyz[2] );
              }
            else
              {
              // event position is not in the window, punt
              this->SliceMagnifier->SetX( 0 );
              this->SliceMagnifier->SetY( 0 );
              this->SliceMagnifier->SetZ( 0 );
              }
            this->SliceMagnifier->SetInput ( appGUI->GetMainSliceGUI0()->GetLogic()->GetImageData());
            this->SliceMagnifierCursor->SetInput ( this->SliceMagnifier->GetOutput());
            this->SliceMagnifierMapper->SetInput ( this->SliceMagnifierCursor->GetOutput() );
            this->SliceMagnifierActor->SetMapper ( this->SliceMagnifierMapper );
            this->ZoomWidget->GetRenderer()->AddActor2D ( this->SliceMagnifierActor );
            this->RequestZoomRender();
            this->PackZoomWidget();
            }
          else if (event == vtkCommand::LeaveEvent )
            {
            this->PackNavigationWidget();          
            this->ZoomWidget->GetRenderer()->RemoveActor2D ( this->SliceMagnifierActor );
            this->SliceMagnifierMapper->SetInput ( NULL );
            this->SliceMagnifierCursor->SetInput ( NULL );
            this->SliceMagnifier->SetInput ( NULL );
            }
          else if ( event == vtkCommand::MouseMoveEvent )
            {
            // configure zoom
            x = this->RedSliceEvents->GetLastPos ()[0];
            y = this->RedSliceEvents->GetLastPos ()[1];

            int xyz[3];
            this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI0(),
                                           x, y, xyz);
            this->SliceMagnifier->SetX ( xyz[0] );
            this->SliceMagnifier->SetY ( xyz[1] );
            this->SliceMagnifier->SetZ ( xyz[2] );

            this->SliceMagnifier->SetInput ( appGUI->GetMainSliceGUI0()->GetLogic()->GetImageData());
            this->RequestZoomRender();
            }
          }
        }
      }
    
    else if ( istyle == this->YellowSliceEvents)
      {
      if ( appGUI->GetMainSliceGUI1()->GetLogic() != NULL )
        {
        if (appGUI->GetMainSliceGUI1()->GetLogic()->GetImageData() != NULL )
          {
          if (event == vtkCommand::EnterEvent )
            {
            // configure zoom
            x = this->YellowSliceEvents->GetLastPos ()[0];
            y = this->YellowSliceEvents->GetLastPos ()[1];

            // check that the event position is in the window
            int *windowSize =
              appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow()->GetSize();
            if ( x >= 0 && y >= 0 && x < windowSize[0] && y < windowSize[1] )
              {
              int xyz[3];
              this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI1(),
                                           x, y, xyz);
              this->SliceMagnifier->SetX ( xyz[0] );
              this->SliceMagnifier->SetY ( xyz[1] );
              this->SliceMagnifier->SetZ ( xyz[2] );
              }
            else
              {
              // event position is not in the window, punt
              this->SliceMagnifier->SetX( 0 );
              this->SliceMagnifier->SetY( 0 );
              this->SliceMagnifier->SetZ( 0 );
              }
            this->SliceMagnifier->SetInput ( appGUI->GetMainSliceGUI1()->GetLogic()->GetImageData());
            this->SliceMagnifierCursor->SetInput ( this->SliceMagnifier->GetOutput());
            this->SliceMagnifierMapper->SetInput ( this->SliceMagnifierCursor->GetOutput() );
            this->SliceMagnifierActor->SetMapper ( this->SliceMagnifierMapper );
            this->ZoomWidget->GetRenderer()->AddActor2D ( this->SliceMagnifierActor );
            this->RequestZoomRender();
            this->PackZoomWidget();
            }
          else if (event == vtkCommand::LeaveEvent )
            {
            this->PackNavigationWidget();
            this->ZoomWidget->GetRenderer()->RemoveActor2D ( this->SliceMagnifierActor );
            this->SliceMagnifierMapper->SetInput ( NULL );
            this->SliceMagnifierCursor->SetInput ( NULL );
            this->SliceMagnifier->SetInput ( NULL );
            }
          else if ( event == vtkCommand::MouseMoveEvent )
            {
            // configure zoom
            x = this->YellowSliceEvents->GetLastPos ()[0];
            y = this->YellowSliceEvents->GetLastPos ()[1];

            int xyz[3];
            this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI1(),
                                           x, y, xyz);
            this->SliceMagnifier->SetX ( xyz[0] );
            this->SliceMagnifier->SetY ( xyz[1] );
            this->SliceMagnifier->SetZ ( xyz[2] );

            this->SliceMagnifier->SetInput (appGUI->GetMainSliceGUI1()->GetLogic()->GetImageData());
            this->RequestZoomRender();
            }
          }
        }
      }
    else if ( istyle == this->GreenSliceEvents )
      {
      if ( appGUI->GetMainSliceGUI2()->GetLogic() != NULL )
        {
        if (appGUI->GetMainSliceGUI2()->GetLogic()->GetImageData() != NULL )
          {
          if (event == vtkCommand::EnterEvent )
            {
            // configure zoom
            x = this->GreenSliceEvents->GetLastPos ()[0];
            y = this->GreenSliceEvents->GetLastPos ()[1];

            // check that the event position is in the window
            int *windowSize =
              appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow()->GetSize();
            if ( x >= 0 && y >= 0 && x < windowSize[0] && y < windowSize[1] )
              {
              int xyz[3];
              this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI2(),
                                           x, y, xyz);
              this->SliceMagnifier->SetX ( xyz[0] );
              this->SliceMagnifier->SetY ( xyz[1] );
              this->SliceMagnifier->SetZ ( xyz[2] );
              }
            else
              {
              // event position is not in the window, punt
              this->SliceMagnifier->SetX( 0 );
              this->SliceMagnifier->SetY( 0 );
              this->SliceMagnifier->SetZ( 0 );
              }
            this->SliceMagnifier->SetInput ( appGUI->GetMainSliceGUI2()->GetLogic()->GetImageData());
            this->SliceMagnifierCursor->SetInput ( this->SliceMagnifier->GetOutput());
            this->SliceMagnifierMapper->SetInput ( this->SliceMagnifierCursor->GetOutput() );
            this->SliceMagnifierActor->SetMapper ( this->SliceMagnifierMapper );
            this->ZoomWidget->GetRenderer()->AddActor2D ( this->SliceMagnifierActor );
            this->RequestZoomRender();
            this->PackZoomWidget();
            }
          else if (event == vtkCommand::LeaveEvent )
            {
            this->PackNavigationWidget();
            this->ZoomWidget->GetRenderer()->RemoveActor2D ( this->SliceMagnifierActor );
            this->SliceMagnifierMapper->SetInput ( NULL );
            this->SliceMagnifierCursor->SetInput ( NULL );
            this->SliceMagnifier->SetInput ( NULL );
            }
          else if ( event == vtkCommand::MouseMoveEvent )
            {
            // configure zoom
            x = this->GreenSliceEvents->GetLastPos ()[0];
            y = this->GreenSliceEvents->GetLastPos ()[1];

            int xyz[3];
            this->DeviceCoordinatesToXYZ(appGUI->GetMainSliceGUI2(),
                                           x, y, xyz);
            this->SliceMagnifier->SetX ( xyz[0] );
            this->SliceMagnifier->SetY ( xyz[1] );
            this->SliceMagnifier->SetZ ( xyz[2] );

            this->SliceMagnifier->SetInput (appGUI->GetMainSliceGUI2()->GetLogic()->GetImageData());
            this->RequestZoomRender();
            }
          }
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewResetFocalPoint ( )
{
  double x_cen, y_cen, z_cen;
  vtkRenderer *ren;
  double bounds[6];

  int boxVisible;
  int axisLabelsVisible;
  
  // This method computes the visible scene bbox and
  // recenters the camera around the bbox centroid.
  
  vtkMRMLViewNode *vn = this->GetActiveView();
  if ( vn != NULL )
    {
    boxVisible = vn->GetBoxVisible();
    axisLabelsVisible = vn->GetAxisLabelsVisible();

    // if box is visible, turn its visibility temporarily off
    if ( boxVisible )
      {
      vn->SetBoxVisible ( 0 );
      }
    // if axis actors are visible, turn their visibility temporarily off.
    if ( axisLabelsVisible )
      {
      vn->SetAxisLabelsVisible ( 0 );
      }

    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    ren = p->GetViewerWidget()->GetMainViewer()->GetRenderer();
    ren->ComputeVisiblePropBounds( bounds );
    x_cen = (bounds[1] + bounds[0]) / 2.0;
    y_cen = (bounds[3] + bounds[2]) / 2.0;
    z_cen = (bounds[5] + bounds[4]) / 2.0;
    this->MainViewSetFocalPoint ( x_cen, y_cen, z_cen);

  // if box was visible, turn its visibility back on.
    if ( boxVisible )
      {
      vn->SetBoxVisible ( 1 );
      }
  // if axis actors were visible, turn their visibility back on.
    if ( axisLabelsVisible )
      {
      vn->SetAxisLabelsVisible ( 1 );
      }
    
  // Code used to recenter the view around the origin.
  //  this->MainViewSetFocalPoint ( 0.0, 0.0, 0.0);
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewSetFocalPoint ( double x, double y, double z)
{
   if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      double fov = vn->GetFieldOfView ( );
      double widefov = fov*3;
    
      vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();
        cam->SetFocalPoint( x, y, z );
        cam->ComputeViewPlaneNormal ( );
        cam->OrthogonalizeViewUp();
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->Render();
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewRock ( )
{
  if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      if ( vn->GetAnimationMode() == vtkMRMLViewNode::Rock )
        {
        this->RockView ( );
        //this->RequestNavigationRender();
        this->Script ( "update idletasks" );
        this->Script ( "after 5 \"%s MainViewRock \"",  this->GetTclName() );
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RockView ( )
{

  if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )
      {
      vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();

        double frac = (double) this->RockCount / (double) vn->GetRockLength();
        double az = 1.5 * cos ( 2.0 * 3.1415926 * (frac- floor(frac)));
        this->SetRockCount ( this->GetRockCount() + 1 );

        // Move the camera
        cam->Azimuth ( az );
        cam->OrthogonalizeViewUp ( );
        
        //Make the lighting follow the camera to avoid illumination changes
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->Render();
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewSpin ( )
{
  if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      if ( vn->GetAnimationMode() == vtkMRMLViewNode::Spin )
        {
        this->SpinView (vn->GetSpinDirection(), vn->GetSpinDegrees() );
        //this->RequestNavigationRender();
        this->Script ( "update idletasks" );
        this->Script ( "after 5 \"%s MainViewSpin \"",  this->GetTclName() );
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SpinView ( int dir, double degrees )
{
  
  if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();
        double ndegrees = -degrees;

        switch ( dir ) {
        case vtkMRMLViewNode::PitchUp:
          cam->Elevation ( degrees );
          break;
        case vtkMRMLViewNode::PitchDown:
          cam->Elevation ( ndegrees );
          break;
        case vtkMRMLViewNode::YawLeft:
          cam->Azimuth ( degrees );
          break;
        case vtkMRMLViewNode::YawRight:
          cam->Azimuth ( ndegrees );
          break;
        default:
          break;
        }
        cam->OrthogonalizeViewUp ( );

        //Make the lighting follow the camera to avoid illumination changes
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->Render();
        }
      }  
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewBackgroundColor ( double *color )
{
  if ( this->ApplicationGUI )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    // set background color
    p->GetViewerWidget()->GetMainViewer()->SetRendererBackgroundColor ( color );
    this->GetNavigationWidget()->SetRendererBackgroundColor ( color );
    // set axis label colors (prevent white on white)

    if ( color[0] == 1.0 && color[1] == 1.0 && color[2] == 1.0 )
      {
      p->GetViewerWidget()->ColorAxisLabelActors (0.0, 0.0, 0.0 );
      }
    else
      {
      p->GetViewerWidget()->ColorAxisLabelActors (1.0, 1.0, 1.0 );
      }
    p->GetViewerWidget()->UpdateFromMRML();
    }
}





//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewVisibility ( )
{
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      p->GetViewerWidget()->UpdateFromMRML();
      }
    }
}
  



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewSetProjection ( )
{

  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();
        // update the Rendering mode, then toggle
        // the button's icon appropriately 
        if ( vn->GetRenderMode() == vtkMRMLViewNode::Perspective )
          {
          cam->ParallelProjectionOff();
          this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetOrthoButtonIcon() );
          }
        else if ( vn->GetRenderMode() == vtkMRMLViewNode::Orthographic )
          {
          cam->ParallelProjectionOn();
          cam->SetParallelScale ( vn->GetFieldOfView() );
          this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetPerspectiveButtonIcon() );
          }
        p->GetViewerWidget()->GetMainViewer()->Render();
        }
      }
    }
}







//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewRotateAround ( int axis )
{
  double deg, negdeg;
  if ( this->ApplicationGUI)
    {
   vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
   vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      deg = vn->GetRotateDegrees ( );
      negdeg = -deg;

      vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();
        switch ( axis )
          {
          case vtkMRMLViewNode::PitchDown:
            cam->Elevation ( deg );
            break;
          case vtkMRMLViewNode::PitchUp:
            cam->Elevation ( negdeg );
            break;
          case vtkMRMLViewNode::RollLeft:
            cam->Roll ( deg );
            break;
          case vtkMRMLViewNode::RollRight:
            cam->Roll ( negdeg );
            break;
          case vtkMRMLViewNode::YawLeft:
            cam->Azimuth ( deg );
            break;
          case vtkMRMLViewNode::YawRight:
            cam->Azimuth ( negdeg );
            break;
          default:
            break;
          }
        cam->OrthogonalizeViewUp();
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->Render();
        this->RequestNavigationRender();
        }
      }
    }
}




  
//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewLookFrom ( const char *dir )
{

 double fov, *fp, widefov;

   if ( this->ApplicationGUI)
     {
     vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
     // TODO: get the active view, not the 0th view.
     vtkMRMLViewNode *vn = this->GetActiveView();
     if ( vn != NULL )  
       {
       fov = vn->GetFieldOfView ( );
       widefov = fov*3;
    
       vtkMRMLCameraNode *cn = this->GetActiveCamera();
      if ( cn != NULL )
        {
        vtkCamera *cam = cn->GetCamera();
        fp = cn->GetFocalPoint();

        if ( !strcmp (dir, "R"))
          {
          cam->SetPosition ( fp[0]+widefov, fp[1], fp[2] );
          cam->SetViewUp ( 0, 0, 1);
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        else if ( !strcmp (dir, "L"))
          {
          cam->SetPosition ( fp[0]-widefov, fp[1], fp[2]);
          cam->SetViewUp ( 0, 0, 1);        
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        else if ( !strcmp (dir, "S"))
          {
          cam->SetPosition ( fp[0], fp[1], fp[2]+widefov );
          cam->SetViewUp ( 0, 1, 0);
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        else if ( !strcmp (dir, "I"))
          {
          cam->SetPosition ( fp[0], fp[1], fp[2]-widefov );
          cam->SetViewUp ( 0, 1, 0);
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        else if ( !strcmp (dir, "A"))
          {
          cam->SetPosition (fp[0], fp[1]+widefov, fp[2] );
          cam->SetViewUp ( 0, 0, 1 );
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        else if ( !strcmp (dir, "P"))
          {
          cam->SetPosition (fp[0], fp[1]-widefov, fp[2] );
          cam->SetViewUp ( 0, 0, 1 );
          cam->SetFocalPoint ( 0.0, 0.0, 0.0);
          }
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->ResetCameraClippingRange ( );
        cam->ComputeViewPlaneNormal();
        cam->OrthogonalizeViewUp();
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->Render();      
        this->RequestNavigationRender();
        }
       }
     }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildCameraSelectMenu()
{
  
  this->SelectCameraButton->GetMenu( )->DeleteAllItems();
  this->SelectCameraButton->GetMenu()->AddRadioButton ("Save current camera" );
  this->SelectCameraButton->GetMenu()->SetItemStateToDisabled ( "Save current camera" );
  this->SelectCameraButton->GetMenu()->AddSeparator();
  this->SelectCameraButton->GetMenu()->AddSeparator();
  this->SelectCameraButton->GetMenu()->AddCommand ( "close" );
  // save current option will save current view under a
  // standard name like "View0...ViewN"; user can rename
  // this view elsewhere, in the ViewModule.
  // TODO: get existing MRMLViewNodes and add to menu

}
  
//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildVisibilityMenu ( )
{
  this->VisibilityButton->GetMenu()->DeleteAllItems ( );
  this->VisibilityButton->GetMenu()->AddCheckButton ("Fiducial points" );
  this->VisibilityButton->GetMenu()->AddCheckButton ("Fiducial labels" );
  this->VisibilityButton->GetMenu()->AddCheckButton ("3D cube" );
  this->VisibilityButton->GetMenu()->AddCheckButton ("3D axis labels" );
  this->VisibilityButton->GetMenu()->AddRadioButton ("Light blue background" );  
  this->VisibilityButton->GetMenu()->AddRadioButton ("Black background" );  
  this->VisibilityButton->GetMenu()->AddRadioButton ("White background" );  
  this->VisibilityButton->GetMenu()->AddSeparator();
  this->VisibilityButton->GetMenu()->AddCommand ( "close");
  this->VisibilityButton->GetMenu()->SetItemStateToDisabled ( "Fiducial points" );
  this->VisibilityButton->GetMenu()->SetItemStateToDisabled ( "Fiducial labels" );
//  this->VisibilityButton->GetMenu()->SelectItem ("Fiducial points" );
//  this->VisibilityButton->GetMenu()->SelectItem ("Fiducial labels" );
  this->VisibilityButton->GetMenu()->SelectItem ("3D cube" );
  this->VisibilityButton->GetMenu()->SelectItem ("3D axis labels" );
  this->VisibilityButton->GetMenu()->SelectItem ("Light blue background" );
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildStereoSelectMenu ( )
{
  this->StereoButton->GetMenu()->DeleteAllItems ( );
  this->StereoButton->GetMenu()->AddRadioButton ( "No stereo" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Red/Blue" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Anaglyph" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Interlaced" );  
  this->StereoButton->GetMenu()->AddRadioButton ( "CrystalEyes" );
  this->StereoButton->GetMenu()->AddSeparator();
  this->StereoButton->GetMenu()->AddCommand ( "close");
  this->StereoButton->GetMenu()->SelectItem ( "No stereo");
  
  int enableStereoCapableWindow = 0;
  // check whether stereo is enabled.
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplication *slicerApp = vtkSlicerApplication::SafeDownCast( this->GetApplicationGUI()->GetApplication() ); 
    if ( slicerApp->GetStereoEnabled() )
      {
      enableStereoCapableWindow = 1;
      }
    }
 if ( !enableStereoCapableWindow )
   {
     // let always enabled Red/Blue, Anaglyph, and Interlaced since those modes don't need the
     // special enabled stereo window
     this->StereoButton->GetMenu()->SetItemStateToDisabled ( "CrystalEyes" );
   }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildViewSelectMenu ( )
{
  
  this->SelectViewButton->GetMenu( )->DeleteAllItems();
  this->SelectViewButton->GetMenu()->AddRadioButton ("Save current view" );
  this->SelectViewButton->GetMenu()->SetItemStateToDisabled ( "Save current view" );
  this->SelectViewButton->GetMenu()->AddSeparator();
  this->SelectViewButton->GetMenu()->AddSeparator();
  this->SelectViewButton->GetMenu()->AddCommand ( "close" );
  // save current option will save current view under a
  // standard name like "View0...ViewN"; user can rename
  // this view elsewhere, in the ViewModule.
  // TODO: get existing MRMLViewNodes and add to menu

}






//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }

  this->ProcessingMRMLEvent = event;

  vtkDebugMacro("processing event " << event);
   
  if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    }
  else 
    {
    this->SceneClosing = false;
    }


  vtkMRMLViewNode *vnode = vtkMRMLViewNode::SafeDownCast ( caller );
  vtkMRMLSelectionNode *snode = vtkMRMLSelectionNode::SafeDownCast ( caller );
  
  // has a node been added or deleted?
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
       && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    this->UpdateFromMRML();
    this->UpdateNavigationWidgetViewActors ( );
    }

  // has a new camera or view has been selected?
  if ( snode != NULL )
    {
     this->UpdateFromMRML();
     this->UpdateNavigationWidgetViewActors ( );
    }    

  
  // has view been manipulated?
  if ( vnode != NULL )
    {
    if (event == vtkMRMLViewNode::AnimationModeEvent )
      {
      // handle the mode change 
      if ( vnode->GetAnimationMode() == vtkMRMLViewNode::Spin )
        {
        if ( this->RockButton->GetSelectedState() == 1 )
          {
          this->RockButton->Deselect();
          }
        this->MainViewSpin (  );
        }
      // handle the mode change
      else if ( vnode->GetAnimationMode() == vtkMRMLViewNode::Rock )
        {
        if ( this->SpinButton->GetSelectedState() == 1 )
          {
          this->SpinButton->Deselect();
          }
        this->SetRockCount ( vnode->GetRockCount ( ) );
        this->MainViewRock ( );
        }
      else if ( vnode->GetAnimationMode() == vtkMRMLViewNode::Off )
        {
        if ( this->RockButton->GetSelectedState() == 1 )
          {
          this->RockButton->Deselect();
          }
        if ( this->SpinButton->GetSelectedState() == 1 )
          {
          this->SpinButton->Deselect();
          }
        if ( this->SpinButton->GetSelectedState() == 0 &&
             this->RockButton->GetSelectedState() == 0)
          {
          this->RequestNavigationRender();
          }
        }
      }


    // a stereo event?
    if ( event == vtkMRMLViewNode::StereoModeEvent )
      {
      // does the menu match the node? if not, update the menu
      this->MainViewSetStereo ( );
      }
    // background color change?
    else if ( event == vtkMRMLViewNode::BackgroundColorEvent )
      {
      // does the background color match the node? if not, update the menu
      this->MainViewBackgroundColor ( vnode->GetBackgroundColor() );
      }
    // visibility of something changed
    else if ( event == vtkMRMLViewNode::VisibilityEvent )
      {
      // axis labels, fiducial points, fiducial labels or 3Dcube?
      // does the menu match the node? if not update the menu
      this->MainViewVisibility ( );
      }
    // render mode changed
    else if ( event == vtkMRMLViewNode::RenderModeEvent )
      {
      // does the button match the state? if not, update the button.
      this->MainViewSetProjection();
      }
    // whatever else...
    }
  // handle whatever other change is made to the view.
  else
    {
    }
  this->ProcessingMRMLEvent = 0;
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlACallback ( )
{
   if ( this->ApplicationGUI)
     {
     vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
     vtkMRMLViewNode *vn = this->GetActiveView();
     if ( vn != NULL )  
       {
       if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
         {
         this->MainViewRotateAround ( vtkMRMLViewNode::RollLeft );
         }
       else
         {
         this->MainViewLookFrom ( "A" );
         }
       }
     }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlPCallback ( )
{
   if ( this->ApplicationGUI)
     {
     vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
     vtkMRMLViewNode *vn = this->GetActiveView();
     if ( vn != NULL )  
       {
       if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
         {
         this->MainViewRotateAround ( vtkMRMLViewNode::RollRight );
         }
       else
         {
         this->MainViewLookFrom ("P");
         }
       }
     }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlSCallback ( )
{
   if ( this->ApplicationGUI)
     {
     vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
     vtkMRMLViewNode *vn = this->GetActiveView();
     if ( vn != NULL )  
       {
       if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
         {
         this->MainViewRotateAround ( vtkMRMLViewNode::YawLeft );
         }
       else
         {
         this->MainViewLookFrom ("S");
         }
       }
     }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlICallback ( )
{
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->MainViewRotateAround ( vtkMRMLViewNode::YawRight );
        }
      else
        {
        this->MainViewLookFrom ("I");
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlRCallback ( )
{
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->MainViewRotateAround ( vtkMRMLViewNode::PitchUp );
        }
      else
        {
        this->MainViewLookFrom ("R");
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ViewControlLCallback ( )
{
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {
      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->MainViewRotateAround (vtkMRMLViewNode::PitchDown);
        }
      else
        {
        this->MainViewLookFrom ("L");
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisACallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if (vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
        this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
        }
      else
        {
        this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisACallback ( ) {
  this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconLO() );
  this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisPCallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
        this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
        }
      else
        {
        this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisPCallback ( ) {
  this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconLO() );
  this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisRCallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
        this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
        }
      else
        {
        this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisRCallback ( ) {
  this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconLO() );
  this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisLCallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
        this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
        }
      else
        {
        this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisLCallback ( ) {
  this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconLO() );
  this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisSCallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
        this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
        }
      else
        {
        this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisSCallback ( ) {
  this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconLO() );
  this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisICallback ( ) {
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLViewNode *vn = this->GetActiveView();
    if ( vn != NULL )  
      {

      if ( vn->GetViewAxisMode() == vtkMRMLViewNode::RotateAround )
        {
        this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
        this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
        }
      else
        {
        this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisICallback ( ) {
  this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconLO() );
  this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconLO() );
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::PackNavigationWidget ( )
{
  if ( this->ZoomWidget != NULL )
    {
    this->Script ("pack forget %s ", this->ZoomWidget->GetWidgetName() );
    }
  if ( this->NavigationWidget != NULL )
    {
    this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->NavigationWidget->GetWidgetName ( ) );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::PackZoomWidget ( )
{
  if ( this->NavigationWidget != NULL )
    {
    this->Script ("pack forget %s ", this->NavigationWidget->GetWidgetName() );
    }
  if ( this->ZoomWidget != NULL )
    {
    this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->ZoomWidget->GetWidgetName ( ) );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ReconfigureGUIFonts ( )
{
  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );  
  // populate the application's 3DView control GUI panel
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      this->ZoomEntry->GetLabel()->SetFont ( app->GetSlicerTheme()->GetApplicationFont0() );
      this->ZoomEntry->GetWidget()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1 () );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildGUI ( vtkKWFrame *appF )
{


  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );  
  // populate the application's 3DView control GUI panel
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      vtkSlicerGUILayout *layout = app->GetMainLayout ( );

      this->SlicerViewControlIcons = vtkSlicerViewControlIcons::New ( );
      this->SpinButton = vtkKWCheckButton::New ( );
      this->RockButton = vtkKWCheckButton::New ( );
      this->OrthoButton = vtkKWPushButton::New ( );

      this->CenterButton = vtkKWPushButton::New ( );
      this->StereoButton = vtkKWMenuButton::New ( );
      this->SelectViewButton = vtkKWMenuButton::New ( );
      this->SelectCameraButton = vtkKWMenuButton::New ( );
      this->LookFromButton = vtkKWRadioButton::New ( );
      this->RotateAroundButton = vtkKWRadioButton::New ( );
      this->ZoomEntry = vtkKWEntryWithLabel::New ( );
      this->VisibilityButton = vtkKWMenuButton::New ( );

      //--- ui for the ViewControlFrame
      this->ViewAxisAIconButton = vtkKWLabel::New ( );
      this->ViewAxisPIconButton = vtkKWLabel::New ( );
      this->ViewAxisRIconButton = vtkKWLabel::New ( );
      this->ViewAxisLIconButton = vtkKWLabel::New ( );
      this->ViewAxisSIconButton = vtkKWLabel::New ( );
      this->ViewAxisIIconButton = vtkKWLabel::New ( );
      this->ViewAxisCenterIconButton = vtkKWLabel::New ( );
      this->ViewAxisTopCornerIconButton = vtkKWLabel::New ( );
      this->ViewAxisBottomCornerIconButton = vtkKWLabel::New ( );

      // temporary thing until navzoom window is built.
      this->NavigationWidget = vtkKWRenderWidget::New( );
      this->ZoomWidget = vtkKWRenderWidget::New ( );
      this->NavigationZoomFrame = vtkKWFrame::New ( );

      
      // create and pack sub-frames for the ViewControl GUI
      vtkKWFrame *f0 = vtkKWFrame::New ( );
      vtkKWFrame *f1 = vtkKWFrame::New ( );
      vtkKWFrame *f2 = vtkKWFrame::New ( );
      vtkKWFrame *f3 = vtkKWFrame::New ( );
      vtkKWFrame *f4 = vtkKWFrame::New ( );
      vtkKWFrame *f6 = vtkKWFrame::New ( );
      f0->SetParent ( appF);
      f0->Create ( );
      f1->SetParent (f0);
      f1->Create();
      f2->SetParent ( f0);
      f2->Create();
      f3->SetParent ( f0);
      f3->Create();
      f4->SetParent ( f0);
      f4->Create();
      this->NavigationZoomFrame->SetParent ( f0);
      this->NavigationZoomFrame->Create();
      f6->SetParent ( f0);
      f6->Create();
      this->Script ( "pack %s -side left -anchor nw -padx 2 -pady 2 -expand n", f0->GetWidgetName ( ) );      
      this->Script ( "grid %s -row 0 -column 0 -sticky w -padx 0 -pady 0", f1->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 0 -columnspan 2 -sticky ew -padx 0 -pady 0", f2->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 0 -pady 0", f3->GetWidgetName ( ) );
//      this->Script ( "grid %s -row 1 -column 1  -sticky w -padx 0 -pady 0", f4->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 2  -rowspan 2 -sticky news -padx 0 -pady 0", this->NavigationZoomFrame->GetWidgetName ( ) );
//      this->Script ( "grid %s -row 1 -column 2  -sticky w -padx 0 -pady 0", f6->GetWidgetName ( ) );

      // create and pack the look from and rotate around checkbuttons

      // create and pack rollover labels for rotate around and look from camera control
      this->ViewAxisAIconButton->SetParent ( f1);
      this->ViewAxisAIconButton->Create ( );
      this->ViewAxisAIconButton->SetBorderWidth ( 0 );
      this->ViewAxisAIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisAIconLO() );
      this->ViewAxisAIconButton->SetBalloonHelpString ("Rotate camera in 3D view around A-P axis or Look from A toward center.");
      this->ViewAxisPIconButton->SetParent ( f1 );
      this->ViewAxisPIconButton->Create ( );
      this->ViewAxisPIconButton->SetBorderWidth ( 0 );
      this->ViewAxisPIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisPIconLO() );
      this->ViewAxisPIconButton->SetBalloonHelpString ("Rotate camera in 3D view around A-P axis or Look from P toward center.");
      this->ViewAxisRIconButton->SetParent ( f1 );
      this->ViewAxisRIconButton->Create ( );
      this->ViewAxisRIconButton->SetBorderWidth ( 0 );
      this->ViewAxisRIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisRIconLO() );
      this->ViewAxisRIconButton->SetBalloonHelpString ("Rotate camera in 3D view around R-L axis or Loook from R toward center.");
      this->ViewAxisLIconButton->SetParent ( f1 );
      this->ViewAxisLIconButton->Create ( );
      this->ViewAxisLIconButton->SetBorderWidth ( 0 );
      this->ViewAxisLIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisLIconLO() );
      this->ViewAxisLIconButton->SetBalloonHelpString ("Rotate camera in 3D view around R-L axis or Look from L toward center.");
      this->ViewAxisSIconButton->SetParent ( f1 );
      this->ViewAxisSIconButton->Create ( );
      this->ViewAxisSIconButton->SetBorderWidth ( 0 );
      this->ViewAxisSIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisSIconLO() );
      this->ViewAxisSIconButton->SetBalloonHelpString ("Rotate camera in 3D view around S-I axis or Look from S toward center.");
      this->ViewAxisIIconButton->SetParent ( f1 );
      this->ViewAxisIIconButton->Create ( );
      this->ViewAxisIIconButton->SetBorderWidth ( 0 );
      this->ViewAxisIIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisIIconLO() );
      this->ViewAxisIIconButton->SetBalloonHelpString ("Rotate camera in 3D view around S-I axis or Look from I toward center.");
      this->ViewAxisCenterIconButton->SetParent ( f1 );
      this->ViewAxisCenterIconButton->Create ( );
      this->ViewAxisCenterIconButton->SetBorderWidth ( 0 );
      this->ViewAxisCenterIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisCenterIcon() );
      this->ViewAxisTopCornerIconButton->SetParent ( f1 );
      this->ViewAxisTopCornerIconButton->Create ( );
      this->ViewAxisTopCornerIconButton->SetBorderWidth ( 0 );
      this->ViewAxisTopCornerIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisTopCornerIcon() );
      this->ViewAxisBottomCornerIconButton->SetParent ( f1 );
      this->ViewAxisBottomCornerIconButton->Create ( );
      this->ViewAxisBottomCornerIconButton->SetBorderWidth ( 0 );
      this->ViewAxisBottomCornerIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetViewAxisBottomCornerIcon() );
      this->Script ("grid %s -row 1 -column 0 -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisRIconButton->GetWidgetName ( ) );
      this->Script ("grid %s -row 1 -column 1  -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisCenterIconButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 2  -sticky w -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisLIconButton->GetWidgetName ( ) );
      this->Script ("grid %s -row 0 -column 0 -sticky sw -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisPIconButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 1  -sticky sw -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisSIconButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 2 -sticky sw -padx 0 -pady 0 -ipadx 0 -ipady 0",this->ViewAxisTopCornerIconButton->GetWidgetName ( ) );
      this->Script ("grid %s -row 2 -column 0  -sticky nw -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisBottomCornerIconButton->GetWidgetName ( ));
      this->Script ("grid %s -row 2 -column 1 -sticky nw -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisIIconButton->GetWidgetName ( ) );
      this->Script ("grid %s -row 2 -column 2  -sticky nw -padx 0 -pady 0 -ipadx 0 -ipady 0", this->ViewAxisAIconButton->GetWidgetName ( ) );
      this->MakeViewControlRolloverBehavior ( );

      // create and pack other view control icons
      //--- Radiobutton to select rotate view around axis
      this->RotateAroundButton->SetParent ( f3 );
      this->RotateAroundButton->Create ( );
      this->RotateAroundButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundOffButtonIcon() );
      this->RotateAroundButton->SetSelectImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundOnButtonIcon() );
      this->RotateAroundButton->SetBalloonHelpString ( "Set the 3D view control mode to 'rotate around' selected axis ");
      this->RotateAroundButton->SetValueAsInt ( 101 );
      //--- Radiobutton to select view look from direction
      this->LookFromButton->SetParent ( f3 );
      this->LookFromButton->Create ( );
      this->LookFromButton->SetSelectImageToIcon ( this->SlicerViewControlIcons->GetLookFromOnButtonIcon() );
      this->LookFromButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromOffButtonIcon() );
      this->LookFromButton->SetBalloonHelpString ( "Set the 3D view control mode to 'look from' selected direction");
      this->LookFromButton->SetValueAsInt ( 202 );
      this->LookFromButton->SetVariableName ( this->RotateAroundButton->GetVariableName( ) );
      this->LookFromButton->SetSelectedState(1);
      //--- Push button to toggle between perspective and ortho rendering
      this->OrthoButton->SetParent ( f3);
      this->OrthoButton->Create ( );
      this->OrthoButton->SetReliefToFlat ( );
      this->OrthoButton->SetBorderWidth ( 0 );
      this->OrthoButton->SetOverReliefToNone ( );
      this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetOrthoButtonIcon() );      
      this->OrthoButton->SetBalloonHelpString ( "Toggle between orthographic and perspective rendering in the 3D view.");
      //--- Pusbbutton to center the rendered vie on the scene center
      this->CenterButton->SetParent ( f3);
      this->CenterButton->Create ( );
      this->CenterButton->SetReliefToFlat ( );
      this->CenterButton->SetBorderWidth ( 0 );
      this->CenterButton->SetOverReliefToNone ( );
      this->CenterButton->SetImageToIcon ( this->SlicerViewControlIcons->GetCenterButtonIcon() );      
      this->CenterButton->SetBalloonHelpString ( "Center the 3D view on the scene.");
      //--- Menubutton to show stereo options
      this->StereoButton->SetParent ( f3);
      this->StereoButton->Create ( );
      this->StereoButton->SetReliefToFlat ( );
      this->StereoButton->SetBorderWidth ( 0 );
      this->StereoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetStereoButtonIcon() );      
      this->StereoButton->IndicatorVisibilityOff ( );
      this->StereoButton->SetBalloonHelpString ( "Select among stereo viewing options (3DSlicer must be started with stereo enabled to use these features).");
      //--- Menubutton to capture or select among saved 3D views.
      this->SelectViewButton->SetParent ( f3);
      this->SelectViewButton->Create ( );
      this->SelectViewButton->SetReliefToFlat ( );
      this->SelectViewButton->SetBorderWidth ( 0 );
      this->SelectViewButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSelectViewButtonIcon() );
      this->SelectViewButton->IndicatorVisibilityOff ( );
      this->SelectViewButton->SetBalloonHelpString ( "Save current or select among already saved 3D views.");
      //--- Menubutton to capture or select among saved 3D views.
      this->SelectCameraButton->SetParent ( f3);
      this->SelectCameraButton->Create ( );
      this->SelectCameraButton->SetReliefToFlat ( );
      this->SelectCameraButton->SetBorderWidth ( 0 );
      this->SelectCameraButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSelectCameraButtonIcon() );
      this->SelectCameraButton->IndicatorVisibilityOff ( );
      this->SelectCameraButton->SetBalloonHelpString ( "Save current or select among already saved cameras.");

      //--- Checkbutton to spin the view
      this->SpinButton->SetParent ( f3 );
      this->SpinButton->Create ( );
      this->SpinButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSpinOffButtonIcon() );      
      this->SpinButton->SetSelectImageToIcon ( this->SlicerViewControlIcons->GetSpinOnButtonIcon() );      
      this->SpinButton->Deselect();
      this->SpinButton->SetBalloonHelpString ( "Spin the 3D view.");
      //--- CheckButton to rotate the view
      this->RockButton->SetParent ( f3 );
      this->RockButton->Create ( );
      this->RockButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRockOffButtonIcon() );      
      this->RockButton->SetSelectImageToIcon ( this->SlicerViewControlIcons->GetRockOnButtonIcon() );      
      this->RockButton->SetBalloonHelpString ( "Rock the 3D view.");
      this->RockButton->Deselect();
      //--- Menubutton to turn on/off axes, cube, outlines, annotations in 3D view.
      this->VisibilityButton->SetParent (f3);
      this->VisibilityButton->Create ( );
      this->VisibilityButton->IndicatorVisibilityOff ( );
      this->VisibilityButton->SetBorderWidth ( 0 );
      this->VisibilityButton->SetImageToIcon ( this->SlicerViewControlIcons->GetVisibilityButtonIcon ( ) );
      this->VisibilityButton->SetBalloonHelpString ("Toggle visibility of elements in the 3D view." );

      // TODO: why did i have to padx by 4 to get the grid to line up?
      // this works on  win32; will it break on other platforms?
      this->Script ("grid %s -row 0 -column 0 -sticky w -padx 6 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 0 -sticky w -padx 6 -pady 0 -ipadx 0 -ipady 0", this->LookFromButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 1 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->OrthoButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 1 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->StereoButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 2 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->VisibilityButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 1 -column 2 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->SelectCameraButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 3 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->CenterButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 3 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->SelectViewButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 0 -column 4 -sticky e -padx 4 -pady 0 -ipadx 0 -ipady 0", this->SpinButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 4 -sticky e -padx 4 -pady 0 -ipadx 0 -ipady 0", this->RockButton->GetWidgetName ( ));

      this->ZoomEntry->SetParent ( f2 );
      this->ZoomEntry->Create ( );
      this->ZoomEntry->GetLabel()->SetImageToIcon(this->SlicerViewControlIcons->GetPercentZoomIcon() );
      this->ZoomEntry->GetLabel()->SetBalloonHelpString("Set the relative zoom (as percent)");
      this->ZoomEntry->GetWidget()->SetBalloonHelpString("Set the relative zoom (as percent)");
      this->ZoomEntry->SetLabelPositionToLeft();
      this->ZoomEntry->GetWidget()->SetWidth (6);
      this->ZoomEntry->GetWidget()->SetValueAsDouble (100);
      this->ZoomEntry->GetWidget()->SetCommandTrigger (vtkKWEntry::TriggerOnReturnKey );
      this->Script ( "pack %s -side right -anchor c -padx 4 -pady 2 -expand n", this->ZoomEntry->GetWidgetName ( ) );
      
      //--- set fonts for all widgets with text.
      this->ReconfigureGUIFonts();
      
      //--- create the nav/zoom widgets
      this->ZoomWidget->SetParent ( this->NavigationZoomFrame );
      this->ZoomWidget->Create ( );
      this->ZoomWidget->SetWidth ( this->NavigationZoomWidgetWid );
      this->ZoomWidget->SetHeight ( this->NavigationZoomWidgetHit );
      this->ZoomWidget->SetRendererBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->Black );
      this->ZoomWidget->GetRenderWindow()->AddRenderer(this->ZoomWidget->GetRenderer() );
      
      this->SliceMagnifier = vtkSlicerImageCloseUp2D::New();
      this->SliceMagnifier->SetHalfWidth (7 );
      this->SliceMagnifier->SetHalfHeight ( 4 );
      this->SliceMagnifier->SetMagnification ( 10 );
      this->SliceMagnifier->SetInput ( NULL );
      
      this->SliceMagnifierCursor = vtkSlicerImageCrossHair2D::New();
      this->SliceMagnifierCursor->SetCursor (75, 40);
      this->SliceMagnifierCursor->BullsEyeOn();
      this->SliceMagnifierCursor->ShowCursorOn();
      this->SliceMagnifierCursor->IntersectCrossOff();
      this->SliceMagnifierCursor->SetCursorColor ( 1.0, 0.75, 0.0);
      this->SliceMagnifierCursor->SetNumHashes ( 0 );
      this->SliceMagnifierCursor->SetHashLength ( 6 );
      this->SliceMagnifierCursor->SetHashGap ( 10 );
      this->SliceMagnifierCursor->SetMagnification ( 1.0 );      
      this->SliceMagnifierCursor->SetInput ( NULL );

      this->SliceMagnifierMapper = vtkImageMapper::New();
      this->SliceMagnifierMapper->SetColorWindow(255);
      this->SliceMagnifierMapper->SetColorLevel (127.5);
      this->SliceMagnifierMapper->SetInput ( NULL);
      this->SliceMagnifierActor = vtkActor2D::New();
      this->SliceMagnifierActor->SetMapper ( NULL );

      this->CreateFieldOfViewBoxActor ( );

      this->NavigationWidget->SetParent (this->NavigationZoomFrame);
      this->NavigationWidget->Create();
      this->NavigationWidget->SetWidth ( this->NavigationZoomWidgetWid );
      this->NavigationWidget->SetHeight ( this->NavigationZoomWidgetHit );
      this->NavigationWidget->SetRendererBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->NavigationWidget->GetRenderWindow()->AddRenderer(this->NavigationWidget->GetRenderer() );
      this->NavigationWidget->GetRenderWindow()->DoubleBufferOn();
      this->NavigationWidget->GetRenderer()->GetRenderWindow()->GetInteractor()->Disable();

      // TODO: Why does the display flash the first time we 'pack forget' and then 'pack'?
      // pack zoom widget temporarily, then swap in navigation widget.
      this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->ZoomWidget->GetWidgetName ( ) );      
      this->PackNavigationWidget ( );
      
      // populate menus
      this->BuildViewSelectMenu();
      this->BuildCameraSelectMenu();
      this->BuildStereoSelectMenu ( );
      this->BuildVisibilityMenu ( );

      //Create the Enable/Disable BUtton for the navigation widget but don't pack it
      this->EnableDisableNavButton=vtkKWCheckButton::New();
      this->EnableDisableNavButton->SetParent(f0);
      this->EnableDisableNavButton->Create();
      this->EnableDisableNavButton->SelectedStateOn();
    
      // clean up
      f0->Delete ( );
      f1->Delete ( );
      f2->Delete ( );
      f3->Delete ( );
      f4->Delete ( );
      f6->Delete ( );
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddSliceEventObservers()
{
  
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->RedSliceEvents != NULL )
      {
      this->RedSliceEvents->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->YellowSliceEvents != NULL )
      {
      this->YellowSliceEvents->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->GreenSliceEvents != NULL )
      {
      this->GreenSliceEvents->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveSliceEventObservers()
{
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->RedSliceEvents != NULL )
      {
      this->RedSliceEvents->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->YellowSliceEvents != NULL )
      {
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->GreenSliceEvents != NULL )
      {
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    }
}



//---------------------------------------------------------------------------
vtkMRMLViewNode *vtkSlicerViewControlGUI::GetActiveView ( )
{

  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    // TODO: make sure we get active view here, not 0th view, when active views are available.
    vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(p->GetMRMLScene()->GetNthNodeByClass ( 0, "vtkMRMLViewNode"));
    if ( this->ViewNode != vn )
      {
      this->UpdateFromMRML ();
      }
    }
    return ( this->ViewNode );
}



//---------------------------------------------------------------------------
vtkMRMLCameraNode *vtkSlicerViewControlGUI::GetActiveCamera()
{
  if ( this->ApplicationGUI)
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    // TODO: make sure we get active view here, not 0th view, when active cameras are available.
    vtkMRMLCameraNode *cn = vtkMRMLCameraNode::SafeDownCast(p->GetMRMLScene()->GetNthNodeByClass ( 0, "vtkMRMLCameraNode"));
    if ( cn != NULL )  
      {
      return (cn );
      }
    }
  return ( NULL );
}






//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateMainViewerInteractorStyles ( )
{
  // get all views from the scene
  // and observe active view.
  if (this->SceneClosing)
    {
    return;
    }

  // Find current MainViewer; if there is none, do nothing.
  if (  this->GetApplicationGUI()->GetViewerWidget() == NULL )
    {
    return;
    }
  if (  this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer() == NULL )
    {
    return;
    }

  // If the interactor and these references are out of sync...
  if ( ( this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->
         GetRenderWindowInteractor()->GetInteractorStyle() != this->MainViewerEvents ) )
    {
    // tear down pointer assignment
    this->RemoveMainViewerEventObservers();
    this->SetMainViewerEvents(NULL );
    // reassign pointer
    this->SetMainViewerEvents( vtkSlicerViewerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetViewerWidget()->
                                                                 GetMainViewer()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    // add observers on events
    this->AddMainViewerEventObservers();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::CheckAbort(void)
{
    int pending=this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow()->GetEventPending();
    int pendingGUI=vtkKWTkUtilities::CheckForPendingInteractionEvents(this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindow());
    if(pending!=0)//||pendingGUI!=0)
    {
        this->NavigationWidget->GetRenderWindow()->SetAbortRender(1);
        return;
    }
}


