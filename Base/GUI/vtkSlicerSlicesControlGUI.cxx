#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSlicesControlIcons.h"

#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntry.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesControlGUI );
vtkCxxRevisionMacro ( vtkSlicerSlicesControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::vtkSlicerSlicesControlGUI ( )
{

  //--- ui for the SliceControlframe.
  this->SlicesControlIcons = vtkSlicerSlicesControlIcons::New ( );
  this->SliceFadeScale = vtkKWScale::New ( );
  this->ShowFgButton = vtkKWPushButton::New ( );
  this->ShowBgButton = vtkKWPushButton::New ( );
  this->ToggleFgBgButton = vtkKWPushButton::New ( );
  this->LabelOpacityButton = vtkKWPushButton::New ( );
  this->LinkControlsButton = vtkKWPushButton::New ( );
  this->GridButton = vtkKWMenuButton::New  ( );
  this->CrossHairButton = vtkKWMenuButton::New ( );
  this->SpatialUnitsButton = vtkKWMenuButton::New ( );
  this->AnnotationButton = vtkKWMenuButton::New ( );
  this->LabelOpacityTopLevel = vtkKWTopLevel::New ( );
  this->LabelOpacityScale = vtkKWScaleWithEntry::New ( );

}


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::~vtkSlicerSlicesControlGUI ( )
{
  if ( this->SlicesControlIcons )
    {
    this->SlicesControlIcons->Delete ( );
    this->SlicesControlIcons = NULL;
    }
  if ( this->SliceFadeScale )
    {
    this->SliceFadeScale->SetParent ( NULL );
    this->SliceFadeScale->Delete ( );
    this->SliceFadeScale = NULL;    
    }
  if ( this->ShowFgButton )
    {
    this->ShowFgButton->SetParent ( NULL );
    this->ShowFgButton->Delete ( );
    this->ShowFgButton = NULL;    
    }
  if ( this->ShowBgButton )
    {
    this->ShowBgButton->SetParent ( NULL );
    this->ShowBgButton->Delete ( );
    this->ShowBgButton = NULL;    
    }
  if ( this->ToggleFgBgButton )
    {
    this->ToggleFgBgButton->SetParent ( NULL );
    this->ToggleFgBgButton->Delete ( );
    this->ToggleFgBgButton = NULL;
    }
  if ( this->LabelOpacityButton )
    {
    this->LabelOpacityButton->SetParent ( NULL );
    this->LabelOpacityButton->Delete ( );
    this->LabelOpacityButton = NULL;    
    }
  if ( this->LinkControlsButton )
    {
    this->LinkControlsButton->SetParent ( NULL );
    this->LinkControlsButton->Delete ( );
    this->LinkControlsButton = NULL;
    }
  if ( this->GridButton )
    {
    this->GridButton->SetParent ( NULL );
    this->GridButton->Delete ( );
    this->GridButton = NULL;    
    }
  if ( this->CrossHairButton )
    {
    this->CrossHairButton->SetParent ( NULL );
    this->CrossHairButton->Delete ( );    
    this->CrossHairButton = NULL;
    }
  if ( this->SpatialUnitsButton )
    {
    this->SpatialUnitsButton->SetParent ( NULL );
    this->SpatialUnitsButton->Delete ( );
    this->SpatialUnitsButton = NULL;    
    }
  if ( this->AnnotationButton )
    {
    this->AnnotationButton->SetParent ( NULL );
    this->AnnotationButton->Delete ( );
    this->AnnotationButton = NULL;    
    }
  if ( this->LabelOpacityTopLevel )
    {
    this->LabelOpacityTopLevel->SetParent ( NULL );
    this->LabelOpacityTopLevel->Delete ( );    
    this->LabelOpacityTopLevel = NULL;
    }
  if ( this->LabelOpacityScale )
    {
    this->LabelOpacityScale->SetParent ( NULL );
    this->LabelOpacityScale->Delete ( );
    this->LabelOpacityScale = NULL;    
    }

    this->SetApplicationGUI ( NULL );
}





//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerSlicesControlGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "SliceFadeScale: " << this->GetSliceFadeScale ( ) << "\n";
    os << indent << "ShowFgButton: " << this->GetShowFgButton ( ) << "\n";
    os << indent << "ShowBgButton: " << this->GetShowBgButton ( ) << "\n";
    os << indent << "ToggleFgBgButton: " << this->GetToggleFgBgButton ( ) << "\n";
    os << indent << "LabelOpacityButton: " << this->GetLabelOpacityButton ( ) << "\n";
    os << indent << "LabelOpacityScale: " << this->GetLabelOpacityScale ( ) << "\n";
    os << indent << "LabelOpacityTopLevel: " << this->GetLabelOpacityTopLevel ( ) << "\n";
    os << indent << "LinkControlsButton: " << this->GetLinkControlsButton ( ) << "\n";
    os << indent << "GridButton: " << this->GetGridButton ( ) << "\n";
    os << indent << "AnnotationButton: " << this->GetAnnotationButton ( ) << "\n";
    os << indent << "SpatialUnitsButton: " << this->GetSpatialUnitsButton ( ) << "\n";
    os << indent << "CrossHairButton: " << this->GetCrossHairButton ( ) << "\n";
    os << indent << "SlicesControlIcons: " << this->GetSlicesControlIcons ( ) << "\n";
    os << indent << "ApplicationGUI: " << this->GetApplicationGUI ( ) << "\n";
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::RemoveGUIObservers ( )
{
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->LabelOpacityScale->GetScale()->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->ToggleFgBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowFgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LinkControlsButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GridButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpatialUnitsButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CrossHairButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::AddGUIObservers ( )
{


  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ToggleFgBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowFgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LinkControlsButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GridButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpatialUnitsButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CrossHairButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{




  vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
  vtkKWScale *scale = vtkKWScale::SafeDownCast (caller);
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast (caller);

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
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->LinkControlsButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
            {
              if (p->GetMRMLScene()) 
                {
                  p->GetMRMLScene()->SaveStateForUndo();
                }
            }

          if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueChangingEvent ||
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
            {

            if ( pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent )
              {
                  this->SliceFadeScale->SetValue( 1.0 );
              }
            if ( pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
              {
                  this->SliceFadeScale->SetValue( 0.0 );
              }
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
          if ( scale == this->LabelOpacityScale->GetScale() && event == vtkKWScale::ScaleValueStartChangingEvent )
            {
              if (p->GetMRMLScene()) 
                {
                  p->GetMRMLScene()->SaveStateForUndo();
                }
            }

          if ( scale == this->LabelOpacityScale->GetScale() && event == vtkKWScale::ScaleValueChangingEvent )
            {
            // adjust the Label opacity value for all slice composite nodes.
              int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
              vtkMRMLSliceCompositeNode *cnode;
              for (i = 0; i < nnodes; i++)
                {
                  cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                   p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                  cnode->SetLabelOpacity( this->LabelOpacityScale->GetValue() );
                }
            }

          if ( pushb == this->LabelOpacityButton && event == vtkKWPushButton::InvokedEvent )
            {
            this->PopUpLabelOpacityScaleAndEntry();
            }
          //
          // PushButtons:
          //
          // link control
/*
          if ( pushb == this->LinkControlsButton && event == vtkKWPushButton::InvokedEvent )
            {
            // Toggle the control link on all slices
              int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
              int link = 1;
              vtkMRMLSliceCompositeNode *cnode;
              for (i = 0; i < nnodes; i++)
                {
                // find out whether any of the slice nodes is unlinked.
                  cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                   p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                  if ( cnode->GetLinkedControl() == 0 )
                    {
                    // means that at least one slice viewer is unlinked ffrom the others.
                    link = 0;
                    }
                }
              for (i = 0; i < nnodes; i++)
                {
                // find out whether any of the slice nodes is unlinked.
                cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                 p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                if ( link )
                  {
                  // unlink all and update the icon here.
                  cnode->SetLinkedControl ( 0 );
                  this->LinkControlsButton->SetImageToIcon (
                                                                 this->GetSlicesControlIcons()->GetLinkControlsIcon() );
                  }
                else
                  {
                  // link all and update the icon here.
                  this->LinkControlsButton->SetImageToIcon (
                                                                 this->GetSlicesControlIcons()->GetUnlinkControlsIcon() );
                  cnode->SetLinkedControl ( 1 );
                  }
                }
            }
*/
          // MenuButtons:
          if ( menu == this->GridButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // save state for undo and then modify MRML
            }

          if ( menu == this->AnnotationButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // save state for undo and then modify MRML
            }
          if ( menu == this->SpatialUnitsButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // save state for undo and then modify MRML
            }
          if ( menu == this->CrossHairButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
            {
            // save state for undo and then modify MRML
            }
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{

}




//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{

/*
  vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
  if ( p )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
    if ( app)
      {
      vtkMRMLSliceCompositeNode *cnode;
      int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
      int link = 1;
      for (i = 0; i < nnodes; i++)
        {
        // find out whether any of the slice nodes is unlinked.
        cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                         p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
        // if any are unlinked, then update the icon to show that
        // there's no link among all.
        if ( cnode->GetLinkedControl() == 0 )
          {
          link = 0;
          }
        }
      if ( link == 0 )
        {
        this->GetLinkControlsButton()->SetImageToIcon (
                                                         this->GetSlicesControlIcons()->GetUnlinkControlsIcon() );
        }
        else
          {
          // if all are linked, then update the icon to
          // show that there's a link among all slice controllers.
          this->GetLinkControlsButton()->SetImageToIcon (
                                                         this->GetSlicesControlIcons()->GetUnlinkControlsIcon() );
          }
      }
    }
*/

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
void vtkSlicerSlicesControlGUI::BuildAnnotationMenu ( )
{
  this->AnnotationButton->GetMenu()->DeleteAllItems ( );
  this->AnnotationButton->GetMenu()->AddRadioButton ( "None");
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show all");
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show label values only" );
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show voxel and label values only" );
  this->AnnotationButton->GetMenu()->SelectItem ( "Show all" );
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildCrossHairMenu ( )
{
  this->CrossHairButton->GetMenu()->DeleteAllItems ( );
  this->CrossHairButton->GetMenu()->AddRadioButton ("No crosshair" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic crosshair" );  
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + intersection" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + hashmarks" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + hashmarks + intersection" );
  this->CrossHairButton->GetMenu()->AddCheckButton ("Follow mouse" );
  this->CrossHairButton->GetMenu()->DeselectItem ( "Follow mouse" );
  this->CrossHairButton->GetMenu()->SelectItem ("No crosshair");
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildGridMenu ( )
{
  this->GridButton->GetMenu()->DeleteAllItems ( );
  this->GridButton->GetMenu()->AddCheckButton ( "Foreground grid");
  this->GridButton->GetMenu()->AddCheckButton ( "Background grid");
  this->GridButton->GetMenu()->AddCheckButton ( "Label grid");
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildSpacesMenu ( )
{
  // what other spaces should we represent here?
  // TODO: this selection should get saved to the registry.
  this->SpatialUnitsButton->GetMenu()->DeleteAllItems ( );
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "xyz");
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "ijk");  
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "RAS");
  this->SpatialUnitsButton->GetMenu()->SelectItem ( "RAS" );
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::HideLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityTopLevel )
    {
    return;
    }
  this->LabelOpacityTopLevel->Withdraw();
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PopUpLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityButton || !this->LabelOpacityButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, py, ph, scx, scy, sx, sy;
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI ( );
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(appGUI->GetApplication() );
  
  vtkKWTkUtilities::GetMousePointerCoordinates(this->LabelOpacityButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->LabelOpacityButton, NULL, &py);
  vtkKWTkUtilities::GetWidgetSize(this->LabelOpacityButton, NULL, &ph);
  vtkKWTkUtilities::GetWidgetRelativeCoordinates(this->LabelOpacityScale->GetScale(), &sx, &sy);
  sscanf(this->Script("%s coords %g", this->LabelOpacityScale->GetScale()->GetWidgetName(),
                      this->LabelOpacityScale->GetScale()->GetValue()), "%d %d", &scx, &scy);
 
  // Place the scale so that the slider is coincident with the x mouse position
  // and just below the push button
  x -= sx + scx;
  if (py <= y && y <= (py + ph -1))
    {
    y = py + ph - 3;
    }
  else
    {
    y -= sy + scy;
    }

  this->LabelOpacityTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->LabelOpacityTopLevel->DeIconify();
  this->LabelOpacityTopLevel->Raise();
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

      vtkKWFrame *FgBgFrame = vtkKWFrame::New ( );
      FgBgFrame->SetParent ( appF);
      FgBgFrame->Create ( );
        
      this->ShowFgButton->SetParent (FgBgFrame);
      this->ShowFgButton->Create ( );
      this->ShowFgButton->SetBorderWidth ( 0 );
      this->ShowFgButton->SetImageToIcon ( this->SlicesControlIcons->GetFgIcon() );
      this->ShowFgButton->SetBalloonHelpString ( "Show Foreground Layer in Slice Viewers." );

      this->ShowBgButton->SetParent (FgBgFrame);
      this->ShowBgButton->Create ( );
      this->ShowBgButton->SetBorderWidth ( 0 );
      this->ShowBgButton->SetImageToIcon ( this->SlicesControlIcons->GetBgIcon() );
      this->ShowBgButton->SetBalloonHelpString ( "Show Background Layer in Slice Viewers." );
    
      this->ToggleFgBgButton->SetParent (FgBgFrame);
      this->ToggleFgBgButton->Create ( );
      this->ToggleFgBgButton->SetBorderWidth ( 0 );
      this->ToggleFgBgButton->SetImageToIcon ( this->SlicesControlIcons->GetToggleFgBgIcon() );
      this->ToggleFgBgButton->SetBalloonHelpString ( "Toggle visibility of Foreground and Background Layers in Slice Viewers." );

      //--- make scale for sliding slice visibility in the SliceViewers
      this->SliceFadeScale->SetParent ( FgBgFrame );
      this->SliceFadeScale->Create ( );
      this->SliceFadeScale->SetRange (0.0, 1.0);
      this->SliceFadeScale->SetResolution ( 0.01 );
      this->SliceFadeScale->SetValue ( 0.0 );
      this->SliceFadeScale->SetLength ( 120 );
      this->SliceFadeScale->SetOrientationToHorizontal ( );
      this->SliceFadeScale->ValueVisibilityOff ( );
      this->SliceFadeScale->SetBalloonHelpString ( "Scale fades between Foreground and Background Layers." );

      this->LinkControlsButton->SetParent (appF);
      this->LinkControlsButton->Create ( );
      this->LinkControlsButton->SetBorderWidth ( 0 );
      this->LinkControlsButton->SetImageToIcon ( this->SlicesControlIcons->GetLinkControlsIcon() );
      this->LinkControlsButton->SetBalloonHelpString ( "Links/Unlinks the FG/BG/LB/OR controls across all Slice Viewers." );

      this->GridButton->SetParent (appF);
      this->GridButton->Create ( );
      this->GridButton->SetBorderWidth ( 0 );
      this->GridButton->SetImageToIcon ( this->SlicesControlIcons->GetGridIcon() );
      this->GridButton->IndicatorVisibilityOff ( );
      this->GridButton->SetBalloonHelpString ( "Toggle grid visiblity in each Slice Layer for all Slice Viewers." );

      this->AnnotationButton->SetParent (appF);
      this->AnnotationButton->Create ( );
      this->AnnotationButton->SetBorderWidth ( 0 );
      this->AnnotationButton->SetImageToIcon ( this->SlicesControlIcons->GetAnnotationIcon() );
      this->AnnotationButton->IndicatorVisibilityOff ( );
      this->AnnotationButton->SetBalloonHelpString ( "Choose annotation options for all Slice Viewers." );

      this->SpatialUnitsButton->SetParent (appF);
      this->SpatialUnitsButton->Create ( );
      this->SpatialUnitsButton->SetBorderWidth ( 0 );
      this->SpatialUnitsButton->SetImageToIcon ( this->SlicesControlIcons->GetSpatialUnitsIcon() );
      this->SpatialUnitsButton->IndicatorVisibilityOff ( );
      this->SpatialUnitsButton->SetBalloonHelpString ( "Assign axis labels in all Slice Viewers." );

      this->CrossHairButton->SetParent (appF);
      this->CrossHairButton->Create ( );
      this->CrossHairButton->SetBorderWidth ( 0 );
      this->CrossHairButton->SetImageToIcon ( this->SlicesControlIcons->GetCrossHairIcon() );
      this->CrossHairButton->IndicatorVisibilityOff ( );
      this->CrossHairButton->SetBalloonHelpString ( "Choose among crosshair options for all Slice Viewers." );

      //--- Popup Scale with Entry (displayed when user clicks LabelOpacityButton
      //--- LabelOpacityButton, LabelOpacityScale and its entry will be observed
      //--- and their events handled in ProcessGUIEvents;
      //--- the pop-up and hide behavior of the latter two will be managed locally
      //--- in the GUI.
      //--- TODO: make a SlicerWidget that handles this behavior. Leave event?
      this->LabelOpacityTopLevel->SetApplication ( this->GetApplication ( ) );
      this->LabelOpacityTopLevel->SetMasterWindow ( this->LabelOpacityButton );
      this->LabelOpacityTopLevel->Create ( );
      this->LabelOpacityTopLevel->HideDecorationOn ( );
      this->LabelOpacityTopLevel->Withdraw ( );
      this->LabelOpacityTopLevel->SetBorderWidth ( 2 );
      this->LabelOpacityTopLevel->SetReliefToGroove ( );
      //--- create frame
      vtkKWFrame *PopUpFrame = vtkKWFrame::New ( );
      PopUpFrame->SetParent ( this->LabelOpacityTopLevel );
      PopUpFrame->Create ( );
      PopUpFrame->SetBinding ( "<Leave>", this, "HideLabelOpacityScaleAndEntry" );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", PopUpFrame->GetWidgetName ( ) );   
      // Scale and entry packed in the pop-up toplevel's frame
      this->LabelOpacityScale->SetParent ( PopUpFrame );
      this->LabelOpacityScale->Create ( );
      this->LabelOpacityScale->SetRange ( 0.0, 1.0 );
      this->LabelOpacityScale->SetResolution ( 0.01 );
      this->LabelOpacityScale->GetScale()->SetLabelText ( "" );
      this->LabelOpacityScale->GetScale()->ValueVisibilityOff ( );
      this->LabelOpacityScale->SetValue ( 1.0 );
      this->Script ( "pack %s -side left -anchor w -padx 1 -pady 3 -expand n", this->LabelOpacityScale->GetWidgetName ( ) );
      this->LabelOpacityButton->SetParent (appF);
      this->LabelOpacityButton->Create ( );
      this->LabelOpacityButton->SetBorderWidth ( 0 );
      this->LabelOpacityButton->SetImageToIcon ( this->SlicesControlIcons->GetLabelOpacityIcon() );
      this->LabelOpacityButton->SetBalloonHelpString ( "Popup scale to adjust opacity of Label Layer in all Slice Viewers." );

      //--- pack everything up from left to right.
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->LabelOpacityButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->GridButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->AnnotationButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->SpatialUnitsButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->CrossHairButton->GetWidgetName ( ) );
//      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->LinkControlsButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -ipadx 35 -pady 1 -fill x -expand n", FgBgFrame->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 0 -pady 3 -expand n", this->ShowFgButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 0 -pady 3 -expand n", this->SliceFadeScale->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 0 -pady 3 -expand n", this->ShowBgButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 2 -pady 3 -expand n", this->ToggleFgBgButton->GetWidgetName ( ) );

      //--- populate menus
      this->BuildAnnotationMenu ( );
      this->BuildCrossHairMenu ( );
      this->BuildSpacesMenu ( );
      this->BuildGridMenu ();
      PopUpFrame->Delete ( );
      FgBgFrame->Delete ( );
      }
    }

}





