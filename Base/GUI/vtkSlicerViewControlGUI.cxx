#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkImageMapper.h"
#include "vtkActor2D.h"
#include "vtkSlicerInteractorStyle.h"

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

#define NAVZOOMWIDGET_DEBUG

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewControlGUI );
vtkCxxRevisionMacro ( vtkSlicerViewControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::vtkSlicerViewControlGUI ( )
{
  
  this->RenderPending = 0;
  this->SceneClosing = false;
  this->ProcessingMRMLEvent = 0;
  this->RockCount = 0;
  this->NavZoomWidgetWid = 150;
  this->NavZoomWidgetHit = 80;
  this->Magnification = 10.0;

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
  this->FOVEntry = vtkKWEntryWithLabel::New ( );
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

  this->NavZoomInIconButton = vtkKWPushButton::New ( );
  this->NavZoomOutIconButton = vtkKWPushButton::New ( );
  this->NavZoomScale = vtkKWScale::New ( );

  // temporary thing until navzoom window is built.
  this->NavZoomLabel = vtkKWLabel::New ( );
  this->NavWidget = vtkKWRenderWidget::New( );
  this->ZoomWidget = vtkKWRenderWidget::New ( );
  this->NavZoomFrame = vtkKWFrame::New ( );

/*
  this->Zoomer = vtkImageMagnify::New();
  this->Zoomer->SetMagnificationFactors (this->Magnification, this->Magnification, this->Magnification);
  this->Zoomer->SetInterpolate(0);

  this->ZoomExtractor = vtkExtractVOI::New ( );
  this->ZoomExtractor->SetSampleRate ( 1, 1, 1);
  this->ZoomChanger = vtkImageChangeInformation::New ( );
  
  this->ZoomCursor = vtkGlyphSource2D::New();
  this->ZoomCursor->SetGlyphTypeToSquare();
  this->ZoomCursor->SetFilled(0);
  this->ZoomCursor->SetColor ( 1.0, 1.0, 0.0);
  this->ZoomCursor->SetCenter( this->NavZoomWidgetWid/2.0, this->NavZoomWidgetHit/2.0, 0.0);
*/
  
  this->Zoomer = NULL;
  this->ZoomExtractor = NULL;
  this->ZoomChanger = NULL;
  this->ZoomCursor = NULL;
  
  this->ViewNode = NULL;
  this->Slice0Events = NULL;
  this->Slice1Events = NULL;
  this->Slice2Events = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::~vtkSlicerViewControlGUI ( )
{

  this->RenderPending = 0;
  this->SceneClosing = false;
  this->RockCount = 0;  

  if ( this->ZoomCursor)
    {
    this->ZoomCursor->Delete();
    this->ZoomCursor = NULL;
    }
  if ( this->Zoomer)
    {
    this->Zoomer->SetInput ( NULL );
    this->Zoomer->Delete ();
    this->Zoomer = NULL;
    }
  if ( this->ZoomChanger)
    {
    this->ZoomChanger->SetInput ( NULL );
    this->ZoomChanger->Delete ( );
    this->ZoomChanger = NULL;
    }
  if ( this->ZoomExtractor )
    {
    this->ZoomExtractor->SetInput ( NULL );
    this->ZoomExtractor->Delete();
    this->ZoomExtractor = NULL;
    }
  if ( this->SlicerViewControlIcons )
    {
    this->SlicerViewControlIcons->Delete ( );
    this->SlicerViewControlIcons = NULL;
    }
  if ( this->NavZoomLabel )
    {
    this->NavZoomLabel->Delete ( );
    this->NavZoomLabel = NULL;
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
  if ( this->NavWidget )
    {
    this->NavWidget->SetParent ( NULL );
    this->NavWidget->Delete ();
    this->NavWidget = NULL;
    }
  if ( this->ZoomWidget )
    {
    this->ZoomWidget->SetParent ( NULL );
    this->ZoomWidget->Delete ();
    this->ZoomWidget = NULL;
    }
  if ( this->NavZoomFrame )
    {
    this->NavZoomFrame->SetParent ( NULL );
    this->NavZoomFrame->Delete ();
    this->NavZoomFrame = NULL;
    }

  this->RemoveViewObservers();
  this->SetViewNode ( NULL );
  this->RemoveSliceGUIObservers();
  this->SetSlice0Events(NULL);
  this->SetSlice1Events(NULL);
  this->SetSlice2Events(NULL);
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
  os << indent << "NavZoomInIconButton: " << this->GetNavZoomInIconButton (  ) << "\n";
  os << indent << "NavZoomOutIconButton: " << this->GetNavZoomOutIconButton (  ) << "\n";
  os << indent << "NavZoomScale: " << this->GetNavZoomScale (  ) << "\n";
  os << indent << "NavZoomLabel: " << this->GetNavZoomLabel (  ) << "\n";
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
  os << indent << "FOVEntry: " << this->GetFOVEntry(  ) << "\n";
  os << indent << "ZoomEntry: " << this->GetZoomEntry(  ) << "\n";
  os << indent << "SlicerViewControlIcons: " << this->GetSlicerViewControlIcons(  ) << "\n";
  os << indent << "ApplicationGUI: " << this->GetApplicationGUI(  ) << "\n";
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
    this->FOVEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ZoomEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->NavZoomScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->NavZoomScale->RemoveObservers (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
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
    this->FOVEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ZoomEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->NavZoomScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    this->NavZoomScale->AddObserver (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  // Right now this class contains state variables that will be moved
  // to a vtkMRMLViewNode in the next iteration.

  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( appGUI->GetApplication() );
    if ( app != NULL )
      {
      vtkKWCheckButton *b = vtkKWCheckButton::SafeDownCast ( caller );
      vtkKWPushButton *p = vtkKWPushButton::SafeDownCast ( caller );
      vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
      vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
      vtkKWScale *s = vtkKWScale::SafeDownCast ( caller );

      vtkSlicerInteractorStyle *istyle = vtkSlicerInteractorStyle::SafeDownCast ( caller );

#ifndef NAVZOOMWIDGET_DEBUG
      if ( istyle == this->Slice0Events || istyle == this->Slice1Events || istyle == this->Slice2Events)
        {
        this->SliceViewMagnify( event, istyle );
        }
#endif
      
      // Make requested changes to the ViewNode
      // save state for undo
      if ( e == this->FOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent  ||
           e == this->ZoomEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent  ||
           m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->VisibilityButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           m == this->SelectViewButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||           
           m == this->SelectCameraButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ||
           p == this->CenterButton && event == vtkKWPushButton::InvokedEvent ||                      
           p == this->OrthoButton && event == vtkKWPushButton::InvokedEvent ||                      
           b == this->SpinButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           b == this->RockButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           b == this->RotateAroundButton && event == vtkKWCheckButton::SelectedStateChangedEvent ||                      
           b == this->LookFromButton && event == vtkKWCheckButton::SelectedStateChangedEvent  ||
           s == this->NavZoomScale && event == vtkKWScale::ScaleValueChangingEvent  ||
           s == this->NavZoomScale && event == vtkKWScale::ScaleValueStartChangingEvent )                                                                                 
        {
        vtkMRMLViewNode *vn = this->GetActiveView();
        if ( vn != NULL )
          {
          appGUI->GetMRMLScene()->SaveStateForUndo( vn );

          if ( e == this->FOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
            {
            if ( vn != NULL )
              {
              if ( vn->GetFieldOfView() != this->FOVEntry->GetWidget()->GetValueAsDouble() )
                {
                vn->SetFieldOfView ( this->FOVEntry->GetWidget()->GetValueAsDouble ( ) );
                }
              }
            }

      
/*
  else if ( e == this->ZoomEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
  {
  if  (this->CameraNode != NULL )
  {
  {
  }
  }
  }
  else if ( s == this->NavZoomScale  && event == vtkKWScale::ScaleValueChangingEvent )
  {
  if  (this->CameraNode != NULL )
  {
  {
  }
  }
  }
*/
      
          if ( m == this->StereoButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            if ( vn != NULL )
              {
              if ( !strcmp (this->StereoButton->GetValue(), "NoStereo"))
                {
                vn->SetStereoType( vtkMRMLViewNode::NoStereo);
                }
              else if (!strcmp (this->StereoButton->GetValue(), "Red/Blue"))
                {
                vn->SetStereoType( vtkMRMLViewNode::RedBlue);
                }
              else if (!strcmp (this->StereoButton->GetValue(), "CrystalEyes"))
                {
                vn->SetStereoType( vtkMRMLViewNode::CrystalEyes);
                }
              else if (!strcmp (this->StereoButton->GetValue(), "Interlaced"))
                {
                vn->SetStereoType( vtkMRMLViewNode::Interlaced);            
                }
              }
            }
          else if ( m == this->VisibilityButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // Get all menu items
            if ( vn != NULL )
              {
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
                }
              if ( vn->GetAxisLabelsVisible() != m->GetItemSelectedState ("3D axis labels"))
                {
                vn->SetAxisLabelsVisible (m->GetItemSelectedState ("3D axis labels"));
                }
              if ( m->GetItemSelectedState ("Light blue background" ) == 1 )
                {
                vn->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
                }
              else if ( m->GetItemSelectedState ("Black background" ) == 1 )
                {
                vn->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->Black );
                }
              else if ( m->GetItemSelectedState ("White background" ) == 1 )
                {
                vn->SetBackgroundColor (app->GetSlicerTheme()->GetSlicerColors()->White );
                }            
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
            }

          // toggle the Ortho/Perspective rendering state
          if ( (p == this->OrthoButton) && (event == vtkKWPushButton::InvokedEvent ) && vn )
            {
            if ( vn != NULL )
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
          if (( b == this->RotateAroundButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent)  &&
              ( vn->GetViewAxisMode() == vtkMRMLViewNode::LookFrom) )
            {
            vn->SetViewAxisMode ( vtkMRMLViewNode::RotateAround );
            }
          if (( b == this->LookFromButton ) && ( event == vtkKWCheckButton::SelectedStateChangedEvent ) &&
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
void vtkSlicerViewControlGUI::MainViewSetStereo ( )
{
  // TODO: check whether stereo is enabled.
  int stereoEnabled = 0;
  if ( stereoEnabled )
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
          }
        }  
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::SliceViewMagnify(int event, vtkSlicerInteractorStyle *istyle )
{
  int x, y;
  int magx, magy, magz;
  this->Zoomer->GetMagnificationFactors ( magx, magy, magz );
  int xwid = int ( (this->NavZoomWidgetWid / 2.0) / (float)magx);
  int yhit = int ( (this->NavZoomWidgetHit / 2.0) / (float)magx);
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));    
    if ( istyle == this->Slice0Events )
      {
      this->ZoomExtractor->SetInput ( NULL );
      this->ZoomChanger->SetInput ( NULL );
      this->Zoomer->SetInput ( NULL );
      if ( event == vtkCommand::EnterEvent )
        {
        // configure zoom
        x = this->Slice0Events->GetLastPos ()[0];
        y = this->Slice0Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI0()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        // update zoom and render
        this->MagnifyActiveSlice();
        this->PackZoomWidget();
        }
      else if (event == vtkCommand::LeaveEvent )
        {
        this->PackNavWidget();
        }
      else if ( event == vtkCommand::MouseMoveEvent )
        {
        // configure zoom
        x = this->Slice0Events->GetLastPos ()[0];
        y = this->Slice0Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI0()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        // update zoom and render
        this->MagnifyActiveSlice();
        }
      }      
      
    else if ( istyle == this->Slice1Events)
      {
      this->ZoomExtractor->SetInput ( NULL );
      this->ZoomChanger->SetInput ( NULL );
      this->Zoomer->SetInput ( NULL );
      if (event == vtkCommand::EnterEvent )
        {
        // configure zoom
        x = this->Slice1Events->GetLastPos ()[0];
        y = this->Slice1Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI1()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        this->MagnifyActiveSlice();
        this->PackZoomWidget();
        }
      else if (event == vtkCommand::LeaveEvent )
        {
        this->PackNavWidget();
        }      
      else if ( event == vtkCommand::MouseMoveEvent )
        {
        x = this->Slice1Events->GetLastPos ()[0];
        y = this->Slice1Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI1()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        this->MagnifyActiveSlice();
        }
      }
      
    else if ( istyle == this->Slice2Events )
      {
      this->ZoomExtractor->SetInput ( NULL );
      this->ZoomChanger->SetInput ( NULL );
      this->Zoomer->SetInput ( NULL );
      if (event == vtkCommand::EnterEvent )
        {
        // configure zoom
        x = this->Slice2Events->GetLastPos ()[0];
        y = this->Slice2Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI2()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        this->MagnifyActiveSlice();
        this->PackZoomWidget();
        }
      else if (event == vtkCommand::LeaveEvent )
        {
        this->PackNavWidget();
        }      
      else if ( event == vtkCommand::MouseMoveEvent )
        {
        x = this->Slice2Events->GetLastPos ()[0];
        y = this->Slice2Events->GetLastPos ()[1];
        this->ZoomExtractor->SetInput (appGUI->GetMainSliceGUI2()->GetLogic()->GetImageData());
        this->ZoomExtractor->SetVOI ( x-xwid, x+xwid, y-yhit, y+yhit, 0, 0 );
        this->ZoomChanger->SetInput ( this->ZoomExtractor->GetOutput ());
        this->ZoomChanger->SetOutputExtentStart ( x-xwid, y-yhit, 0);
        this->ZoomChanger->SetExtentTranslation ( -(x-xwid), -(y-yhit), 0);
        this->ZoomChanger->SetOutputOrigin ( 0, 0, 0 );
        this->Zoomer->SetInput ( this->ZoomChanger->GetOutput () );
        this->MagnifyActiveSlice();
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MainViewResetFocalPoint ( )
{
  this->MainViewSetFocalPoint ( 0.0, 0.0, 0.0);
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
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render();

        // this->NavZoomRender();
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
      this->Script ( "update idletasks" );
      this->Script ( "after idle \"%s MainViewRock \"",  this->GetTclName() );
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
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render();
        // this->NavZoomRender();        
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
        this->Script ( "update idletasks" );
        this->Script ( "after idle \"%s MainViewSpin \"",  this->GetTclName() );
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
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render();
        // this->NavZoomRender();
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
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render(); 
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
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render();
        // this->NavZoomRender();        
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
          }
        else if ( !strcmp (dir, "L"))
          {
          cam->SetPosition ( fp[0]-widefov, fp[1], fp[2]);
          cam->SetViewUp ( 0, 0, 1);        
          }
        else if ( !strcmp (dir, "S"))
          {
          cam->SetPosition ( fp[0], fp[1], fp[2]+widefov );
          cam->SetViewUp ( 0, 1, 0);
          }
        else if ( !strcmp (dir, "I"))
          {
          cam->SetPosition ( fp[0], fp[1], fp[2]-widefov );
          cam->SetViewUp ( 0, 1, 0);
          }
        else if ( !strcmp (dir, "A"))
          {
          cam->SetPosition (fp[0], fp[1]+widefov, fp[2] );
          cam->SetViewUp ( 0, 0, 1 );
          }
        else if ( !strcmp (dir, "P"))
          {
          cam->SetPosition (fp[0], fp[1]-widefov, fp[2] );
          cam->SetViewUp ( 0, 0, 1 );
          }
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->ResetCameraClippingRange ( );
        cam->ComputeViewPlaneNormal();
        cam->OrthogonalizeViewUp();
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->UpdateLightsGeometryToFollowCamera();
        p->GetViewerWidget()->GetMainViewer()->GetRenderer()->Render();      
        // this->NavZoomRender();
        }
       }
     }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildCameraSelectMenu()
{
  
  this->SelectCameraButton->GetMenu( )->DeleteAllItems();
  this->SelectCameraButton->GetMenu()->AddRadioButton ("Save current camera (not yet available)" );
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
  this->StereoButton->GetMenu()->AddRadioButton ( "CrystalEyes" );
  this->StereoButton->GetMenu()->AddRadioButton ( "Interlaced" );
  this->StereoButton->GetMenu()->AddSeparator();
  this->StereoButton->GetMenu()->AddCommand ( "close");
  this->StereoButton->GetMenu()->SelectItem ( "No stereo");
  // TODO: check whether stereo is enabled.
  int stereoEnabled = 0;
  if ( !stereoEnabled )
    {
    this->StereoButton->GetMenu()->SetItemStateToDisabled ( "No stereo" );
    this->StereoButton->GetMenu()->SetItemStateToDisabled ( "Red/Blue" );
    this->StereoButton->GetMenu()->SetItemStateToDisabled ( "CrystalEyes" );
    this->StereoButton->GetMenu()->SetItemStateToDisabled ( "Interlaced" );
    }
  
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::BuildViewSelectMenu ( )
{
  
  this->SelectViewButton->GetMenu( )->DeleteAllItems();
  this->SelectViewButton->GetMenu()->AddRadioButton ("Save current view (not yet available)" );
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



  // has a node been added or deleted?
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
       && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    this->UpdateFromMRML();
    }
  
  vtkMRMLCameraNode *cnode = vtkMRMLCameraNode::SafeDownCast ( caller );
  vtkMRMLViewNode *vnode = vtkMRMLViewNode::SafeDownCast ( caller );
  vtkMRMLSelectionNode *snode = vtkMRMLSelectionNode::SafeDownCast ( caller );

  // has a new camera or view has been selected?
  if ( snode != NULL )
    {
     this->UpdateFromMRML();
    }    

  // has camera been manipulated (aside from auto spin or rock?)
  else if (cnode != NULL )
    {
    }  

  // has view been manipulated?
  else if ( vnode != NULL )
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
        }
      }
    // a field of view entry event?
    if ( this->FOVEntry->GetWidget()->GetValueAsDouble() != vnode->GetFieldOfView() )
      {
      this->FOVEntry->GetWidget()->SetValueAsDouble( vnode->GetFieldOfView() );
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
void vtkSlicerViewControlGUI::PackNavWidget ( )
{
  if ( this->ZoomWidget != NULL )
    {
    this->Script ("pack forget %s ", this->ZoomWidget->GetWidgetName() );
    }
  if ( this->NavWidget != NULL )
    {
    this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->NavWidget->GetWidgetName ( ) );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::PackZoomWidget ( )
{
  if ( this->NavWidget != NULL )
    {
    this->Script ("pack forget %s ", this->NavWidget->GetWidgetName() );
    }
  if ( this->ZoomWidget != NULL )
    {
    this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->ZoomWidget->GetWidgetName ( ) );
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

      // create and pack sub-frames for the ViewControl GUI
      vtkKWFrame *f0 = vtkKWFrame::New ( );
      vtkKWFrame *f1 = vtkKWFrame::New ( );
      vtkKWFrame *f2 = vtkKWFrame::New ( );
      vtkKWFrame *f3 = vtkKWFrame::New ( );
      vtkKWFrame *f4 = vtkKWFrame::New ( );
//      vtkKWFrame *f6 = vtkKWFrame::New ( );
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
      this->NavZoomFrame->SetParent ( f0);
      this->NavZoomFrame->Create();
//      f6->SetParent ( f0);
//      f6->Create();
      this->Script ( "pack %s -side left -anchor nw -padx 2 -pady 2 -expand n", f0->GetWidgetName ( ) );      
      this->Script ( "grid %s -row 0 -column 0 -sticky w -padx 0 -pady 0", f1->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 0 -sticky w -padx 0 -pady 0", f2->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 0 -pady 0", f3->GetWidgetName ( ) );
      this->Script ( "grid %s -row 1 -column 1  -sticky w -padx 0 -pady 0", f4->GetWidgetName ( ) );
      this->Script ( "grid %s -row 0 -column 2  -rowspan 2 -sticky news -padx 0 -pady 0", this->NavZoomFrame->GetWidgetName ( ) );
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
      this->RotateAroundButton->SetReliefToFlat ( );
      this->RotateAroundButton->SetBorderWidth ( 0 );
      this->RotateAroundButton->SetOverReliefToNone ( );
      this->RotateAroundButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRotateAroundButtonIcon() );
      this->RotateAroundButton->SetBalloonHelpString ( "Set the 3D view control mode to 'rotate around' selected axis ");
      this->RotateAroundButton->SetValueAsInt ( 101 );
      //--- Radiobutton to select view look from direction
      this->LookFromButton->SetParent ( f3 );
      this->LookFromButton->Create ( );
      this->LookFromButton->SetReliefToFlat ( );
      this->LookFromButton->SetBorderWidth ( 0 );
      this->LookFromButton->SetOverReliefToNone ( );
      this->LookFromButton->SetImageToIcon ( this->SlicerViewControlIcons->GetLookFromButtonIcon() );
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
      this->SelectViewButton->SetBalloonHelpString ( "Save curren or select among already saved 3D views.");
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
      this->SpinButton->SetImageToIcon ( this->SlicerViewControlIcons->GetSpinButtonIcon() );      
      this->SpinButton->Deselect();
      this->SpinButton->SetBalloonHelpString ( "Spin the 3D view.");
      //--- CheckButton to rotate the view
      this->RockButton->SetParent ( f3 );
      this->RockButton->Create ( );
      this->RockButton->SetImageToIcon ( this->SlicerViewControlIcons->GetRockButtonIcon() );      
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
      this->Script ("grid %s -row 0 -column 0 -sticky w -padx 4 -pady 0 -ipadx 0 -ipady 0", this->RotateAroundButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 0 -sticky w -padx 4 -pady 0 -ipadx 0 -ipady 0", this->LookFromButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 1 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->OrthoButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 1 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->StereoButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 2 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->VisibilityButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 1 -column 2 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->SelectCameraButton->GetWidgetName ( ));
      this->Script ("grid %s -row 0 -column 3 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->CenterButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 3 -sticky w -padx 1 -pady 0 -ipadx 0 -ipady 0", this->SelectViewButton->GetWidgetName ( ));      
      this->Script ("grid %s -row 0 -column 4 -sticky e -padx 2 -pady 0 -ipadx 0 -ipady 0", this->SpinButton->GetWidgetName ( ));
      this->Script ("grid %s -row 1 -column 4 -sticky e -padx 2 -pady 0 -ipadx 0 -ipady 0", this->RockButton->GetWidgetName ( ));

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

/*      
      f5->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->NavZoomLabel->SetParent (f5);
      this->NavZoomLabel->Create();        
      this->NavZoomLabel->SetWidth ( 15 );
      this->NavZoomLabel->SetHeight ( 6 );
      this->NavZoomLabel->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-10-*-*-*-*-*-*-*" );
      this->NavZoomLabel->SetText ( "3DNav / SliceZoom" );
      this->NavZoomLabel->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->NavZoomLabel->GetWidgetName ( ) );
*/

      //--- create the nav/zoom widgets
      this->ZoomWidget->SetParent ( this->NavZoomFrame );
      this->ZoomWidget->Create ( );
      this->ZoomWidget->SetWidth ( this->NavZoomWidgetWid );
      this->ZoomWidget->SetHeight ( this->NavZoomWidgetHit );
      this->ZoomWidget->SetRendererBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->Black );
      vtkImageMapper *zoomMapper = vtkImageMapper::New ( );
      zoomMapper->SetColorWindow(255);
      zoomMapper->SetColorLevel (127.5);
      zoomMapper->SetInput ( this->Zoomer->GetOutput() );
      vtkActor2D *zoomActor = vtkActor2D::New();
      zoomActor->SetMapper( zoomMapper);
      this->ZoomWidget->GetRenderer()->AddActor2D ( zoomActor );
      this->ZoomWidget->GetRenderWindow()->DoubleBufferOn();
      zoomMapper->Delete();
      zoomActor->Delete();

      this->NavWidget->SetParent (this->NavZoomFrame);
      this->NavWidget->Create();
      this->NavWidget->SetWidth ( this->NavZoomWidgetWid );
      this->NavWidget->SetHeight ( this->NavZoomWidgetHit );
      this->NavWidget->SetRendererBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
      vtkImageMapper *navMapper = vtkImageMapper::New ( );
      navMapper->SetColorWindow(255);
      navMapper->SetColorLevel (127.5);
//      navMapper->SetInput ( this->Zoomer->GetOutput() );
      vtkActor2D *navActor = vtkActor2D::New();
      navActor->SetMapper( navMapper);
      this->NavWidget->GetRenderer()->AddActor2D ( navActor );
      this->NavWidget->GetRenderWindow()->DoubleBufferOn();
      navMapper->Delete();
      navActor->Delete();

      // TODO: Why does the display flash the first time we 'pack forget' and then 'pack'?
      // pack zoom widget temporarily, then swap in navigation widget.
      this->Script ( "pack %s -side top -anchor c -padx 0 -pady 0 -fill x -fill y -expand n", this->ZoomWidget->GetWidgetName ( ) );      
      this->PackNavWidget ( );
      

/*   comment out for now; let nav/zoom widget fill space
      //--- create the nav/zoom scale and buttons
      this->NavZoomInIconButton->SetParent ( f6 );
      this->NavZoomInIconButton->Create ( );
      this->NavZoomInIconButton->SetReliefToFlat ( );        
      this->NavZoomOutIconButton->SetParent ( f6 );        
      this->NavZoomOutIconButton->Create ( );
      this->NavZoomOutIconButton->SetReliefToFlat ( );
      this->NavZoomScale->SetParent ( f6 );
      this->NavZoomScale->Create ( );
      this->NavZoomScale->SetRange (1.0, 200.0);
      this->NavZoomScale->SetResolution ( 2.0 );
      this->NavZoomScale->SetBorderWidth ( 1 );
      this->NavZoomScale->SetValue ( 100.0 );

      // make scale long enough to fill the frame,
      // leaving room for the zoomin, zoomout buttons.
      this->NavZoomScale->SetLength ( 110 );
      this->NavZoomScale->SetOrientationToHorizontal ( );
      this->NavZoomScale->ValueVisibilityOff ( );
      this->NavZoomScale->SetBalloonHelpString ( "Use scale to zoom the navigation window in/out" );
      //--- assign image data to the zoom buttons
      this->NavZoomInIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetNavZoomInIcon() );
      this->NavZoomOutIconButton->SetImageToIcon ( this->SlicerViewControlIcons->GetNavZoomOutIcon() );
      
      this->Script ( "pack %s -side left -anchor n -padx 0 -pady 2 -expand n", this->NavZoomOutIconButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor c -padx 0 -pady 2 -expand n", this->NavZoomScale->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor n -padx 0 -pady 2 -expand n", this->NavZoomInIconButton->GetWidgetName ( ) );
*/
      
      // populate menus
      this->BuildViewSelectMenu();
      this->BuildCameraSelectMenu();
      this->BuildStereoSelectMenu ( );
      this->BuildVisibilityMenu ( );
      
      // clean up
      f0->Delete ( );
      f1->Delete ( );
      f2->Delete ( );
      f3->Delete ( );
      f4->Delete ( );
      // f6->Delete ( );
      }
    }

}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::MagnifyActiveSlice()
{


  // update
  this->Zoomer->Update();
  // and render
  this->ZoomWidget->Render();
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddSliceGUIObservers()
{
  
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->Slice0Events != NULL )
      {
      this->Slice0Events->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice0Events->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice0Events->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      }
    if ( this->Slice1Events != NULL )
      {
      this->Slice1Events->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice1Events->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice1Events->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      }
    if ( this->Slice2Events != NULL )
      {
      this->Slice2Events->AddObserver ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice2Events->AddObserver ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice2Events->AddObserver ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      }
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveSliceGUIObservers()
{
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->Slice0Events != NULL )
      {
      this->Slice0Events->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice0Events->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice0Events->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      }
    if ( this->Slice1Events != NULL )
      {
      this->Slice1Events->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice1Events->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice1Events->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
      }
    if ( this->Slice2Events != NULL )
      {
      this->Slice2Events->RemoveObservers ( vtkCommand::EnterEvent, this->GUICallbackCommand );
      this->Slice2Events->RemoveObservers ( vtkCommand::LeaveEvent, this->GUICallbackCommand );
      this->Slice2Events->RemoveObservers ( vtkCommand::MouseMoveEvent, this->GUICallbackCommand );
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
void vtkSlicerViewControlGUI::UpdateGUI ( )
{

  vtkMRMLViewNode *vn = this->GetActiveView();
  if ( vn != NULL )
    {
    this->FOVEntry->GetWidget()->SetValueAsDouble ( vn->GetFieldOfView() );
    // for now.
    this->ZoomEntry->GetWidget()->SetValueAsDouble ( 100.0 );
    if ( vn->GetRenderMode() == vtkMRMLViewNode::Orthographic )
      {
      this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetPerspectiveButtonIcon() );
      }
    else
      {
      this->OrthoButton->SetImageToIcon ( this->SlicerViewControlIcons->GetOrthoButtonIcon() );
      }
    switch ( vn->GetStereoType() )
      {
      case vtkMRMLViewNode::NoStereo:
        this->StereoButton->GetMenu()->SelectItem ( "No stereo");
        break;
      case vtkMRMLViewNode::RedBlue:
        this->StereoButton->GetMenu()->SelectItem ( "Red/Blue");
        break;
      case vtkMRMLViewNode::Interlaced:
        this->StereoButton->GetMenu()->SelectItem ( "Interlaced");        
        break;
      case vtkMRMLViewNode::CrystalEyes:
        this->StereoButton->GetMenu()->SelectItem ( "CrystalEyes");
        break;
      default:
        break;
      }
    if ( vn->GetFiducialsVisible() == 1 )
      {
//      this->VisibilityButton->GetMenu()->SelectItem ("Fiducial points" );
      }
    else
      {
//      this->VisibilityButton->GetMenu()->DeselectItem ("Fiducial points" );
      }
    if ( vn->GetFiducialLabelsVisible() == 1 )
      {
//      this->VisibilityButton->GetMenu()->SelectItem ("Fiducial labels" );
      }
    else
      {
//      this->VisibilityButton->GetMenu()->DeselectItem ("Fiducial labels" );
      }
    if ( vn->GetBoxVisible() == 1 )
      {
      this->VisibilityButton->GetMenu()->SelectItem ("3D cube" );
      }
    else
      {
      this->VisibilityButton->GetMenu()->DeselectItem ("3D cube" );
      }
    if ( vn->GetAxisLabelsVisible ( ) == 1 )
      {
      this->VisibilityButton->GetMenu()->SelectItem ("3D axis labels" );
      }
    else
      {
      this->VisibilityButton->GetMenu()->DeselectItem ("3D axis labels" );
      }
    double *c = vn->GetBackgroundColor();
    if ( c[0] == 0.0 )
      {
      this->VisibilityButton->GetMenu()->SelectItem ("Black background" );
      }
    if  (c[0] == 1.0 )
      {
      this->VisibilityButton->GetMenu()->SelectItem ("White background");
      }
    else 
      {  

      this->VisibilityButton->GetMenu()->SelectItem ("Light blue background");
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
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->Slice0Events ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI1()->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->Slice1Events ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI2()->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->Slice2Events ) )
    {
    this->RemoveSliceGUIObservers();
    this->SetSlice0Events(NULL );
    this->SetSlice1Events(NULL );
    this->SetSlice2Events(NULL );

    this->SetSlice0Events( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI0()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetSlice1Events( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI1()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetSlice2Events( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI2()->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->AddSliceGUIObservers();
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
  // repopulate the NavZoom widget's actors, etc.,
  // and rerender the NavZoom widget's view.

  this->UpdateView();
  this->UpdateCurrentCameraAndActors ( );
  this->RequestRender ( );
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateView()
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
    this->RemoveViewObservers();
    this->SetViewNode (NULL);
    }
  if ( this->ViewNode != NULL && this->MRMLScene->GetNodeByID(this->ViewNode->GetID()) == NULL)
    {
    // local node not in the scene
    this->RemoveViewObservers();
    this->SetViewNode(NULL );
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
    this->SetViewNode ( node );
    this->AddViewObservers();
    }
}




//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddViewObservers()
{
  // observe scene for add/remove nodes
  if ( this->ViewNode )
    {
    this->ViewNode->AddObserver ( vtkMRMLViewNode::AnimationModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->AddObserver ( vtkMRMLViewNode::RenderModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->AddObserver ( vtkMRMLViewNode::StereoModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->AddObserver ( vtkMRMLViewNode::VisibilityEvent, this->MRMLCallbackCommand );
    this->ViewNode->AddObserver ( vtkMRMLViewNode::BackgroundColorEvent, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveViewObservers()
{
  if ( this->ViewNode )
    {
    this->ViewNode->RemoveObservers ( vtkMRMLViewNode::AnimationModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->RemoveObservers ( vtkMRMLViewNode::RenderModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->RemoveObservers ( vtkMRMLViewNode::StereoModeEvent, this->MRMLCallbackCommand );
    this->ViewNode->RemoveObservers ( vtkMRMLViewNode::VisibilityEvent, this->MRMLCallbackCommand );
    this->ViewNode->RemoveObservers ( vtkMRMLViewNode::BackgroundColorEvent, this->MRMLCallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::UpdateCurrentCameraAndActors ( )
{
  // TODO: implement
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RequestRender()
{
  if (this->GetRenderPending())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after idle \"%s Render\"", this->GetTclName());
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::Render()
{

  // TODO: implement
//  this->NavZoomViewer->Render();
  this->SetRenderPending(0);
}
