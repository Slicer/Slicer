#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerViewControlGUI.h"
#include "vtkSlicerWindow.h"

#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkSlicerViewControlIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewControlGUI );
vtkCxxRevisionMacro ( vtkSlicerViewControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::vtkSlicerViewControlGUI ( )
{
  
  this->SlicerViewControlIcons = vtkSlicerViewControlIcons::New ( );
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
}


//---------------------------------------------------------------------------
vtkSlicerViewControlGUI::~vtkSlicerViewControlGUI ( )
{

  if ( this->SlicerViewControlIcons )
    {
      this->SlicerViewControlIcons->Delete ( );
      this->SlicerViewControlIcons = NULL;
    }

     //--- widgets from ViewControlFrame
    if ( this->SpinButton ) {
      this->SpinButton->SetParent ( NULL );
        this->SpinButton->Delete();
        this->SpinButton = NULL;
    }
    if ( this->RockButton) {
      this->RockButton->SetParent ( NULL );
        this->RockButton->Delete();
        this->RockButton = NULL;
    }
    if ( this->OrthoButton ) {
      this->OrthoButton->SetParent ( NULL );
        this->OrthoButton->Delete();
        this->OrthoButton = NULL;
    }
    if ( this->CenterButton ) {
      this->CenterButton->SetParent ( NULL );      
        this->CenterButton->Delete();
        this->CenterButton = NULL;
    }
    if ( this->SelectButton ) {
      this->SelectButton->SetParent ( NULL );
        this->SelectButton->Delete();
        this->SelectButton = NULL;
    }
    if ( this->FOVEntry ) {
      this->FOVEntry->SetParent ( NULL );
        this->FOVEntry->Delete();
        this->FOVEntry= NULL;
    }
        if ( this->RotateAroundAIconButton ) {
      this->RotateAroundAIconButton->SetParent ( NULL );      
        this->RotateAroundAIconButton->Delete ( );
        this->RotateAroundAIconButton = NULL;
    }
    if ( this->RotateAroundPIconButton ) {
      this->RotateAroundPIconButton->SetParent ( NULL );
        this->RotateAroundPIconButton->Delete ( );
        this->RotateAroundPIconButton = NULL;
    }
    if ( this->RotateAroundRIconButton ) {
      this->RotateAroundRIconButton->SetParent ( NULL );
        this->RotateAroundRIconButton->Delete ( );
        this->RotateAroundRIconButton = NULL;
    }
    if ( this->RotateAroundLIconButton ) {
      this->RotateAroundLIconButton->SetParent ( NULL );
        this->RotateAroundLIconButton->Delete ( );
        this->RotateAroundLIconButton = NULL;
    }
    if ( this->RotateAroundSIconButton ) {
      this->RotateAroundSIconButton->SetParent ( NULL );
        this->RotateAroundSIconButton->Delete ( );
        this->RotateAroundSIconButton = NULL;
    }
    if ( this->RotateAroundIIconButton ) {
      this->RotateAroundIIconButton->SetParent ( NULL );
        this->RotateAroundIIconButton->Delete ( );
        this->RotateAroundIIconButton = NULL;
    }
    if ( this->RotateAroundMiddleIconButton ) {
      this->RotateAroundMiddleIconButton->SetParent ( NULL );
        this->RotateAroundMiddleIconButton->Delete ( );
        this->RotateAroundMiddleIconButton = NULL;
    }
    if ( this->RotateAroundTopCornerIconButton ) {
      this->RotateAroundTopCornerIconButton->SetParent ( NULL );
        this->RotateAroundTopCornerIconButton->Delete ( );
        this->RotateAroundTopCornerIconButton = NULL;
    }
    if ( this->RotateAroundBottomCornerIconButton ) {
      this->RotateAroundBottomCornerIconButton->SetParent ( NULL );
        this->RotateAroundBottomCornerIconButton->Delete ( );
        this->RotateAroundBottomCornerIconButton = NULL;
    }
    if ( this->LookFromAIconButton ) {
      this->LookFromAIconButton->SetParent ( NULL );
        this->LookFromAIconButton->Delete ( );
        this->LookFromAIconButton = NULL;
    }
    if ( this->LookFromPIconButton ) {
      this->LookFromPIconButton->SetParent ( NULL );
        this->LookFromPIconButton->Delete ( );
        this->LookFromPIconButton = NULL;
    }
    if ( this->LookFromRIconButton ) {
      this->LookFromRIconButton->SetParent ( NULL );
        this->LookFromRIconButton->Delete ( );
        this->LookFromRIconButton = NULL;
    }
    if ( this->LookFromLIconButton ) {
      this->LookFromLIconButton->SetParent ( NULL );
        this->LookFromLIconButton->Delete ( );
        this->LookFromLIconButton = NULL;
    }
    if ( this->LookFromSIconButton ) {
      this->LookFromSIconButton->SetParent ( NULL );
        this->LookFromSIconButton->Delete ( );
        this->LookFromSIconButton = NULL;
    }
    if ( this->LookFromIIconButton ) {
      this->LookFromIIconButton->SetParent ( NULL );
        this->LookFromIIconButton->Delete ( );
        this->LookFromIIconButton = NULL;
    }
    if ( this->LookFromMiddleIconButton ) {
      this->LookFromMiddleIconButton->SetParent ( NULL );
        this->LookFromMiddleIconButton->Delete ( );
        this->LookFromMiddleIconButton = NULL;
    }
    if ( this->LookFromTopCornerIconButton ) {
      this->LookFromTopCornerIconButton->SetParent ( NULL );
        this->LookFromTopCornerIconButton->Delete ( );
        this->LookFromTopCornerIconButton = NULL;
    }
    if ( this->LookFromBottomCornerIconButton ) {
      this->LookFromBottomCornerIconButton->SetParent ( NULL );
        this->LookFromBottomCornerIconButton->Delete ( );
        this->LookFromBottomCornerIconButton = NULL;
    }
    if ( this->NavZoomInIconButton ) {
      this->NavZoomInIconButton->SetParent ( NULL );
        this->NavZoomInIconButton->Delete ( );
        this->NavZoomInIconButton = NULL;
    }
    if ( this->NavZoomOutIconButton ) {
      this->NavZoomOutIconButton->SetParent ( NULL );
        this->NavZoomOutIconButton->Delete ( );
        this->NavZoomOutIconButton = NULL;
    }
    if ( this->NavZoomScale ) {
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
void vtkSlicerViewControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerViewControlGUI: " << this->GetClassName ( ) << "\n";

}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::RemoveGUIObservers ( )
{
  // FILL IN
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::AddGUIObservers ( )
{
  // FILL IN
}


//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  if ( this->GetApplicationGUI() != NULL )
    {
      vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
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
void vtkSlicerViewControlGUI::AssignIcons ( )
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
void vtkSlicerViewControlGUI::EnterRotateAroundACallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconHI() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundACallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconLO() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterRotateAroundPCallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconHI() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundPCallback ( ) {
    this->RotateAroundPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundPIconLO() );
    this->RotateAroundAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterRotateAroundRCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconHI() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundRCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconLO() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterRotateAroundLCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconHI() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundLCallback ( ) {
    this->RotateAroundRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundRIconLO() );
    this->RotateAroundLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterRotateAroundSCallback ( ) {
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconHI() );
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundSCallback ( ) {
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconLO() );
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterRotateAroundICallback ( ) {
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconHI() );
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveRotateAroundICallback ( ) {
    this->RotateAroundIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundIIconLO() );
    this->RotateAroundSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetRotateAroundSIconLO() );

}



//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromACallback ( ) {
    this->LookFromAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromAIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromACallback ( ) {
    this->LookFromAIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromAIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromPCallback ( ) {
    this->LookFromPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromPIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromPCallback ( ) {
    this->LookFromPIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromPIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromRCallback ( ) {
    this->LookFromRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromRIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromRCallback ( ) {
    this->LookFromRIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromRIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromLCallback ( ) {
    this->LookFromLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromLIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromLCallback ( ) {
    this->LookFromLIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromLIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromSCallback ( ) {
    this->LookFromSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromSIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromSCallback ( ) {
    this->LookFromSIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromSIconLO() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::EnterLookFromICallback ( ) {
    this->LookFromIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromIIconHI() );
}

//---------------------------------------------------------------------------
void vtkSlicerViewControlGUI::LeaveLookFromICallback ( ) {
    this->LookFromIIconButton->SetImageToIcon (this->SlicerViewControlIcons->GetLookFromIIconLO() );
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

        vtkKWFrame *f1 = vtkKWFrame::New ( );
        vtkKWFrame *f1a = vtkKWFrame::New ( );    
        vtkKWFrame *f1b = vtkKWFrame::New ( );    
        vtkKWFrame *f2 = vtkKWFrame::New ( );

        // divide the GUI panel into two frames of identical wid.
        int wid = layout->GetDefaultGUIPanelWidth() ;
        int buf = 4;
        int thirdwid = wid/3 - buf;
        
        // create frames and set their widths.
        f1->SetParent ( appF );
        f1->Create ( );
        f1->SetWidth ( thirdwid );
        f1->SetHeight (layout->GetDefaultViewControlFrameHeight ( ) );
        f1->SetReliefToGroove();

        f2->SetParent ( appF );
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

        this->AssignIcons ( );
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
}





