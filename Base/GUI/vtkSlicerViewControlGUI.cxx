#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerViewControlGUI.h"
#include "vtkSlicerWindow.h"

#include "vtkKWFrame.h"
#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkSlicerViewControlIcons.h"

#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewControlGUI );
vtkCxxRevisionMacro ( vtkSlicerViewControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::vtkSlicerViewControlGUI ( )
{
  
  // Description:
  // parameters of automatic spin
  this->Spin = 0;
  this->Rock = 0;
  this->SpinDegrees = 2.0;
  this->SpinDirection = 3;
  this->SpinMs = 5;

  // Description:
  // parameters of automatic rock
  this->RockLength = 200;
  this->RockCount = 0;

  // Description:
  // stereo viewing mode and type
  this->Stereo = 0;
  this->StereoType = 0;
  this->ViewAxisMode = 0;
  this->RenderMode = 0;

  this->SlicerViewControlIcons = vtkSlicerViewControlIcons::New ( );
  this->SpinButton = vtkKWCheckButton::New ( );
  this->RockButton = vtkKWCheckButton::New ( );
  this->OrthoButton = vtkKWPushButton::New ( );

  this->CenterButton = vtkKWPushButton::New ( );
  this->StereoButton = vtkKWMenuButton::New ( );
  this->SelectButton = vtkKWMenuButton::New ( );
  this->LookFromButton = vtkKWRadioButton::New ( );
  this->RotateAroundButton = vtkKWRadioButton::New ( );
  this->FOVEntry = vtkKWEntryWithLabel::New ( );
  this->ZoomEntry = vtkKWEntryWithLabel::New ( );

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

  this->NavZoomInIconButton = vtkKWPushButton::New ( );
  this->NavZoomOutIconButton = vtkKWPushButton::New ( );
  this->NavZoomScale = vtkKWScale::New ( );

  // temporary thing until navzoom window is built.
  this->NavZoomLabel = vtkKWLabel::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::~vtkSlicerViewControlGUI ( )
{

  // parameters of automatic spin
  this->Spin = 0;
  this->Rock = 0;
  this->SpinDegrees = 0.0;
  this->SpinDirection = 0;
  this->SpinMs = 0;

  // parameters of automatic rock
  this->RockLength = 0;
  this->RockCount = 0;

  // stereo off
  this->Stereo = 0;
  this->StereoType = 0;

  this->ViewAxisMode = 0;
  this->RenderMode = 0;

  if ( this->NavZoomLabel )
    {
    this->NavZoomLabel->Delete ( );
    this->NavZoomLabel = NULL;
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
  if ( this->SelectButton ) 
    {
    this->SelectButton->SetParent ( NULL );
    this->SelectButton->Delete();
    this->SelectButton = NULL;
    }
  if ( this->StereoButton ) 
    {
    this->StereoButton->SetParent ( NULL );
    this->StereoButton->Delete();
    this->StereoButton = NULL;
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
  if ( this->FOVEntry ) 
    {
    this->FOVEntry->SetParent ( NULL );
    this->FOVEntry->Delete();
    this->FOVEntry= NULL;
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
  if ( this->NavZoomInIconButton ) 
    {
    this->NavZoomInIconButton->SetParent ( NULL );
    this->NavZoomInIconButton->Delete ( );
    this->NavZoomInIconButton = NULL;
    }
  if ( this->NavZoomOutIconButton ) 
    {
    this->NavZoomOutIconButton->SetParent ( NULL );
    this->NavZoomOutIconButton->Delete ( );
    this->NavZoomOutIconButton = NULL;
    }
  if ( this->NavZoomScale ) 
    {
    this->NavZoomScale->SetParent ( NULL );
    this->NavZoomScale->Delete ( );
    this->NavZoomScale = NULL;
    }

  this->SetApplicationGUI ( NULL );
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MakeViewControlRolloverBehavior ( )
{

  //--- configure and bind for rollover interaction
  this->ViewAxisAIconButton->SetBorderWidth (0);
  this->ViewAxisAIconButton->SetBinding ( "<Enter>",  this, "EnterViewAxisACallback");
  this->ViewAxisAIconButton->SetBinding ( "<Leave>",  this, "LeaveViewAxisACallback");
  this->Script ( "%s ListMethods", this->GetTclName() );

  this->ViewAxisPIconButton->SetBorderWidth (0);
  this->ViewAxisPIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisPCallback");
  this->ViewAxisPIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisPCallback");

  this->ViewAxisRIconButton->SetBorderWidth (0);
  this->ViewAxisRIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisRCallback");
  this->ViewAxisRIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisRCallback");

  this->ViewAxisLIconButton->SetBorderWidth (0);
  this->ViewAxisLIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisLCallback");
  this->ViewAxisLIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisLCallback");

  this->ViewAxisSIconButton->SetBorderWidth (0);
  this->ViewAxisSIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisSCallback");
  this->ViewAxisSIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisSCallback");
  
  this->ViewAxisIIconButton->SetBorderWidth (0);
  this->ViewAxisIIconButton->SetBinding ( "<Enter>", this, "EnterViewAxisICallback");
  this->ViewAxisIIconButton->SetBinding ( "<Leave>", this, "LeaveViewAxisICallback");
  
  this->ViewAxisCenterIconButton->SetBorderWidth (0);
  this->ViewAxisTopCornerIconButton->SetBorderWidth (0);
  this->ViewAxisBottomCornerIconButton->SetBorderWidth (0);
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  //TODO: need to print everything
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "SlicerViewControlGUI: " << this->GetClassName ( ) << "\n";

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
    this->SelectButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FOVEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
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
    this->SelectButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FOVEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ZoomEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  // Right now this class contains state variables that will be moved
  // to a vtkMRMLViewNode in the next iteration.

  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
    if ( app != NULL )
      {
      vtkKWCheckButton *b = vtkKWCheckButton::SafeDownCast ( caller );
      vtkKWPushButton *p = vtkKWPushButton::SafeDownCast ( caller );
      vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
      vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
      // toggle the Ortho/Perspective rendering state
      if ( e == this->FOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        }
      if ( e == this->ZoomEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        }
      if ( m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        }
      if ( m == this->SelectButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        }
      if ( (p == this->CenterButton) && (event == vtkKWPushButton::InvokedEvent ) )
        {
        }
      if ( (p == this->OrthoButton) && (event == vtkKWPushButton::InvokedEvent ) )
        {
        if ( this->RenderMode == vtkSlicerViewControlGUI::Orthographic )
          {
          this->RenderMode = vtkSlicerViewControlGUI::Perspective;
          }
        else if ( this->RenderMode == vtkSlicerViewControlGUI::Perspective )
          {
          this->RenderMode = vtkSlicerViewControlGUI::Orthographic;
          }
        // then toggle the button's icon appropriately (handle this in process logic events later
        if ( this->RenderMode == vtkSlicerViewControlGUI::Perspective )
          {
          this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetOrthoButtonIcon() );
          }
        else if ( this->RenderMode == vtkSlicerViewControlGUI::Orthographic )
          {
          this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetPerspectiveButtonIcon() );
          }
        }

      //--- turn View Spin and Rocking on and off
      if ( (b == this->SpinButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent) )
        {
        // toggle the Spin (and turn off Rock if necessary)
        if ( this->Spin == 1 )
          {
          this->Spin = 0;
          }
        else if ( this->Spin == 0 )
          {
          this->Spin = 1;
          }
        // handle the interaction 
        if ( this->Spin == 1 )
          {
          this->RockButton->Deselect();
          this->MainViewSpin ( );
          }
        }
      if ( (b == this->RockButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent) )
        {
        // toggle the Rock (and turn off Spin if necessary)
        if (this->Rock == 1 )
          {
          this->Rock = 0;
          }
        else if ( this->Rock == 0 )
          {
          this->Rock = 1;
          }
        // handle the interaction
        if ( this->Rock == 1 )
          {
          this->SpinButton->Deselect();
          this->MainViewRock ( );
          }
        }

      //--- automatic camera control mode: switch 'rotate around axis' or 'look from direction'
      if (( b == this->RotateAroundButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent)  &&
          ( this->ViewAxisMode == vtkSlicerViewControlGUI::LookFrom) )
        {
        this->ViewAxisMode = vtkSlicerViewControlGUI::RotateAround;
        }
      if (( b == this->LookFromButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent ) &&
          (this->ViewAxisMode == vtkSlicerViewControlGUI::RotateAround) )
        {
        this->ViewAxisMode = vtkSlicerViewControlGUI::LookFrom;
        }

      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildStereoSelectMenu ( )
{
  this->StereoButton->GetMenu()->DeleteAllItems ( );
  this->StereoButton->GetMenu()->AddRadioButton ( "No stereo" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Red/Blue" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Full/Color" );
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildViewSelectMenu ( )
{
  
  this->SelectButton->GetMenu( )->DeleteAllItems();
  this->SelectButton->GetMenu()->AddRadioButton ("Save current" );
  // save current option will save current view under a
  // standard name like "View0...ViewN"; user can rename
  // this view elsewhere, in the ViewModule.
  // TODO: get existing MRMLViewNodes and add to menu

}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewRock ( )
{
  if ( this->Rock )
    {
/*
  if ( this->GetApplicationGUI() != NULL )
      {
      vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
      vtkMRMLCameraNode *cam = p->GetViewerWidget()->GetCameraNode();
      vtkMRMLViewNode *v = p->GetViewerWidget()->GetViewNode();
    
      // change icon to stop rock icon.
      double frac = this->RockCount / this->RockLength;
      double az = 1.5 * cos ( 2.0 * 3.1415926 * (frac- floor(frac)));
      this->SetRockCount ( this->GetRockCount() + 1 );
      // Move the camera
      cam->GetCamera()->Azimuth ( az );
      //Make the lighting follow the camera to avoid illumination changes
      //  appGUI->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
      // Render
      // Render3D
      this->Script ( "update idletasks" );
      const char *name = this->GetTclName();
      this->Script ( "after %s %s MainViewRock", this->SpinMs, name );
      }
*/
      }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::StopViewRock ( )
{
  this->Rock = 0;
  this->RockButton->Deselect();

}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewSpin ( )
{

  if ( this->Spin )
    {
    // change icon to stop spin icon
    /*
    this->MainViewRotate (this->SpinDirection, this->SpinDegrees );
    this->Script ( "update idletasks" );
    const char *name = this->GetTclName();
    this->Script ( "after %s %s MainViewSpin", this->SpinMs, name );
    */
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::StopViewSpin ( )
{
  this->Spin = 0;
  this->SpinButton->Deselect();

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewRotate ( int dir )
{

  this->MainViewRotate( dir, this->SpinDegrees );

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewRotate ( int dir, double degrees )
{
  
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    vtkMRMLCameraNode *c = p->GetViewerWidget()->GetCameraNode();
    double ndegrees = -degrees;

    switch ( dir ) {
    case Up:
      c->GetCamera()->Elevation ( degrees );
      break;
    case Down:
      c->GetCamera()->Elevation ( ndegrees );
      break;
    case Left:
      c->GetCamera()->Azimuth ( degrees );
      break;
    case Right:
      c->GetCamera()->Azimuth ( ndegrees );
      break;
    default:
      break;
    }
    c->GetCamera()->OrthogonalizeViewUp ( );

    //Make the lighting follow the camera to avoid illumination changes
    //  appGUI->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();

    //Render
    //p->Render3D;
    }
  
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
    // Fill in
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
void vtkSlicerViewControlGUI::EnterViewAxisACallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
    this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
    }
  else
    {
    this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisACallback ( ) {
  this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconLO() );
  this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisPCallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
    this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconHI() );
    }
  else
    {
    this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconHI() );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisPCallback ( ) {
  this->ViewAxisPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisPIconLO() );
  this->ViewAxisAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisRCallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
    this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
    }
  else
    {
    this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisRCallback ( ) {
  this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconLO() );
  this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisLCallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconHI() );
    this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
    }
  else
    {
    this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconHI() );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisLCallback ( ) {
  this->ViewAxisRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisRIconLO() );
  this->ViewAxisLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisSCallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
    this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
    }
  else
    {
    this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisSCallback ( ) {
    this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconLO() );
    this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterViewAxisICallback ( ) {
  if ( this->ViewAxisMode == RotateAround )
    {
    this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
    this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconHI() );
    }
  else
    {
    this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconHI() );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveViewAxisICallback ( ) {
  this->ViewAxisIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisIIconLO() );
  this->ViewAxisSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetViewAxisSIconLO() );
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

      // create and pack sub-frames for the ViewControl GUI
      vtkKWFrame *f0 = vtkKWFrame::New ( );
      vtkKWFrame *f1 = vtkKWFrame::New ( );
      vtkKWFrame *f2 = vtkKWFrame::New ( );
      vtkKWFrame *f3 = vtkKWFrame::New ( );
      vtkKWFrame *f4 = vtkKWFrame::New ( );
      vtkKWFrame *f5 = vtkKWFrame::New ( );
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
      f5->SetParent ( f0);
      f5->Create();
      f6->SetParent ( f0);
      f6->Create();
      this->Script ( "pack %s -side left -anchor nw -padx 2 -pady 2 -expand n", f0->GetWidgetName ( ) );      
      this->Script ( "grid %s -row 0 -column 0 -sticky w -padx 0 -pady 0", f1->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 0 -sticky w -padx 0 -pady 0", f2->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 0 -pady 0", f3->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 1  -sticky w -padx 0 -pady 0", f4->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 2  -sticky news -padx 0 -pady 0", f5->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 2  -sticky w -padx 0 -pady 0", f6->GetWidgetName ( ) );

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
      this->RotateAroundButton->SetParent ( f3 );
      this->RotateAroundButton->Create ( );
      this->RotateAroundButton->SetReliefToFlat ( );
      this->RotateAroundButton->SetBorderWidth ( 0 );
      this->RotateAroundButton->SetOverReliefToNone ( );
      this->RotateAroundButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundButtonIcon() );
      this->RotateAroundButton->SetBalloonHelpString ( "Set the 3D view control mode to 'rotate around' selected axis ");
      this->RotateAroundButton->SetValueAsInt ( 101 );
      this->LookFromButton->SetParent ( f3 );
      this->LookFromButton->SetReliefToFlat ( );
      this->LookFromButton->SetBorderWidth ( 0 );
      this->LookFromButton->SetOverReliefToNone ( );
      this->LookFromButton->Create ( );
      this->LookFromButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromButtonIcon() );
      this->LookFromButton->SetBalloonHelpString ( "Set the 3D view control mode to 'look from' selected direction");
      this->LookFromButton->SetValueAsInt ( 202 );
      this->LookFromButton->SetVariableName ( this->RotateAroundButton->GetVariableName( ) );
      this->LookFromButton->SetSelectedState(1);
      this->ViewAxisMode = vtkSlicerViewControlGUI::LookFrom;

      this->OrthoButton->SetParent ( f3);
      this->OrthoButton->Create ( );
      this->OrthoButton->SetReliefToFlat ( );
      this->OrthoButton->SetBorderWidth ( 0 );
      this->OrthoButton->SetOverReliefToNone ( );
      this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetOrthoButtonIcon() );      
      this->OrthoButton->SetBalloonHelpString ( "Toggle between orthographic and perspective rendering in the 3D view.");
      this->CenterButton->SetParent ( f3);
      this->CenterButton->Create ( );
      this->CenterButton->SetReliefToFlat ( );
      this->CenterButton->SetBorderWidth ( 0 );
      this->CenterButton->SetOverReliefToNone ( );
      this->CenterButton->SetImageToIcon ( this->SlicerViewControlIcons->GetCenterButtonIcon() );      
      this->CenterButton->SetBalloonHelpString ( "Center the 3D view on the scene.");
      this->StereoButton->SetParent ( f3);
      this->StereoButton->Create ( );
      this->StereoButton->SetReliefToFlat ( );
      this->StereoButton->SetBorderWidth ( 0 );
      this->StereoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetStereoButtonIcon() );      
      this->StereoButton->IndicatorVisibilityOff ( );
      this->StereoButton->SetBalloonHelpString ( "Select among stereo viewing options.");
      this->SelectButton->SetParent ( f3);
      this->SelectButton->Create ( );
      this->SelectButton->SetReliefToFlat ( );
      this->SelectButton->SetBorderWidth ( 0 );
      this->SelectButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSelectButtonIcon() );
      this->SelectButton->IndicatorVisibilityOff ( );
      this->SelectButton->SetBalloonHelpString ( "Select among saved 3D views.");
      this->SpinButton->SetParent ( f3 );
      this->SpinButton->Create ( );
      this->SpinButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSpinButtonIcon() );      
      this->SpinButton->Deselect();
      this->SpinButton->SetBalloonHelpString ( "Spin the 3D view.");
      this->RockButton->SetParent ( f3 );
      this->RockButton->Create ( );
      this->RockButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRockButtonIcon() );      
      this->RockButton->SetBalloonHelpString ( "Rock the 3D view.");
      this->RockButton->Deselect();
      // TODO: why did i have to padx by 4 to get the grid to line up?
      // this works on  win32; will it break on other platforms?
      this->Script ("grid %s -row 0 -column 0 -sticky w -padx 4 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 0 -sticky w -padx 2 -pady 0 -ipadx 0 -ipady 0", this->LookFromButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 1 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->OrthoButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 1 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->StereoButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 0 -column 2 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->CenterButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 2 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->SelectButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 0 -column 3 -sticky e -padx 2 -pady 0 -ipadx 0 -ipady 0", this->SpinButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 3 -sticky e -padx 2 -pady 0 -ipadx 0 -ipady 0", this->RockButton->GetWidgetName ( ));

      // create and pack entry widgets
      this->FOVEntry->SetParent ( f4 );
      this->FOVEntry->Create ( );
      this->FOVEntry->GetLabel()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-8-*-*-*-*-*-*-*" );
      this->FOVEntry->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*" );
      this->FOVEntry->SetLabelText ( "FOV: ");
      this->FOVEntry->GetWidget()->SetWidth (7);
      this->ZoomEntry->SetParent ( f4 );
      this->ZoomEntry->Create ( );
      this->ZoomEntry->GetLabel()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-8-*-*-*-*-*-*-*" );
      this->ZoomEntry->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*" );
      this->ZoomEntry->SetLabelText ( "%: ");
      this->ZoomEntry->GetWidget()->SetWidth (7);
      this->Script ( "pack %s -side left -anchor w -padx 1 -pady 2 -expand n", this->FOVEntry->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor e -padx 2 -pady 2 -expand n", this->ZoomEntry->GetWidgetName ( ) );
      
      // TODO: replace with real nav zoom controls
      f5->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->NavZoomLabel->SetParent (f5);
      this->NavZoomLabel->Create();        
      this->NavZoomLabel->SetWidth ( 20);
      this->NavZoomLabel->SetHeight ( 5 );
      this->NavZoomLabel->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*" );
      this->NavZoomLabel->SetText ( "3DNav / SliceZoom" );
      this->NavZoomLabel->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->NavZoomLabel->GetWidgetName ( ) );

      //--- create the nav/zoom widgets
      this->NavZoomInIconButton->SetParent ( f6 );
      this->NavZoomInIconButton->Create ( );
      this->NavZoomInIconButton->SetReliefToFlat ( );        
      this->NavZoomOutIconButton->SetParent ( f6 );        
      this->NavZoomOutIconButton->Create ( );
      this->NavZoomOutIconButton->SetReliefToFlat ( );
      this->NavZoomScale->SetParent ( f6 );
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
      
      this->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -expand n", this->NavZoomOutIconButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor n -padx 0 -pady 2 -fill x -expand n", this->NavZoomScale->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor n -padx 2 -pady 2 -expand n", this->NavZoomInIconButton->GetWidgetName ( ) );

      // populate menus
      this->BuildViewSelectMenu();
      this->BuildStereoSelectMenu ( );
      
      // clean up
      f0->Delete ( );
      f1->Delete ( );
      f2->Delete ( );
      f3->Delete ( );
      f4->Delete ( );
      f5->Delete ( );
      f6->Delete ( );
      }
    }
}



