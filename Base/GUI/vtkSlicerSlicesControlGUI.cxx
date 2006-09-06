#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"

#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesControlGUI );
vtkCxxRevisionMacro ( vtkSlicerSlicesControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::vtkSlicerSlicesControlGUI ( )
{
     //--- ui for the SliceControlframe.
    this->ToggleAnnotationButton = vtkKWPushButton::New ( );
    this->ToggleFgBgButton = vtkKWPushButton::New ( );
    this->SliceFadeScale = vtkKWScale::New ( );
    this->SliceOpacityScale = vtkKWScale::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::~vtkSlicerSlicesControlGUI ( )
{

   //--- widgets from the SlicesControlFrame
    if ( this->ToggleAnnotationButton ) {
      this->ToggleAnnotationButton->SetParent ( NULL );
        this->ToggleAnnotationButton->Delete ( );
        this->ToggleAnnotationButton = NULL;
    }
    if ( this->ToggleFgBgButton ) {
      this->ToggleFgBgButton->SetParent ( NULL );
        this->ToggleFgBgButton->Delete ( );
        this->ToggleFgBgButton = NULL;
    }
    if ( this->SliceFadeScale ) {
      this->SliceFadeScale->SetParent ( NULL );
        this->SliceFadeScale->Delete ( );
        this->SliceFadeScale = NULL;
    }

    if ( this->SliceOpacityScale ) {
      this->SliceOpacityScale->SetParent ( NULL );
        this->SliceOpacityScale->Delete ( );
        this->SliceOpacityScale = NULL;
    }
    this->SetApplicationGUI ( NULL );
}





//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerSlicesControlGUI: " << this->GetClassName ( ) << "\n";

}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::RemoveGUIObservers ( )
{
  // FILL IN
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->SliceOpacityScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceOpacityScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->ToggleFgBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::AddGUIObservers ( )
{
  // FILL IN
  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ToggleFgBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceOpacityScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceOpacityScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
  vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);

  if ( this->GetApplicationGUI() != NULL )
    {
      vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      if ( app != NULL )
        {

          // Process the Fade scale and button
          // -- set save state when manipulation starts
          // -- toggle the value if needed
          // -- adjust the Opacity of every composite node on every event
          if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueStartChangingEvent ||
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent )
            {
              if (p->GetMRMLScene()) 
                {
                  p->GetMRMLScene()->SaveStateForUndo();
                }
            }

          if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueChangingEvent ||
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent )
            {

              if ( pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ) 
                {
                  // it seems like this following statement should be handled
                  // in ProcessMRMLEvents, after the Composite node changes, no?
                  this->SliceFadeScale->SetValue( 1.0 - this->SliceFadeScale->GetValue() );
                }

              int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
              vtkMRMLSliceCompositeNode *cnode;
              for (i = 0; i < nnodes; i++)
                {
                  cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                   p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                  cnode->SetForegroundOpacity( this->SliceFadeScale->GetValue() );
                }
            }

          // Process the label Opacity scale 
          // -- set save state when manipulation starts
          // -- adjust the Opacity of every composite node on every event
          if ( scale == this->SliceOpacityScale && event == vtkKWScale::ScaleValueStartChangingEvent )
            {
              if (p->GetMRMLScene()) 
                {
                  p->GetMRMLScene()->SaveStateForUndo();
                }
            }

          if ( scale == this->SliceOpacityScale && event == vtkKWScale::ScaleValueChangingEvent )
            {

              int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
              vtkMRMLSliceCompositeNode *cnode;
              for (i = 0; i < nnodes; i++)
                {
                  cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                   p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                  cnode->SetLabelOpacity( this->SliceOpacityScale->GetValue() );
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}




//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildGUI ( vtkKWFrame *appF )
{

  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
  //--- Populate the Slice Control Frame
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
        appF->SetReliefToGroove();
        
        //--- create frames
        vtkKWFrame *f1 = vtkKWFrame::New ( );
        f1->SetParent ( appF );
        f1->Create ( );
        vtkKWFrame *f2 = vtkKWFrame::New ( );
        f2->SetParent ( appF );
        f2->Create ( );
        vtkKWFrame *f3 = vtkKWFrame::New ( );
        f3->SetParent ( appF );
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
        this->SliceOpacityScale->SetValue ( 1.0 );
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

}





