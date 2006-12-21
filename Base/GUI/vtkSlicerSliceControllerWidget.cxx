#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkImageData.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"

#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWIcon.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceControllerWidget );
vtkCxxRevisionMacro ( vtkSlicerSliceControllerWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceControllerWidget::vtkSlicerSliceControllerWidget ( ) {

  //---  
  // widgets comprising the SliceControllerWidget for now.
  this->OffsetScale = NULL;
  this->OrientationMenu = NULL;
  this->ForegroundSelector = NULL;
  this->BackgroundSelector = NULL;
  this->LabelSelector = NULL;
  this->VisibilityToggle = NULL;
  this->LabelOpacityButton = NULL;
  this->LabelOpacityScale = NULL;
  this->LabelOpacityTopLevel = NULL;
  this->LinkButton = NULL;
  this->VisibilityIcons = NULL;
  this->SliceNode = NULL;
  this->SliceCompositeNode = NULL;
  this->SliceLogic = NULL;
  this->ScaleFrame = NULL;
  this->ColorCodeButton = NULL;
  this->SliceControlIcons = NULL;
  this->ContainerFrame = NULL;
  this->FitToWindowButton = NULL;
  this->VolumeDisplayMenuButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceControllerWidget::~vtkSlicerSliceControllerWidget ( ){

  if ( this->FitToWindowButton )
    {
    this->FitToWindowButton->SetParent ( NULL );
    this->FitToWindowButton->Delete( );
    this->FitToWindowButton = NULL;
    }
  if ( this->OffsetScale )
    {
    this->OffsetScale->SetParent(NULL);
    this->OffsetScale->Delete ( );
    this->OffsetScale = NULL;
    }
  if ( this->OrientationMenu )
    {
    this->OrientationMenu->SetParent(NULL);
    this->OrientationMenu->Delete ( );
    this->OrientationMenu = NULL;
    }
  if ( this->ForegroundSelector )
    {
    this->ForegroundSelector->SetParent(NULL);
    this->ForegroundSelector->Delete ( );
    this->ForegroundSelector = NULL;
    }
  if ( this->BackgroundSelector )
    {
    this->BackgroundSelector->SetParent(NULL);
    this->BackgroundSelector->Delete ( );
    this->BackgroundSelector = NULL;
    }
  if ( this->LabelSelector )
    {
    this->LabelSelector->SetParent(NULL);
    this->LabelSelector->Delete ( );
    this->LabelSelector = NULL;
    }
  if ( this->VolumeDisplayMenuButton)
    {
    this->VolumeDisplayMenuButton->SetParent(NULL);
    this->VolumeDisplayMenuButton->Delete  ( );
    this->VolumeDisplayMenuButton = NULL;
    }
  if ( this->VisibilityToggle )
    {
    this->VisibilityToggle->SetParent(NULL);
    this->VisibilityToggle->Delete  ( );
    this->VisibilityToggle = NULL;
    }
  if ( this->LabelOpacityButton )
    {
    this->LabelOpacityButton->SetParent(NULL);
    this->LabelOpacityButton->Delete  ( );
    this->LabelOpacityButton = NULL;
    }
  if ( this->LabelOpacityScale )
    {
    this->LabelOpacityScale->SetParent(NULL);
    this->LabelOpacityScale->Delete  ( );
    this->LabelOpacityScale = NULL;
    }
  if ( this->LabelOpacityTopLevel )
    {
    this->LabelOpacityTopLevel->SetParent(NULL);
    this->LabelOpacityTopLevel->Delete  ( );
    this->LabelOpacityTopLevel = NULL;
    }
  if ( this->LinkButton )
    {
    this->LinkButton->SetParent(NULL);
    this->LinkButton->Delete  ( );
    this->LinkButton = NULL;
    }
  if ( this->VisibilityIcons )
    {
    this->VisibilityIcons->Delete  ( );
    this->VisibilityIcons = NULL;
    }
  if ( this->SliceControlIcons )
    {
    this->SliceControlIcons->Delete  ( );
    this->SliceControlIcons = NULL;
    }
  if ( this->ScaleFrame )
    {
    this->ScaleFrame->SetParent(NULL);
    this->ScaleFrame->Delete ( );
    this->ScaleFrame = NULL;
    }
  if ( this->ColorCodeButton )
    {
    this->ColorCodeButton->SetParent(NULL);
    this->ColorCodeButton->Delete ( );
    this->ColorCodeButton = NULL;
    }
  if ( this->ContainerFrame )
    {
    this->ContainerFrame->SetParent(NULL);
    this->ContainerFrame->Delete ( );
    this->ContainerFrame = NULL;
    }

  this->SetSliceNode ( NULL );
  this->SetSliceCompositeNode ( NULL );
  this->SetSliceLogic ( NULL );
}




//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::AddWidgetObservers ( )
{
  if ( this->OffsetScale == NULL ) 
    {
    vtkErrorMacro ("Can't add observers because CreateWidget hasn't been called");
    return;
    }

    this->OrientationMenu->GetWidget()->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
    this->ForegroundSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->BackgroundSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->LabelSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->OffsetScale->GetWidget()->AddObserver( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->AddObserver( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->AddObserver( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
    this->VisibilityToggle->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->LabelOpacityButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
    this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
    this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );    
    this->LinkButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->FitToWindowButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->VolumeDisplayMenuButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );    

}
  

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::RemoveWidgetObservers ( ) {

  if ( this->OffsetScale == NULL ) 
    {
    vtkErrorMacro ("Can't remove observers because CreateWidget hasn't been called");
    return;
    }

    this->OrientationMenu->GetWidget()->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
    this->ForegroundSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->BackgroundSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->LabelSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
    this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
    this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
    this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );    
    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
    this->VisibilityToggle->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->LabelOpacityButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->LinkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );        
    this->FitToWindowButton->RemoveObservers (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
    this->VolumeDisplayMenuButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );    
}



//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ApplyColorCode ( double *c )
{
  this->ColorCodeButton->SetBackgroundColor (c[0], c[1], c[2] );
}




//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::CreateWidget ( ) 
{

    if ( !this->MRMLScene ) {
        vtkErrorMacro ( << " MRML Scene must be set before creating widgets.");
        return;
    }

    // the widget is a frame with some widgets inside
    if (this->IsCreated ( ) ) {
        vtkErrorMacro ( << this->GetClassName() << "already created.");
        return;
        
    }
    this->Superclass::CreateWidget ( );
    this->SliceControlIcons = vtkSlicerSlicesControlIcons::New ( );
    //
    // A stripe that color codes the SliceGUI this controller belongs to.
    //

    this->ColorCodeButton = vtkKWPushButton::New ( );
    this->ColorCodeButton->SetParent ( this );
    this->ColorCodeButton->Create ( );
    this->ColorCodeButton->SetBorderWidth (0 );
    this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinDown );
    this->ColorCodeButton->SetHeight (7 );
    this->ColorCodeButton->SetCommand (this, "Shrink");
    this->ColorCodeButton->SetBalloonHelpString ("Click to shrink/expand" );

    this->ContainerFrame = vtkKWFrame::New ( );
    this->ContainerFrame->SetParent ( this );
    this->ContainerFrame->Create ( );

    //
    // Orientation  (TODO: make this into a vtkSlicerOrientationWidget)
    //
    this->OrientationMenu = vtkKWMenuButtonWithSpinButtonsWithLabel::New ();
    this->OrientationMenu->SetParent ( this->ContainerFrame );
    this->OrientationMenu->Create ( );    
    this->OrientationMenu->GetWidget()->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-9-*-*-*-*-*-*-*" );
    this->OrientationMenu->GetLabel()->SetImageToIcon ( this->SliceControlIcons->GetSetOrIcon() );
    this->OrientationMenu->GetLabel()->SetBalloonHelpString ("Select orientation" );
    vtkKWMenuButton *mb = this->OrientationMenu->GetWidget()->GetWidget();
    mb->SetWidth ( 12 );
    mb->GetMenu()->AddRadioButton ( "Axial" );
    mb->GetMenu()->AddRadioButton ( "Sagittal" );
    mb->GetMenu()->AddRadioButton ( "Coronal" );
    mb->SetValue ("Axial");    

    //
    // Foreground, Background, and Label selections
    //
    this->ForegroundSelector = vtkSlicerNodeSelectorWidget::New();
   this->ForegroundSelector->SetParent ( this->ContainerFrame );
    this->ForegroundSelector->Create ( );
    this->ForegroundSelector->NoneEnabledOn();
    this->ForegroundSelector->GetWidget()->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-9-*-*-*-*-*-*-*" );
    this->ForegroundSelector->GetLabel()->SetImageToIcon ( this->SliceControlIcons->GetSetFgIcon() );
    this->ForegroundSelector->GetLabel()->SetBalloonHelpString ( "Select the foreground");
    this->ForegroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->ForegroundSelector->SetMRMLScene( this->MRMLScene );
    this->ForegroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
    this->ForegroundSelector->GetWidget()->GetWidget()->SetWidth(12);

    this->BackgroundSelector = vtkSlicerNodeSelectorWidget::New();
    this->BackgroundSelector->SetParent ( this->ContainerFrame );
    this->BackgroundSelector->Create ( );
    this->BackgroundSelector->NoneEnabledOn();
    this->BackgroundSelector->GetWidget()->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-9-*-*-*-*-*-*-*" );
    this->BackgroundSelector->GetLabel()->SetImageToIcon ( this->SliceControlIcons->GetSetBgIcon ( ) );
    this->BackgroundSelector->GetLabel()->SetBalloonHelpString ( "Select the background");
    this->BackgroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->BackgroundSelector->SetMRMLScene( this->MRMLScene );
    this->BackgroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
    this->BackgroundSelector->GetWidget()->GetWidget()->SetWidth(12);

    this->LabelSelector = vtkSlicerNodeSelectorWidget::New();
    this->LabelSelector->SetParent ( this->ContainerFrame );
    this->LabelSelector->Create ( );
    this->LabelSelector->NoneEnabledOn();
    this->LabelSelector->GetWidget()->GetWidget()->SetFont ( "-Adobe-Helvetica-Bold-R-Normal-*-9-*-*-*-*-*-*-*" );
    this->LabelSelector->GetLabel()->SetImageToIcon ( this->SliceControlIcons->GetSetLbIcon ( ) );
    this->LabelSelector->GetLabel()->SetBalloonHelpString ( "Select the label map");
    this->LabelSelector->SetNodeClass ("vtkMRMLVolumeNode", "LabelMap", "1", NULL);
    this->LabelSelector->SetMRMLScene( this->MRMLScene );
    this->LabelSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
    this->LabelSelector->GetWidget()->GetWidget()->SetWidth(12);

    //
    // Create the frame to contain scale and visibility toggle
    //
    this->ScaleFrame = vtkKWFrame::New ();
    this->ScaleFrame->SetParent ( this->ContainerFrame );
    this->ScaleFrame->Create ( );

    //
    // Create a button to toggle the slice visibility in the main viewer and icons for it
    //
    this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );
    this->VisibilityToggle = vtkKWPushButton::New ( );
    this->VisibilityToggle->SetParent ( this->ScaleFrame );
    this->VisibilityToggle->Create ( );
    this->VisibilityToggle->SetReliefToFlat ( );
    this->VisibilityToggle->SetOverReliefToNone ( );
    this->VisibilityToggle->SetBorderWidth ( 0 );
    this->VisibilityToggle->SetImageToIcon ( this->VisibilityIcons->GetInvisibleIcon ( ) );        
    this->VisibilityToggle->SetBalloonHelpString ( "Toggles slice visibility in the MainViewer." );

    //
    // Create a button to toggle the slice visibility in the main viewer and icons for it
    //
    this->LinkButton = vtkKWPushButton::New ( );
    this->LinkButton->SetParent ( this->ScaleFrame );
    this->LinkButton->Create ( );
    this->LinkButton->SetReliefToFlat ( );
    this->LinkButton->SetOverReliefToNone ( );
    this->LinkButton->SetBorderWidth ( 0 );
    this->LinkButton->SetImageToIcon ( this->SliceControlIcons->GetUnlinkControlsIcon ( ) );        
    this->LinkButton->SetBalloonHelpString ( "Links/Unlinks the slice controls (except scales) across all Slice Viewers." );

    this->FitToWindowButton = vtkKWPushButton::New ( );
    this->FitToWindowButton->SetParent ( this->ScaleFrame );
    this->FitToWindowButton->Create ( );
    this->FitToWindowButton->SetReliefToFlat ( );
    this->FitToWindowButton->SetOverReliefToNone ( );
    this->FitToWindowButton->SetBorderWidth ( 0 );
    this->FitToWindowButton->SetImageToIcon ( this->SliceControlIcons->GetFitToWindowIcon ( ));    
    this->FitToWindowButton->SetBalloonHelpString ( "Fits the image data to the Slice Viewer's window.");

    this->VolumeDisplayMenuButton = vtkKWMenuButton::New ( );
    this->VolumeDisplayMenuButton->SetParent ( this->ScaleFrame );
    this->VolumeDisplayMenuButton->Create ( );
    this->VolumeDisplayMenuButton->SetBorderWidth ( 0 );
    this->VolumeDisplayMenuButton->IndicatorVisibilityOff ( );
    this->VolumeDisplayMenuButton->SetImageToIcon ( this->SliceControlIcons->GetWinLevThreshColIcon ( ));    
    this->VolumeDisplayMenuButton->SetBalloonHelpString ( "Adjust window, level, threshold and color palette for a Slice Layer.");
    this->VolumeDisplayMenuButton->GetMenu()->AddRadioButton ( "Foreground volume" );
    this->VolumeDisplayMenuButton->GetMenu()->AddRadioButton ( "Background volume" );
    this->VolumeDisplayMenuButton->GetMenu()->AddRadioButton ( "Label map" );
    this->VolumeDisplayMenuButton->GetMenu()->AddSeparator();
    this->VolumeDisplayMenuButton->GetMenu()->AddCommand ( "close" );    

    //--- Popup Scale with Entry (displayed when user clicks LabelOpacityButton
    //--- LabelOpacityButton, LabelOpacityScale and its entry will be observed
    //--- and their events handled in ProcessGUIEvents;
    //--- the pop-up and hide behavior of the latter two will be managed locally
    //--- in the GUI.
    //--- TODO: make a SlicerWidget that handles this behavior. Leave event?
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
    this->LabelOpacityTopLevel = vtkKWTopLevel::New ( );
    this->LabelOpacityTopLevel->SetApplication ( app );
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
    this->LabelOpacityScale = vtkKWScaleWithEntry::New ( );
    this->LabelOpacityScale->SetParent ( PopUpFrame );
    this->LabelOpacityScale->Create ( );
    this->LabelOpacityScale->SetRange ( 0.0, 1.0 );
    this->LabelOpacityScale->SetResolution ( 0.01 );
    this->LabelOpacityScale->GetScale()->SetLabelText ( "" );
    this->LabelOpacityScale->GetScale()->ValueVisibilityOff ( );
    this->LabelOpacityScale->SetValue ( 1.0 );
    this->Script ( "pack %s -side left -anchor w -padx 1 -pady 3 -expand n", this->LabelOpacityScale->GetWidgetName ( ) );
    this->LabelOpacityButton = vtkKWPushButton::New ( );
    this->LabelOpacityButton->SetParent (this->ScaleFrame );
    this->LabelOpacityButton->Create ( );
    this->LabelOpacityButton->SetBorderWidth ( 0 );
    this->LabelOpacityButton->SetImageToIcon ( this->SliceControlIcons->GetLabelOpacityIcon() );
    this->LabelOpacityButton->SetBalloonHelpString ( "Popup scale to adjust opacity of Label Layer." );
    PopUpFrame->Delete ( );


    //
    // Create a scale to control the slice number displayed
    //
    this->OffsetScale = vtkKWScaleWithEntry::New();
    this->OffsetScale->SetParent ( this->ScaleFrame );
    this->OffsetScale->Create();
    this->OffsetScale->RangeVisibilityOff ( );
    this->OffsetScale->SetEntryWidth(8);
    this->OffsetScale->SetLabelPositionToLeft();
            
    this->Script ( "pack %s -side top -expand 1 -fill x", 
                   this->ColorCodeButton->GetWidgetName ( ));
    this->Script ("pack %s -side bottom -expand 1 -fill x", 
                  this->ContainerFrame->GetWidgetName());

    this->Script("grid columnconfigure %s 0 -weight 1", 
                 this->ContainerFrame->GetWidgetName());
    this->Script("grid columnconfigure %s 1 -weight 1", 
                 this->ContainerFrame->GetWidgetName());
    

    this->Script("grid %s %s -sticky ew", 
                 this->OrientationMenu->GetWidgetName(), 
                 this->ForegroundSelector->GetWidgetName());
    this->Script("grid %s %s -sticky ew", 
                 this->LabelSelector->GetWidgetName(), 
                 this->BackgroundSelector->GetWidgetName());
    this->Script ( "grid %s -sticky ew -columnspan 2", 
                   this->ScaleFrame->GetWidgetName ( ) );

    this->Script ("pack %s -side left -expand n -padx 1", 
                  this->LinkButton->GetWidgetName ( ) );
    this->Script ("pack %s -side left -expand n -padx 1", 
                  this->VisibilityToggle->GetWidgetName ( ) );
    this->Script ("pack %s -side left -expand n -padx 1", 
                  this->FitToWindowButton->GetWidgetName ( ) );
    this->Script ("pack %s -side left -expand n -padx 1", 
                  this->LabelOpacityButton->GetWidgetName ( ) );
    this->Script ("pack %s -side left -expand n -padx 1", 
                  this->VolumeDisplayMenuButton->GetWidgetName ( ) );    
    this->Script("pack %s -side left -fill x -expand y", 
                 this->OffsetScale->GetWidgetName());

    // put observers on widgets
    this->AddWidgetObservers();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData ) 
{ 
  //
  // Is this Slice controller linked to others?
  //
  int link, i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  vtkMRMLSliceNode *snode;
  
  if ( this->SliceCompositeNode )
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }
  
  //
  // Get a route to all SliceGUI's SliceNodes and SliceCompositeNodes in case of link
  //
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  vtkSlicerSlicesGUI *sgui;
  if (app)
    {
    sgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName("Slices"));
    }
  else
    {
    sgui = NULL;
    }

   if (this->SliceNode != NULL && this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    this->SetSliceNode(NULL);
    }
  if (this->SliceCompositeNode != NULL && this->MRMLScene->GetNodeByID(this->SliceCompositeNode->GetID()) == NULL)
    {
    this->SetSliceCompositeNode(NULL);
    }

  //
  // Update orientation if needed
  //
  if ( this->SliceNode != NULL && 
       vtkKWMenu::SafeDownCast(caller) == this->OrientationMenu->GetWidget()->GetWidget()->GetMenu() )
    {
    //--- if slice viewers are linked, modify all Controller's SliceNodes.
      vtkKWMenuButton *mb = this->OrientationMenu->GetWidget()->GetWidget();
      if ( !strcmp (mb->GetValue(), "Axial") )   
        {
        if ( link && sgui )
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");
          for ( i=0; i<nnodes; i++)
            {
            snode = vtkMRMLSliceNode::SafeDownCast (
                                                             this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
            snode->SetOrientationToAxial();
            }
          }
        else
          {
          this->SliceNode->SetOrientationToAxial();
          }
        }
      if ( !strcmp (mb->GetValue(), "Sagittal") )   
        {
        if ( link && sgui )
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
          for ( i=0; i<nnodes; i++)
            {
            snode = vtkMRMLSliceNode::SafeDownCast (
                                                             this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
            snode->SetOrientationToSagittal();
            }
          }
        else
          {
          this->SliceNode->SetOrientationToSagittal();
          }
        }
      if ( !strcmp (mb->GetValue(), "Coronal") )   
        {
        if ( link && sgui )
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
          for ( i=0; i<nnodes; i++)
            {
            snode = vtkMRMLSliceNode::SafeDownCast (
                                                             this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
            snode->SetOrientationToCoronal();
            }
          }
        else
          {
          this->SliceNode->SetOrientationToCoronal();
          }
        }
    }

  //
  // Update Foreground if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->ForegroundSelector )
    {
    //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes.
    if ( link && sgui )
      {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                             this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            if ( cnode != NULL && this->ForegroundSelector->GetSelected() != NULL )
              {
              cnode->SetForegroundVolumeID( this->ForegroundSelector->GetSelected()->GetID() );
              }
            else if ( cnode != NULL )
              {
              cnode->SetForegroundVolumeID ( NULL );
              }
            }
      }
    else
      {
      if  (this->ForegroundSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetForegroundVolumeID( this->ForegroundSelector->GetSelected()->GetID() );
        } 
      else if (this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetForegroundVolumeID( NULL );
        }
      }
    }

  //
  // Update Background if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->BackgroundSelector )
    {
    //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes.
    if ( link && sgui )
      {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                    this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            if ( cnode != NULL && this->BackgroundSelector->GetSelected() != NULL)
              {
              cnode->SetBackgroundVolumeID( this->BackgroundSelector->GetSelected()->GetID() );
              }
            else if ( cnode != NULL )
              {
              cnode->SetBackgroundVolumeID ( NULL );
              }
            }
      }
    else
      {
      if  (this->BackgroundSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetBackgroundVolumeID( this->BackgroundSelector->GetSelected()->GetID() );
        } 
      else if (this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetBackgroundVolumeID( NULL );
        }
      }
    }

  
  //
  // Update Label layer if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->LabelSelector )
    {
    //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes.
    if ( link && sgui )
      {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                    this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            if ( cnode != NULL && this->LabelSelector->GetSelected() != NULL )
              {
              cnode->SetLabelVolumeID( this->LabelSelector->GetSelected()->GetID() );
              }
            else if ( cnode != NULL )
              {
              cnode->SetLabelVolumeID ( NULL );
              }
            }
      }
    else
      {
      if  (this->LabelSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetLabelVolumeID( this->LabelSelector->GetSelected()->GetID() );
        } 
      else if (this->SliceCompositeNode != NULL)
        {
        this->SliceCompositeNode->SetLabelVolumeID( NULL );
        }
      }
    }
  
  if ( !this->SliceNode)
    {
    return;
    }
  
  //
  // Was event invoked by other widgets? (button, scale, entry)?
  //
  vtkKWPushButton *toggle = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWScale *scale = vtkKWScale::SafeDownCast (caller);
  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);
  // Toggle the SliceNode's visibility.
  if ( toggle == this->GetVisibilityToggle() &&
       event == vtkKWPushButton::InvokedEvent )
    {
    //--- if slice viewers are linked, modify all Controller's SliceNodes
    int vis = this->SliceNode->GetSliceVisible();
    if ( link )
      {
      nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
      for ( i=0; i<nnodes; i++)
        {
        snode = vtkMRMLSliceNode::SafeDownCast (
                                                this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
        this->MRMLScene->SaveStateForUndo ( snode );
        snode->SetSliceVisible ( !vis );
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      this->SliceNode->SetSliceVisible ( !vis );
      }
    }

  //
  // Toggle the Linked control or pop up opacity scale.
  //
  if ( toggle == this->GetLinkButton() && event == vtkKWPushButton::InvokedEvent )
    {
    this->MRMLScene->SaveStateForUndo ( this->SliceNode );
    this->ToggleSlicesLink ( );
    }
  else if ( toggle == this->LabelOpacityButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->PopUpLabelOpacityScaleAndEntry();
    }

  //
  // Scales starting to move? save state for undo.
  //
  if ( this->OffsetScale->GetWidget() == vtkKWScale::SafeDownCast( caller ) &&
          event == vtkKWScale::ScaleValueStartChangingEvent )
    {
    // set an undo state when the scale starts being dragged
    this->MRMLScene->SaveStateForUndo( this->SliceNode );
    }
  else if ( scale == this->LabelOpacityScale->GetWidget() && event == vtkKWScale::ScaleValueStartChangingEvent )
    {
    if ( link && sgui ) 
      {
      nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
      for ( i=0; i<nnodes; i++)
        {
        // set an undo state when the scale starts being dragged
        cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
        this->MRMLScene->SaveStateForUndo( cnode );
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceCompositeNode );
      }
    }

  //
  // Scales are moving? modify MRML
  //
  int modified = 0;
  if ( (double) this->LabelOpacityScale->GetValue() != this->SliceCompositeNode->GetLabelOpacity() )
    {
    //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes.
    if ( link && sgui )
      {
      nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
      for ( i=0; i<nnodes; i++)
        {
        cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
        cnode->SetLabelOpacity ( (double) this->LabelOpacityScale->GetValue() );
        }
      }
    else
      {
      this->SliceCompositeNode->SetLabelOpacity ( (double) this->LabelOpacityScale->GetValue() );
//      modified = 1;
      }
    }
  if ( (double) this->OffsetScale->GetValue() != this->SliceLogic->GetSliceOffset() )
    {
    this->SliceLogic->SetSliceOffset( (double) this->OffsetScale->GetValue() );
    modified = 1;
    }

  if ( modified )
    {
    this->Modified();
    }

}

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::HideLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityTopLevel )
    {
    return;
    }
  this->LabelOpacityTopLevel->Withdraw();
}


//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PopUpLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityButton || !this->LabelOpacityButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, py, ph, scx, scy, sx, sy;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  
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

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::LinkAllSlices  ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  vtkSlicerSlicesGUI *sgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName("Slices"));
  vtkMRMLSliceCompositeNode *cnode;
  if ( app && sgui )
    {
    // link all slice controllers
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
    for ( int i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      cnode->SetLinkedControl ( 1 );
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UnlinkAllSlices  ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  vtkSlicerSlicesGUI *sgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName("Slices"));
  vtkMRMLSliceCompositeNode *cnode;
  if ( app && sgui )
    {
    // unlink all slice controllers.
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
    for ( int i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      cnode->SetLinkedControl ( 0 );
      }
    }
}

//----------------------------------------------------------------------------
int vtkSlicerSliceControllerWidget::AllSlicesLinked ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  vtkSlicerSlicesGUI *sgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName("Slices"));
  vtkMRMLSliceCompositeNode *cnode;
  int link = 1;
  if ( app && sgui )
    {
    // are all slice controllers linked? assume they are
    // unless we find one that's unliked.
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
    for ( int i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                       this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      if ( cnode->GetLinkedControl ( ) == 0 )
        {
        link = 0;
        }
      }
    }
  return ( link );
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ToggleSlicesLink  ( )
{
  // check to see what the current link status is.
  int link = this->SliceCompositeNode->GetLinkedControl();

  if ( link==1 )
    {
    this->UnlinkAllSlices();
    }
  else
    {
    // slices are currently unlinked, so link them all.
    this->LinkAllSlices();
    }
}




//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) 
{ 
  if (this->SliceNode != NULL && this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    this->SetSliceNode(NULL);
    }

  if (this->SliceCompositeNode != NULL && this->MRMLScene->GetNodeByID(this->SliceCompositeNode->GetID()) == NULL)
    {
    this->SetSliceCompositeNode(NULL);
    }

  if ( !this->SliceNode)
    {
    return;
    }
   if ( !this->SliceCompositeNode)
     {
     return;
     }
  
  int modified = 0;


  // 
  // Update the menu to match the node
  //
  vtkKWMenuButton *mb = this->OrientationMenu->GetWidget()->GetWidget();
  mb->SetValue( this->SliceNode->GetOrientationString() );

  //
  // Set the scale range to match the field of view
  //
  double fovover2 = this->SliceNode->GetFieldOfView()[2] / 2.;
  double min, max;
  this->OffsetScale->GetRange(min, max);
  if ( min != -fovover2 || max != fovover2 )
    {
    this->OffsetScale->SetRange(-fovover2, fovover2);
    modified = 1;
    }


  //
  // Update the VisibilityButton in the SliceController to match the logic state
  //
  if ( this->SliceNode->GetSliceVisible() > 0 ) 
      {
      this->GetVisibilityToggle()->SetImageToIcon ( 
            this->GetVisibilityIcons()->GetVisibleIcon ( ) );        
      } 
  else 
      {
      this->GetVisibilityToggle()->SetImageToIcon ( 
            this->GetVisibilityIcons()->GetInvisibleIcon ( ) );        
      }

  //
  // Update the Linked Controls Icon in the SliceController to match logic state.
  //
  if ( this->SliceCompositeNode != NULL && this->SliceCompositeNode->GetLinkedControl() > 0 )
    {
    this->GetLinkButton()->SetImageToIcon (
            this->GetSliceControlIcons()->GetLinkControlsIcon() );
    }
  else
    {
    this->GetLinkButton()->SetImageToIcon (
            this->GetSliceControlIcons()->GetUnlinkControlsIcon() );
    }
  
  //
  // Set the opacity value to match the value
  //
  if ( this->SliceCompositeNode != NULL && (double) this->LabelOpacityScale->GetValue() != this->SliceCompositeNode->GetLabelOpacity() )
    {
    this->LabelOpacityScale->SetValue ( this->SliceCompositeNode->GetLabelOpacity() );
    }


  //
  // Set the scale and entry widgets' value to match the offset
  //
  if ( (double) this->OffsetScale->GetValue() != this->SliceLogic->GetSliceOffset() )
    {
    this->OffsetScale->SetValue( this->SliceLogic->GetSliceOffset() );
    }


  //
  // when the composite node changes, update the menus to match
  //
  if ( caller == this->SliceCompositeNode )
    {
    vtkMRMLNode *node = this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetForegroundVolumeID() );
    if ( node )
      {
      this->ForegroundSelector->SetSelected(node);
      }
    else
      {
      this->ForegroundSelector->GetWidget()->GetWidget()->GetMenu()->SelectItem("None");
      }

    node = this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetBackgroundVolumeID() );
    if ( node )
      {
      this->BackgroundSelector->SetSelected(node);
      }
    else
      {
      this->BackgroundSelector->GetWidget()->GetWidget()->GetMenu()->SelectItem("None");
      }    

    node = this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetLabelVolumeID() );
    if ( node )
      {
      this->LabelSelector->SetSelected(node);
      }
    else
      {
      this->LabelSelector->GetWidget()->GetWidget()->GetMenu()->SelectItem("None");
      }
    }

  //
  //  Trigger events if needed
  //
  if ( modified )
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::Shrink() 
{ 
  if (this->ContainerFrame && this->ContainerFrame->IsPacked())
    {
    if (this->ColorCodeButton)
      {
      this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinUp );
      this->ColorCodeButton->SetCommand (this, "Expand");
      }
    this->Script ("pack forget %s", 
                  this->ContainerFrame->GetWidgetName());
    this->InvokeEvent(vtkSlicerSliceControllerWidget::ShrinkEvent, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::Expand() 
{ 
  if (this->ContainerFrame && !this->ContainerFrame->IsPacked())
    {
    if (this->ColorCodeButton)
      {
      this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinDown );
      this->ColorCodeButton->SetCommand (this, "Shrink");
      }
    this->Script ("pack %s -side bottom -expand 1 -fill x", 
                  this->ContainerFrame->GetWidgetName());
    this->InvokeEvent(vtkSlicerSliceControllerWidget::ExpandEvent, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

