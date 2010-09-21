#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkImageData.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
//#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerGUILayout.h"

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
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"

#include "vtkRenderer.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceControllerWidget );
vtkCxxRevisionMacro ( vtkSlicerSliceControllerWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerSliceControllerWidget::vtkSlicerSliceControllerWidget ( ) {

  //---  
  // widgets comprising the SliceControllerWidget for now.
  this->MoreMenuButton = NULL;
  this->OffsetScale = NULL;
  this->OffsetScaleMin = 0.;
  this->OffsetScaleResolution = 1.;
  this->OffsetEntry = NULL;
  this->SliceIndexEntry = NULL;
  this->OrientationSelector = NULL;
  this->ForegroundSelector = NULL;
  this->BackgroundSelector = NULL;
  this->LabelSelector = NULL;
  this->OrientationMenuButton  = NULL;
  this->ForegroundMenuButton = NULL;
  this->BackgroundMenuButton = NULL;
  this->LabelMenuButton = NULL;
  this->VisibilityToggle = NULL;
  this->LoadDataButton = NULL;
  this->LabelOpacityButton = NULL;
  this->LabelOpacityScale = NULL;
  this->LabelOpacityToggleButton = NULL;
  this->LabelOpacityTopLevel = NULL;
  this->LabelOutlineToggleButton = NULL;
  this->LightboxTopLevel = NULL;
  this->LinkButton = NULL;
  this->FoundationIcons = vtkSlicerFoundationIcons::New();
  this->VisibilityIcons = NULL;
  this->ViewConfigureIcons = NULL;
  this->SliceNode = NULL;
  this->SliceCompositeNode = NULL;
  this->SliceLogic = NULL;
  this->ScaleFrame = NULL;
  this->IconFrame = NULL;
  this->ColorCodeButton = NULL;
  this->SliceControlIcons = NULL;
  this->ContainerFrame = NULL;
  this->FitToWindowButton = NULL;
  this->VolumeDisplayMenuButton = NULL;
  this->LightboxButton = NULL;
  this->LightboxRowsEntry = NULL;
  this->LightboxColumnsEntry = NULL;
  this->LightboxApplyButton = NULL;
  this->LightboxRows = 1;
  this->LightboxColumns = 1;
  this->LastLabelOpacity = 1.0;
  this->PrescribedSliceSpacingEntry = 0;
  this->PrescribedSliceSpacingTopLevel = NULL;
  this->PrescribedSliceSpacingApplyButton = NULL;
  this->PrescribedSliceSpacingCancelButton = NULL;
  
  this->OffsetScaleActive = false; // Is the user interacting with
                                   // this slider?
}


//---------------------------------------------------------------------------
vtkSlicerSliceControllerWidget::~vtkSlicerSliceControllerWidget ( ){

  if ( this->FitToWindowButton )
    {
    this->FitToWindowButton->SetParent ( NULL );
    this->FitToWindowButton->Delete( );
    this->FitToWindowButton = NULL;
    }
  if ( this->MoreMenuButton)
    {
    this->MoreMenuButton->SetParent ( NULL );
    this->MoreMenuButton->Delete();
    this->MoreMenuButton = NULL;
    }
  if ( this->OffsetScale )
    {
    this->OffsetScale->SetParent(NULL);
    this->OffsetScale->Delete ( );
    this->OffsetScale = NULL;
    }
  if ( this->OffsetEntry )
    {
    this->OffsetEntry->SetParent(NULL);
    this->OffsetEntry->Delete ( );
    this->OffsetEntry = NULL;
    }
  if ( this->SliceIndexEntry )
    {
    this->SliceIndexEntry->SetParent(NULL);
    this->SliceIndexEntry->Delete ( );
    this->SliceIndexEntry = NULL;
    }
  if ( this->OrientationSelector )
    {
    this->OrientationSelector->SetParent(NULL);
    this->OrientationSelector->Delete ( );
    this->OrientationSelector = NULL;
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
  if ( this->OrientationMenuButton )
    {
    this->OrientationMenuButton->SetParent ( NULL );
    this->OrientationMenuButton->Delete();
    this->OrientationMenuButton = NULL;
    }
  if ( this->ForegroundMenuButton )
    {
    this->ForegroundMenuButton->SetParent ( NULL );
    this->ForegroundMenuButton->Delete();
    this->ForegroundMenuButton = NULL;    
    }
  if ( this->BackgroundMenuButton )
    {
    this->BackgroundMenuButton->SetParent ( NULL );
    this->BackgroundMenuButton->Delete();
    this->BackgroundMenuButton = NULL;    
    }
  if ( this->LabelMenuButton )
    {
    this->LabelMenuButton->SetParent ( NULL );
    this->LabelMenuButton->Delete();
    this->LabelMenuButton = NULL;    
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
  if ( this->LoadDataButton )
    {
    this->LoadDataButton->SetParent(NULL);
    this->LoadDataButton->Delete  ( );
    this->LoadDataButton = NULL;
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
  if ( this->LabelOpacityToggleButton )
    {
    this->LabelOpacityToggleButton->SetParent(NULL);
    this->LabelOpacityToggleButton->Delete  ( );
    this->LabelOpacityToggleButton = NULL;
    }
  
  if ( this->LabelOpacityTopLevel )
    {
    this->LabelOpacityTopLevel->SetParent(NULL);
    this->LabelOpacityTopLevel->Delete  ( );
    this->LabelOpacityTopLevel = NULL;
    }
  if ( this->LabelOutlineToggleButton )
    {
    this->LabelOutlineToggleButton->SetParent(NULL);
    this->LabelOutlineToggleButton->Delete  ( );
    this->LabelOutlineToggleButton = NULL;
    }
  if ( this->LightboxTopLevel )
    {
    this->LightboxTopLevel->SetParent(NULL);
    this->LightboxTopLevel->Delete  ( );
    this->LightboxTopLevel = NULL;
    }
  if ( this->LightboxButton )
    {
    this->LightboxButton->SetParent ( NULL );
    this->LightboxButton->Delete();
    this->LightboxButton = NULL;
    }
  if ( this->LightboxRowsEntry )
    {
    this->LightboxRowsEntry->SetParent ( NULL );
    this->LightboxRowsEntry->Delete();
    this->LightboxRowsEntry = NULL;    
    }
  if ( this->LightboxColumnsEntry )
    {
    this->LightboxColumnsEntry->SetParent ( NULL );
    this->LightboxColumnsEntry->Delete();
    this->LightboxColumnsEntry = NULL;    
    }
  if ( this->LinkButton )
    {
    this->LinkButton->SetParent(NULL);
    this->LinkButton->Delete  ( );
    this->LinkButton = NULL;
    }
  if ( this->FoundationIcons )
    {
    this->FoundationIcons->Delete();
    this->FoundationIcons = NULL;
    }
  if ( this->VisibilityIcons )
    {
    this->VisibilityIcons->Delete  ( );
    this->VisibilityIcons = NULL;
    }
  if ( this->ViewConfigureIcons )
    {
    this->ViewConfigureIcons->Delete ( );
    this->ViewConfigureIcons = NULL;
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
  if ( this->IconFrame )
    {
    this->IconFrame->SetParent(NULL);
    this->IconFrame->Delete ( );
    this->IconFrame = NULL;
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
  if ( this->LightboxButton )
    {
    this->LightboxButton->SetParent ( NULL );
    this->LightboxButton->Delete();
    this->LightboxButton = NULL;
    }
  if ( this->LightboxApplyButton )
    {
    this->LightboxApplyButton->SetParent ( NULL );
    this->LightboxApplyButton->Delete();
    this->LightboxApplyButton = NULL;
    }
  this->SetSliceNode ( NULL );
  this->SetSliceCompositeNode ( NULL );
  this->SetAndObserveSliceLogic ( NULL );
  if ( this->PrescribedSliceSpacingEntry )
    {
    this->PrescribedSliceSpacingEntry->SetParent( NULL );
    this->PrescribedSliceSpacingEntry->Delete();
    this->PrescribedSliceSpacingEntry = 0;
    }
  if ( this->PrescribedSliceSpacingTopLevel )
    {
    this->PrescribedSliceSpacingTopLevel->SetParent(NULL);
    this->PrescribedSliceSpacingTopLevel->Delete  ( );
    this->PrescribedSliceSpacingTopLevel = NULL;
    }
  if ( this->PrescribedSliceSpacingApplyButton )
    {
    this->PrescribedSliceSpacingApplyButton->SetParent ( NULL );
    this->PrescribedSliceSpacingApplyButton->Delete();
    this->PrescribedSliceSpacingApplyButton = NULL;
    }
  if ( this->PrescribedSliceSpacingCancelButton )
    {
    this->PrescribedSliceSpacingCancelButton->SetParent ( NULL );
    this->PrescribedSliceSpacingCancelButton->Delete();
    this->PrescribedSliceSpacingCancelButton = NULL;
    }

}




//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::AddWidgetObservers ( )
{
  if ( this->OffsetScale == NULL ) 
    {
    vtkErrorMacro ("Can't add observers because CreateWidget hasn't been called");
    return;
    }

  this->OrientationSelector->GetWidget()->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->ForegroundSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->BackgroundSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->LabelSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->OffsetScale->AddObserver( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
  this->OffsetScale->AddObserver( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
  this->OffsetScale->AddObserver( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
  this->OffsetEntry->AddObserver( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand );
  this->VisibilityToggle->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LoadDataButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LabelOpacityButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale ( )->AddObserver( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
  this->LabelOpacityToggleButton->AddObserver(vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LabelOutlineToggleButton->AddObserver(vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LinkButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->FitToWindowButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Slice spacing mode"))->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Lightbox view"))->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Compositing"))->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->VolumeDisplayMenuButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );    
  this->LightboxButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->LightboxApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->PrescribedSliceSpacingApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->PrescribedSliceSpacingCancelButton->AddObserver (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );    
  this->ForegroundMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->BackgroundMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
}
  

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::RemoveWidgetObservers ( ) {

  if ( this->OffsetScale == NULL ) 
    {
    vtkErrorMacro ("Can't remove observers because CreateWidget hasn't been called");
    return;
    }

  this->OrientationSelector->GetWidget()->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);
  this->ForegroundSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->BackgroundSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->LabelSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->GUICallbackCommand);
  this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale ( )->RemoveObservers( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
  this->LabelOpacityToggleButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LabelOutlineToggleButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->OffsetScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
  this->OffsetScale->RemoveObservers ( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
  this->OffsetScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
  this->OffsetEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, this->GUICallbackCommand );
  this->VisibilityToggle->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LoadDataButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LabelOpacityButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->LinkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );        
  this->FitToWindowButton->RemoveObservers (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Slice spacing mode"))->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Lightbox view"))->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Compositing"))->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->VolumeDisplayMenuButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->LightboxButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->LightboxApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->PrescribedSliceSpacingApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
  this->PrescribedSliceSpacingCancelButton->RemoveObservers (vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );    
  this->ForegroundMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
  this->BackgroundMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ApplyColorCode ( double *c )
{
  this->ColorCodeButton->SetBackgroundColor (c[0], c[1], c[2] );
}



//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::CreateWidget ( ) 
{

  int screenWidthThreshold;
  int viewerWidthThreshold;
  
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

  //
  // A stripe that color codes the SliceGUI this controller belongs to.
  //
  this->SliceControlIcons = vtkSlicerSlicesControlIcons::New ( );
  this->ColorCodeButton = vtkKWPushButton::New ( );
  this->ColorCodeButton->SetParent ( this );
  this->ColorCodeButton->Create ( );
  this->ColorCodeButton->SetBorderWidth (0 );
  this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinUp );
  this->ColorCodeButton->SetHeight (7 );
  this->ColorCodeButton->SetCommand (this, "Shrink");
  this->ColorCodeButton->SetBalloonHelpString ("Click to shrink/expand" );
  this->ContainerFrame = vtkKWFrame::New ( );
  this->ContainerFrame->SetParent ( this );
  this->ContainerFrame->Create ( );

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
  vtkSlicerGUILayout *geom = NULL;

  //
  // if screen resolution is below a certain threshold,
  // need to pack the slice controller differently so
  // that all widgets will be displayed and usable.
  //
  if ( app )
    {
    geom = app->GetDefaultGeometry();
    app->GetApplicationGUI();
    }
  if ( geom )
    {
    screenWidthThreshold = geom->GetSliceControllerResolutionThreshold();
    viewerWidthThreshold = geom->GetSliceViewerWidthThreshold();
    }
  else
    {
    screenWidthThreshold = 975;
    viewerWidthThreshold = 260;
    }

  //
  // Foreground, Background, Label and Orientation MenuButtons + Menus
  //
  this->OrientationMenuButton = vtkKWMenuButton::New();
  this->OrientationMenuButton->SetParent ( this->ContainerFrame);
  this->OrientationMenuButton->Create ( );
  this->OrientationMenuButton->SetBorderWidth ( 0 );
  this->OrientationMenuButton->SetImageToIcon ( this->SliceControlIcons->GetSetOrIcon() );
  this->OrientationMenuButton->IndicatorVisibilityOff ( );
  this->OrientationMenuButton->SetBalloonHelpString ( "Select options for the Viewer's orientation (not yet implemented)." );    
//    this->OrientationMenuButton->GetMenu()->DeleteAllItems();
//    this->OrientationMenuButton->GetMenu()->AddRadioButton ( "..." );
//    this->OrientationMenuButton->AddSeparator ( );
//    this->OrientationMenuButton->GetMenu()->AddCommand ( "close" );

    
  this->LabelMenuButton = vtkKWMenuButton::New();
  this->LabelMenuButton->SetParent ( this->ContainerFrame);
  this->LabelMenuButton->Create ( );
  this->LabelMenuButton->SetBorderWidth ( 0 );
  this->LabelMenuButton->SetImageToIcon ( this->SliceControlIcons->GetSetLbIcon() );
  this->LabelMenuButton->IndicatorVisibilityOff ( );
  this->LabelMenuButton->SetBalloonHelpString ( "Select options for the Label Layer (not yet implemented)." );    
//    this->LabelMenuButton->GetMenu()->DeleteAllItems();
//    this->LabelMenuButton->GetMenu()->AddRadioButton ( "...");
//    this->LabelMenuButton->GetMenu()->AddSeparator ( );    
//    this->LabelMenuButton->GetMenu()->AddCommand ( "close ");
    

  this->ForegroundMenuButton = vtkKWMenuButton::New();
  this->ForegroundMenuButton->SetParent ( this->ContainerFrame);
  this->ForegroundMenuButton->Create ( );
  this->ForegroundMenuButton->SetBorderWidth ( 0 );
  this->ForegroundMenuButton->SetImageToIcon ( this->SliceControlIcons->GetSetFgIcon() );
  this->ForegroundMenuButton->IndicatorVisibilityOff ( );
  this->ForegroundMenuButton->SetBalloonHelpString ( "Select options for the Foreground Layer" );    
  this->ForegroundMenuButton->GetMenu()->DeleteAllItems ( );
  this->ForegroundMenuButton->GetMenu()->AddCheckButton ( "interpolation" );
  this->ForegroundMenuButton->GetMenu()->AddSeparator ( );
  this->ForegroundMenuButton->GetMenu()->AddCommand ( "close");    
    
  this->BackgroundMenuButton = vtkKWMenuButton::New();
  this->BackgroundMenuButton->SetParent ( this->ContainerFrame);
  this->BackgroundMenuButton->Create ( );
  this->BackgroundMenuButton->SetBorderWidth ( 0 );
  this->BackgroundMenuButton->SetImageToIcon ( this->SliceControlIcons->GetSetBgIcon() );
  this->BackgroundMenuButton->IndicatorVisibilityOff ( );
  this->BackgroundMenuButton->SetBalloonHelpString ( "Select options for the Background Layer" );    
  this->BackgroundMenuButton->GetMenu()->DeleteAllItems ( );
  this->BackgroundMenuButton->GetMenu()->AddCheckButton ( "interpolation" );
  this->BackgroundMenuButton->GetMenu()->AddSeparator ( );
  this->BackgroundMenuButton->GetMenu()->AddCommand ( "close");    

  //
  // Orientation  (TODO: make this into a vtkSlicerOrientationWidget)
  //
  this->OrientationSelector = vtkKWMenuButtonWithSpinButtonsWithLabel::New ();
  this->OrientationSelector->SetParent ( this->ContainerFrame );
  this->OrientationSelector->Create ( );    
  this->OrientationSelector->SetBalloonHelpString ("Select orientation" );
  vtkKWMenuButton *mb = this->OrientationSelector->GetWidget()->GetWidget();
  mb->SetWidth ( 10 );
  mb->GetMenu()->AddRadioButton ( "Axial" );
  mb->GetMenu()->AddRadioButton ( "Sagittal" );
  mb->GetMenu()->AddRadioButton ( "Coronal" );
  mb->GetMenu()->AddRadioButton ( "Reformat" );
  mb->SetValue ("Axial");    

  // Foreground, Background, and Label selections
  //
  this->ForegroundSelector = vtkSlicerNodeSelectorWidget::New();
  this->ForegroundSelector->SetParent ( this->ContainerFrame );
  this->ForegroundSelector->Create ( );
  this->ForegroundSelector->NoneEnabledOn();
  this->ForegroundSelector->SetBalloonHelpString ( "Select the foreground");
  this->ForegroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
  this->ForegroundSelector->SetMRMLScene( this->MRMLScene );
  this->ForegroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
  this->ForegroundSelector->GetWidget()->GetWidget()->SetWidth(10);

  this->BackgroundSelector = vtkSlicerNodeSelectorWidget::New();
  this->BackgroundSelector->SetParent ( this->ContainerFrame );
  this->BackgroundSelector->Create ( );
  this->BackgroundSelector->NoneEnabledOn();
    
  this->BackgroundSelector->SetBalloonHelpString ( "Select the background");
  this->BackgroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
  this->BackgroundSelector->SetMRMLScene( this->MRMLScene );
  this->BackgroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
  this->BackgroundSelector->GetWidget()->GetWidget()->SetWidth(10);

  this->LabelSelector = vtkSlicerNodeSelectorWidget::New();
  this->LabelSelector->SetParent ( this->ContainerFrame );
  this->LabelSelector->Create ( );
  this->LabelSelector->NoneEnabledOn();
  this->LabelSelector->SetBalloonHelpString ( "Select the label map");
  this->LabelSelector->SetNodeClass ("vtkMRMLVolumeNode", "LabelMap", "1", NULL);
  this->LabelSelector->SetMRMLScene( this->MRMLScene );
  this->LabelSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);
  this->LabelSelector->GetWidget()->GetWidget()->SetWidth(10);

  //
  // Create the frame to contain scale and icons
  //
  this->ScaleFrame = vtkKWFrame::New ();
  this->ScaleFrame->SetParent ( this->ContainerFrame );
  this->ScaleFrame->Create ( );
  //
  // Create a frame to contain just the icons, if
  // slicer window is too small to pack the scale
  // and icons in the same row of the controller.
  this->IconFrame = vtkKWFrame::New ();
  this->IconFrame->SetParent ( this->ContainerFrame );
  this->IconFrame->Create ( );

  //
  // icons we need...
  this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );
  this->ViewConfigureIcons = vtkSlicerToolbarIcons::New ( );
  //

    
  //
  // tailor layout of gui packing for window size.
  //
  // TODO: once applicationsettings can save the slicer window
  // width, use this instead of screen resolution.
  // FOR NOW: use screen resolution:
  const char *str = this->Script ("winfo screenwidth .");
  int screenwidth = atoi (str);

  this->LinkButton = vtkKWPushButton::New ( );
  this->LinkButton->SetParent ( this->ContainerFrame );
  this->VisibilityToggle = vtkKWPushButton::New ( );
  this->VisibilityToggle->SetParent ( this->ContainerFrame );      
  this->LoadDataButton = vtkKWPushButton::New ( );
  this->LoadDataButton->SetParent ( this->ContainerFrame );      
  this->FitToWindowButton = vtkKWPushButton::New ( );
  this->FitToWindowButton->SetParent ( this->ContainerFrame );
  this->LabelOutlineToggleButton = vtkKWPushButton::New ( );
  this->LabelOutlineToggleButton->SetParent ( this->ContainerFrame );
  this->VolumeDisplayMenuButton = vtkKWMenuButton::New ( );
  this->VolumeDisplayMenuButton->SetParent ( this->ContainerFrame );
  this->LightboxButton = vtkKWMenuButton::New();
  this->LightboxButton->SetParent ( this->ContainerFrame );
  this->LabelOpacityButton = vtkKWPushButton::New ( );
  this->LabelOpacityButton->SetParent (this->ContainerFrame );
  this->MoreMenuButton = vtkKWMenuButton::New();
  this->MoreMenuButton->SetParent ( this->ContainerFrame );


  //
  // Create a button to toggle the slice visibility in the main viewer and icons for it
  //
  this->VisibilityToggle->Create ( );
  this->VisibilityToggle->SetReliefToFlat ( );
  this->VisibilityToggle->SetOverReliefToNone ( );
  this->VisibilityToggle->SetBorderWidth ( 0 );
  this->VisibilityToggle->SetImageToIcon ( this->VisibilityIcons->GetInvisibleIcon ( ) );        
  this->VisibilityToggle->SetBalloonHelpString ( "Toggles slice visibility in the MainViewer." );


  //
  // Create a button to toggle the slice visibility in the main viewer and icons for it
  //

  this->LinkButton->Create ( );
  this->LinkButton->SetReliefToFlat ( );
  this->LinkButton->SetOverReliefToNone ( );
  this->LinkButton->SetBorderWidth ( 0 );
  this->LinkButton->SetImageToIcon ( this->SliceControlIcons->GetUnlinkControlsIcon ( ) );        
  this->LinkButton->SetBalloonHelpString ( "Links/Unlinks the slice controls (except scales) across all Slice Viewers." );

  //
  // Create a button to fit the view to the window
  //

  this->FitToWindowButton->Create ( );
  this->FitToWindowButton->SetReliefToFlat ( );
  this->FitToWindowButton->SetOverReliefToNone ( );
  this->FitToWindowButton->SetBorderWidth ( 0 );
  this->FitToWindowButton->SetImageToIcon ( this->SliceControlIcons->GetFitToWindowIcon ( ));    
  this->FitToWindowButton->SetBalloonHelpString ( "Adjusts the Slice Viewer's field of view to match the extent of lowest non-None volume layer (bg, then fg, then label).");

  //
  // Create a button to toggle the label outline view

  this->LabelOutlineToggleButton->Create();
  this->LabelOutlineToggleButton->SetReliefToFlat ( );
  this->LabelOutlineToggleButton->SetOverReliefToNone ( );
  this->LabelOutlineToggleButton->SetBorderWidth ( 0 );
  this->LabelOutlineToggleButton->SetImageToIcon ( this->SliceControlIcons->GetSliceLabelOutlineOnIcon ( ));    
  this->LabelOutlineToggleButton->SetBalloonHelpString ( "Toggle between showing label map volume with regions outlined or filled.");


  //
  // Create a menubutton that navigates to Volumes->Display
  // 

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

  //
  // Create a lightbox menubutton that allows viewer to be reconfigured
  //


  this->LightboxButton->Create();
  this->LightboxButton->SetBorderWidth ( 0 );
  this->LightboxButton->IndicatorVisibilityOff ( );
  this->LightboxButton->SetImageToIcon ( this->ViewConfigureIcons->GetLightBoxViewIcon ( ) );
  this->LightboxButton->SetBalloonHelpString ( "Configure the Slice viewer layout");
  this->LightboxButton->GetMenu()->AddRadioButton ("1x1 view");
  this->LightboxButton->GetMenu()->AddRadioButton ( "1x2 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ( "1x3 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ( "1x4 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ( "1x6 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ( "1x8 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ("2x2 view");
  this->LightboxButton->GetMenu()->AddRadioButton ("3x3 view");
  this->LightboxButton->GetMenu()->AddRadioButton ( "6x6 view");    
  this->LightboxButton->GetMenu()->AddRadioButton ( "customized view");    
  this->LightboxButton->GetMenu()->AddSeparator ( );
  this->LightboxButton->GetMenu()->AddCommand ("close");
  //this->LightboxButton->GetMenu()->SetItemStateToDisabled ( "customized view" );
            
  //--- Pop-up frame for custom NXM lightbox configuration
  this->LightboxTopLevel = vtkKWTopLevel::New ( );
  this->LightboxTopLevel->SetApplication ( app );
//    this->LightboxTopLevel->SetMasterWindow ( this->LightboxButton );
  this->LightboxTopLevel->SetMasterWindow ( this->ContainerFrame );
  this->LightboxTopLevel->Create ( );
  this->LightboxTopLevel->HideDecorationOn ( );
  this->LightboxTopLevel->Withdraw ( );
  this->LightboxTopLevel->SetBorderWidth ( 2 );
  this->LightboxTopLevel->SetReliefToGroove ( );

  //--- create temporary pop-up frame to display when custom configuration is selected
  vtkKWFrame *popUpFrame1 = vtkKWFrame::New ( );
  popUpFrame1->SetParent ( this->LightboxTopLevel );
  popUpFrame1->Create ( );
//    popUpFrame1->SetBinding ( "<Leave>", this, "HideLightboxCustomLayoutFrame" );
  this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", popUpFrame1->GetWidgetName ( ) );   
  this->LightboxRowsEntry = vtkKWEntry::New ( );
  this->LightboxRowsEntry->SetParent ( popUpFrame1 );
  this->LightboxRowsEntry->Create ( );
  this->LightboxRowsEntry->SetValueAsInt (1);
  this->LightboxRowsEntry->SetWidth ( 3 );
  this->LightboxColumnsEntry = vtkKWEntry::New ( );
  this->LightboxColumnsEntry->SetParent ( popUpFrame1 );
  this->LightboxColumnsEntry->Create ( );
  this->LightboxColumnsEntry->SetWidth ( 3 );
  this->LightboxColumnsEntry->SetValueAsInt (1);
  vtkKWLabel *rowsLabel = vtkKWLabel::New();
  rowsLabel->SetParent ( popUpFrame1 );
  rowsLabel->Create ( );
  rowsLabel->SetText ( "Number of rows:" );
  vtkKWLabel *columnsLabel = vtkKWLabel::New();
  columnsLabel->SetParent ( popUpFrame1 );
  columnsLabel->Create ( );
  columnsLabel->SetText ( "Number of columns:" );

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( popUpFrame1);
  f->Create();
  this->LightboxApplyButton = vtkKWPushButton::New ( );
  this->LightboxApplyButton->SetParent ( f );
  this->LightboxApplyButton->Create ( );
  this->LightboxApplyButton->SetText ("Apply");
  vtkKWPushButton *b = vtkKWPushButton::New();
  b->SetParent ( f );
  b->Create();
  b->SetText ( "Cancel");
  b->SetBinding ( "<Button-1>", this,  "HideLightboxCustomLayoutFrame");
  this->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 8", rowsLabel->GetWidgetName());
  this->Script ( "grid %s -row 0 -column 1 -padx 6 -pady 8", this->LightboxRowsEntry->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 8", columnsLabel->GetWidgetName());
  this->Script ( "grid %s -row 1 -column 1 -padx 6 -pady 8", this->LightboxColumnsEntry->GetWidgetName() );
  this->Script ( "grid %s -row 2 -column 0 -columnspan 2 -pady 8 -sticky ew", f->GetWidgetName() );
  this->Script ( "pack %s %s -side left -padx 4 -anchor c", b->GetWidgetName(), this->LightboxApplyButton->GetWidgetName() );

  // delete temporary stuff
  b->Delete();
  f->Delete();
  rowsLabel->Delete();
  columnsLabel->Delete();
  popUpFrame1->Delete();

  this->PrescribedSliceSpacingTopLevel = vtkKWTopLevel::New ( );
  this->PrescribedSliceSpacingTopLevel->SetApplication ( app );
  this->PrescribedSliceSpacingTopLevel->SetMasterWindow ( this->ContainerFrame );
  this->PrescribedSliceSpacingTopLevel->Create ( );
  this->PrescribedSliceSpacingTopLevel->HideDecorationOn ( );
  this->PrescribedSliceSpacingTopLevel->Withdraw ( );
  this->PrescribedSliceSpacingTopLevel->SetBorderWidth ( 2 );
  this->PrescribedSliceSpacingTopLevel->SetReliefToGroove ( );

  vtkKWFrame *popUpFrameP = vtkKWFrame::New ( );
  popUpFrameP->SetParent ( this->PrescribedSliceSpacingTopLevel );
  popUpFrameP->Create ( );
  this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", popUpFrameP->GetWidgetName ( ) );
  this->PrescribedSliceSpacingEntry = vtkKWSpinBoxWithLabel::New();
  this->PrescribedSliceSpacingEntry->SetParent( popUpFrameP );
  this->PrescribedSliceSpacingEntry->Create();
  this->PrescribedSliceSpacingEntry->GetWidget()->SetValue( 1.0 );
  this->PrescribedSliceSpacingEntry->GetWidget()->SetIncrement( 0.001 );
  this->PrescribedSliceSpacingEntry->GetWidget()->SetRange(0, VTK_LARGE_FLOAT);
  this->PrescribedSliceSpacingEntry->GetWidget()->SetWidth( 5 );
  this->PrescribedSliceSpacingEntry->SetLabelText( "Prescribed spacing: " );
  popUpFrameP->Delete();

  vtkKWFrame *fP = vtkKWFrame::New();
  fP->SetParent ( popUpFrameP);
  fP->Create();
  this->PrescribedSliceSpacingApplyButton = vtkKWPushButton::New ( );
  this->PrescribedSliceSpacingApplyButton->SetParent ( fP );
  this->PrescribedSliceSpacingApplyButton->Create ( );
  this->PrescribedSliceSpacingApplyButton->SetText ("Apply");
  this->PrescribedSliceSpacingCancelButton = vtkKWPushButton::New();
  PrescribedSliceSpacingCancelButton->SetParent ( fP );
  PrescribedSliceSpacingCancelButton->Create();
  PrescribedSliceSpacingCancelButton->SetText ( "Cancel");
//    PrescribedSliceSpacingCancelButton->SetBinding ( "<Button-1>", this,  "HidePrescribedSliceSpacingEntry");
  this->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 8", this->PrescribedSliceSpacingEntry->GetWidgetName());
  this->Script ( "grid %s -row 1 -column 0 -columnspan 1 -pady 8 -sticky ew", fP->GetWidgetName() );
  this->Script ( "pack %s %s -side left -padx 4 -anchor c", PrescribedSliceSpacingCancelButton->GetWidgetName(), this->PrescribedSliceSpacingApplyButton->GetWidgetName() );
  fP->Delete();
    
    
  this->MoreMenuButton->Create();
  this->MoreMenuButton->SetBorderWidth ( 0 );
  this->MoreMenuButton->SetImageToIcon ( this->SliceControlIcons->GetSliceMoreOptionsIcon() );
  this->MoreMenuButton->SetBalloonHelpString ( "Displays a menu of more options for the Slice Viewer." );
  this->MoreMenuButton->IndicatorVisibilityOff();

  //--- 
  //--- FIT SLICE IMAGE TO WINDOW
  //---
  int index;
  const char *imageName;
  this->MoreMenuButton->GetMenu()->AddCommand ("Fit to window", this, "FitSliceToBackground") ;
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Fit to window");
  imageName = "FitImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetFitToWindowIcon ( ));
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  //--- 
  //--- Rotate SLICE IMAGE TO WINDOW
  //---
  this->MoreMenuButton->GetMenu()->AddCommand ("Rotate to Volume Plane", this, "RotateSliceToBackground") ;
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Rotate to Volume Plane");
  if ( this->FoundationIcons != NULL )
    {
    imageName = "RotateToPixelSpaceImage";
    vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->FoundationIcons->GetSlicerRotateToPixelSpaceIcon ( ));
    this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
    }
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  

  //---     //--- 
  //--- COMPOSITING
  //--- 

  //--- LABEL OPACITY
  //---
  this->MoreMenuButton->GetMenu()->AddCommand ("Adjust label map opacity", this, "PopUpLabelOpacityScaleAndEntry" );
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Adjust label map opacity");
  imageName = "LabelOpacityImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetAllLabelOpacityIcon ( ));
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  //--- 
  //--- TOGGLE LABELMAP OUTLINES
  //---
  this->MoreMenuButton->GetMenu()->AddCommand ("Show label volume outlines", this, "ToggleLabelOutline" );
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Show label volume outlines");
  imageName = "SliceLabelOutlineImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceLabelOutlineOnIcon ( ));
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    

  //--- 
  //--- TOGGLE REFORMAT WIDGET
  //---
  this->MoreMenuButton->GetMenu()->AddCommand ("Show reformat widget", this, "ToggleReformatWidget");
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Show reformat widget");
  imageName = "ReformatImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceWidgetOnIcon() );
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  //--- 
  //--- COMPOSITING
  //--- 
  vtkKWMenu* m3 = vtkKWMenu::New();
  m3->SetParent(this->MoreMenuButton->GetMenu());
  m3->Create();
  m3->AddRadioButton ( "Alpha blend" );
  m3->AddRadioButton ( "Reverse alpha blend" );
  m3->AddRadioButton ( "Add" );
  m3->AddRadioButton ( "Subtract" );
  m3->AddSeparator();
  m3->AddCommand( "close" );
  this->MoreMenuButton->GetMenu()->AddCascade ("Compositing", m3);
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Compositing");
  imageName = "CompositeImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( app, imageName,  this->FoundationIcons->GetSlicerCompositeIcon ( ));
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  //--- initialize button
  vtkMRMLSliceCompositeNode *cnode = this->SliceCompositeNode;
  if ( cnode )
    {
    if ( cnode->GetCompositing() == vtkMRMLSliceCompositeNode::Alpha )
      {
      m3->SelectItem ("Alpha blend" );
      }
    else if ( cnode->GetCompositing() == vtkMRMLSliceCompositeNode::Add)
      {
      m3->SelectItem ( "Add");
      }
    else if ( cnode->GetCompositing() == vtkMRMLSliceCompositeNode::Subtract )
      {
      m3->SelectItem ( "Subtract");
      }
    }
  else
    {
    m3->SelectItem ("Alpha blend" );
    }
  m3->Delete();

  //--- 
  //--- SLICE SPACING MODE AND OPTIONS
  //---
  vtkKWMenu *ms = vtkKWMenu::New();
  ms->SetParent(this->MoreMenuButton->GetMenu());
  ms->Create();
  ms->AddRadioButton( "Automatic" );
  ms->AddRadioButton( "Manual" );
  ms->AddSeparator();
  ms->AddCommand( "close" );
  ms->SelectItem( "Automatic" );
  // add the cascade to the slice spacing mode radiobutton
  this->MoreMenuButton->GetMenu()->AddCascade("Slice spacing mode", ms);
  // set the icon for slice spacing mode in its default 'automatic' setting.
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Slice spacing mode");
  imageName = "SliceSpacingModeImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SliceControlIcons->GetAutomaticSliceSpacingModeIcon ( ) );
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  // set icons for Automatic and manual slice spacing options
  vtkKWMenu *msc = this->MoreMenuButton->GetMenu()->GetItemCascade( index );
  index = msc->GetIndexOfItem ("Automatic");
  imageName = "AutomaticSliceSpacingImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SliceControlIcons->GetAutomaticSliceSpacingIcon ( ) );
  msc->SetItemImage ( index, imageName);
  msc->SetItemCompoundModeToLeft ( index );    
  index = msc->GetIndexOfItem ("Manual");
  imageName = "ManualSliceSpacingImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SliceControlIcons->GetManualSliceSpacingIcon ( ) );
  msc->SetItemImage ( index, imageName);
  msc->SetItemCompoundModeToLeft ( index );    
  ms->Delete();

  // this->MoreMenuButton->GetMenu()->AddSeparator ( );

  //--- 
  //--- LIGHTBOX
  //---
  vtkKWMenu* m1 = vtkKWMenu::New();
  m1->SetParent(this->MoreMenuButton->GetMenu());
  m1->Create();
  m1->AddRadioButton ( "1x1 view" );
  m1->AddRadioButton ( "1x2 view");    
  m1->AddRadioButton ( "1x3 view");    
  m1->AddRadioButton ( "1x4 view");    
  m1->AddRadioButton ( "1x6 view");    
  m1->AddRadioButton ( "1x8 view");    
  m1->AddRadioButton ( "2x2 view" );
  m1->AddRadioButton ( "3x3 view" );
  m1->AddRadioButton ( "6x6 view" );
  m1->AddRadioButton ( "customized view" );
  m1->AddSeparator();
  m1->AddCommand ( "close" );    
  this->MoreMenuButton->GetMenu()->AddCascade ("Lightbox view", m1);
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Lightbox view");
  imageName = "LightboxImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->ViewConfigureIcons->GetLightBoxViewIcon ( ) );
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  m1->Delete();

  //--- 
  //--- ADJUST DISPLAY PROPERTIES
  //---
  vtkKWMenu* m2 = vtkKWMenu::New();
  m2->SetParent(this->MoreMenuButton->GetMenu());
  m2->Create();
  m2->AddRadioButton ( "Foreground volume" );
  m2->AddRadioButton ( "Background volume" );
  m2->AddRadioButton ( "Label map volume" );
  m2->AddSeparator();
  m2->AddCommand( "close" );
  this->MoreMenuButton->GetMenu()->AddCascade ("Adjust display", m2);
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "Adjust display");
  imageName = "DisplayImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetWinLevThreshColIcon ( ));
  this->MoreMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->MoreMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  m2->Delete();

  this->MoreMenuButton->GetMenu()->AddSeparator ( );
  this->MoreMenuButton->GetMenu()->AddCommand ( "tear off control panel" );
  index = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( "tear off control panel");
  this->MoreMenuButton->GetMenu()->SetItemStateToDisabled ( index );
    
  this->MoreMenuButton->GetMenu()->AddSeparator ( );
  this->MoreMenuButton->GetMenu()->AddCommand ("close");


  //--- Popup Scale with Entry (displayed when user clicks LabelOpacityButton
  //--- LabelOpacityButton, LabelOpacityScale and its entry will be observed
  //--- and their events handled in sssGUIEvents;
  //--- the pop-up and hide behavior of the latter two will be managed locally
  //--- in the GUI.
  //--- TODO: make a SlicerWidget that handles this behavior. Leave event?
  this->LabelOpacityTopLevel = vtkKWTopLevel::New ( );
  this->LabelOpacityTopLevel->SetApplication ( app );
  this->LabelOpacityTopLevel->SetMasterWindow ( this->ContainerFrame );
  this->LabelOpacityTopLevel->Create ( );
  this->LabelOpacityTopLevel->HideDecorationOn ( );
  this->LabelOpacityTopLevel->Withdraw ( );
  this->LabelOpacityTopLevel->SetBorderWidth ( 2 );
  this->LabelOpacityTopLevel->SetReliefToGroove ( );
  //--- create temporary frame
  vtkKWFrame *popUpFrame2 = vtkKWFrame::New ( );
  popUpFrame2->SetParent ( this->LabelOpacityTopLevel );
  popUpFrame2->Create ( );
  popUpFrame2->SetBinding ( "<Leave>", this, "HideLabelOpacityScaleAndEntry" );
  this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", popUpFrame2->GetWidgetName ( ) );   
  // Scale and entry packed in the pop-up toplevel's frame
  this->LabelOpacityScale = vtkKWScaleWithEntry::New ( );
  this->LabelOpacityScale->SetParent ( popUpFrame2 );
  this->LabelOpacityScale->Create ( );
  this->LabelOpacityScale->SetRange ( 0.0, 1.0 );
  this->LabelOpacityScale->SetResolution ( 0.01 );
  this->LabelOpacityScale->GetScale()->SetLabelText ( "" );
  this->LabelOpacityScale->GetScale()->ValueVisibilityOff ( );
  this->LabelOpacityScale->SetValue ( 1.0 );
  // button to toggle between opacity 0 and 1
  this->LabelOpacityToggleButton = vtkKWPushButton::New();
  this->LabelOpacityToggleButton->SetParent ( popUpFrame2 );
  this->LabelOpacityToggleButton->Create();
  this->LabelOpacityToggleButton->SetBorderWidth(0);
  // show the visible icon, sine default label opacity is 1.0
  this->LabelOpacityToggleButton->SetImageToIcon(this->GetVisibilityIcons()->GetVisibleIcon ( ));
  this->LabelOpacityToggleButton->SetBalloonHelpString( "Toggle Label opacity between 0 and last opacity in the Slice Viewers" );
  this->Script ( "pack %s -side left -anchor w -padx 1 -pady 3 -expand n", this->LabelOpacityScale->GetWidgetName ( ) );
  this->Script ( "pack %s -side left -anchor e -padx 2 -pady 3 -expand n", this->LabelOpacityToggleButton->GetWidgetName());

  this->LabelOpacityButton->Create ( );
  this->LabelOpacityButton->SetBorderWidth ( 0 );
  this->LabelOpacityButton->SetImageToIcon ( this->SliceControlIcons->GetAllLabelOpacityIcon() );
  this->LabelOpacityButton->SetBalloonHelpString ( "Popup scale to adjust opacity of Label Layer." );
  //--- delete temporary frame
  popUpFrame2->Delete ( );


  //
  // Create a scale to control the slice number displayed
  //
  this->OffsetScale = vtkKWScale::New();
  this->OffsetScale->SetParent ( this->ScaleFrame );
  this->OffsetScale->Create();
  this->OffsetScale->SetValueVisibility(0);

  this->OffsetEntry = vtkKWEntry::New();
  this->OffsetEntry->SetParent ( this->ScaleFrame );
  this->OffsetEntry->Create();
  this->OffsetEntry->SetRestrictValueToDouble();
  this->OffsetEntry->SetWidth(8);
  this->OffsetEntry->SetBalloonHelpString ( "Slice distance from RAS origin" );

  this->SliceIndexEntry = vtkKWEntry::New();
  this->SliceIndexEntry->SetParent ( this->ScaleFrame );
  this->SliceIndexEntry->Create();
  this->SliceIndexEntry->SetWidth(0);
  this->SliceIndexEntry->SetBalloonHelpString ( "Slice number display\nInteger: slice number starting from 1 (not 0)\nLetter 'O': slice is out of volume\nLetter 'R': use Rotate to Volume Plane to see slice index\n\nUse this display to cross-reference to filmed images or other displays." );
  this->SliceIndexEntry->SetReliefToFlat();
  this->SliceIndexEntry->ReadOnlyOn();
  this->SliceIndexEntry->SetConfigurationOption("-justify", "left");
            
  //
  // Pack everyone up
  ///
    this->Script ( "pack %s -side top -expand 1 -fill x", 
                   this->ColorCodeButton->GetWidgetName ( ));
    this->Script ("pack %s -side bottom -expand 1 -fill x", 
                  this->ContainerFrame->GetWidgetName());
    this->Script("grid columnconfigure %s 0 -weight 0", 
                 this->ContainerFrame->GetWidgetName());
    this->Script("grid columnconfigure %s 1 -weight 1", 
                 this->ContainerFrame->GetWidgetName());
    this->Script("grid columnconfigure %s 2 -weight 0", 
                 this->ContainerFrame->GetWidgetName());
    this->Script("grid columnconfigure %s 3 -weight 1", 
                 this->ContainerFrame->GetWidgetName());

    
    this->Script("grid %s %s %s %s -sticky ew -padx 1",
                 this->OrientationMenuButton->GetWidgetName (),
                 this->OrientationSelector->GetWidgetName(),
                 this->ForegroundMenuButton->GetWidgetName(),
                 this->ForegroundSelector->GetWidgetName());
    this->Script("grid %s %s %s %s -sticky ew -padx 1", 
                 this->LabelMenuButton->GetWidgetName(),
                 this->LabelSelector->GetWidgetName(),
                 this->BackgroundMenuButton->GetWidgetName(),
                 this->BackgroundSelector->GetWidgetName());

    //
    // Pack the icons and slider on different rows
    // if the overall screen resolution is tiny, or if
    // the window is sized too small to display them
    // on a single row. If resolution is there, put them
    // on the same row to save vertical space for imagedata.
    //
    if ( (screenwidth <= screenWidthThreshold) || (app->GetApplicationWindowWidth() < screenWidthThreshold) )
      {
      this->Script ( "grid %s -sticky ew -columnspan 4", this->IconFrame->GetWidgetName ( ) );
      this->Script ( "grid %s -sticky ew -columnspan 4", this->ScaleFrame->GetWidgetName ( ) );
      this->Script ("pack %s -side left -expand n -padx 1 -in %s", 
                    this->LinkButton->GetWidgetName ( ),
                    this->IconFrame->GetWidgetName());
      this->Script ("pack %s -side left -expand n -padx 1 -in %s", 
                    this->VisibilityToggle->GetWidgetName ( ),
                    this->IconFrame->GetWidgetName());
      this->Script ( "pack %s -side left -expand n -padx 1 -in %s",
                     this->MoreMenuButton->GetWidgetName(),
                     this->IconFrame->GetWidgetName() );
      this->Script("pack %s -side left -fill x -expand y -in %s", 
                   this->OffsetScale->GetWidgetName(),
                   this->ScaleFrame->GetWidgetName());
      this->Script("pack %s %s -side left -expand n -in %s", 
                   this->SliceIndexEntry->GetWidgetName(),
                   this->OffsetEntry->GetWidgetName(),
                   this->ScaleFrame->GetWidgetName());
      }
    else
      {
      this->Script ( "grid %s -sticky ew -columnspan 4", this->ScaleFrame->GetWidgetName ( ) );
      this->Script ("pack %s -side left -expand n -padx 1 -in %s", 
                    this->LinkButton->GetWidgetName ( ),
                    this->ScaleFrame->GetWidgetName());
      this->Script ("pack %s -side left -expand n -padx 1 -in %s", 
                    this->VisibilityToggle->GetWidgetName ( ),
                    this->ScaleFrame->GetWidgetName());
      this->Script ( "pack %s -side left -expand n -padx 1 -in %s",
                     this->MoreMenuButton->GetWidgetName(),
                     this->ScaleFrame->GetWidgetName() );      
      this->Script("pack %s -side left -fill x -expand y -in %s", 
                   this->OffsetScale->GetWidgetName(),
                   this->ScaleFrame->GetWidgetName());
      this->Script("pack %s %s -side left -expand n -in %s", 
                   this->SliceIndexEntry->GetWidgetName(),
                   this->OffsetEntry->GetWidgetName(),
                   this->ScaleFrame->GetWidgetName());
      }

    // we want to get rid of the labels in the node selector widgets.
    // instead of using these, we're using the menubuttons to
    // dual as labels and as containers for configuration options for each layer.
    this->Script ( "pack forget %s", this->BackgroundSelector->GetLabel()->GetWidgetName() );
    this->Script ( "pack forget %s", this->ForegroundSelector->GetLabel()->GetWidgetName() );
    this->Script ( "pack forget %s", this->LabelSelector->GetLabel()->GetWidgetName() );
    this->Script ( "pack forget %s", this->OrientationSelector->GetLabel()->GetWidgetName() );

    // and put observers on widgets
    this->AddWidgetObservers();
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UpdateLayerMenus()
{
  // ensure that the interpolation checkbutton state matches the node
  int interp;
  if ( this->SliceLogic )
    {
    if ( this->SliceLogic->GetBackgroundLayer() )
      {
      vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->SliceLogic->GetBackgroundLayer()->GetVolumeDisplayNode());
      if ( displayNode )
        {
        interp = displayNode->GetInterpolate();
        if ( interp != this->BackgroundMenuButton->GetMenu()->GetItemSelectedState ( "interpolation" ) )
          {
          this->BackgroundMenuButton->GetMenu()->SetItemSelectedState ( "interpolation",
                                                                        displayNode->GetInterpolate() );
          }
        }
      }
    if ( this->SliceLogic->GetForegroundLayer() )
      {
      vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->SliceLogic->GetForegroundLayer()->GetVolumeDisplayNode());
      if ( displayNode )
        {
        interp = displayNode->GetInterpolate();
        if ( interp != this->ForegroundMenuButton->GetMenu()->GetItemSelectedState ( "interpolation" ) )
          {
          this->ForegroundMenuButton->GetMenu()->SetItemSelectedState ( "interpolation",
                                                                        displayNode->GetInterpolate() );
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UpdateOrientation ( int link )
{
  int i, nnodes;
  vtkMRMLSliceNode *snode;

  //--- if slice viewers are linked, modify all Controller's SliceNodes;
  //--- otherwise just update the one we know about.
  vtkKWMenuButton *mb = this->OrientationSelector->GetWidget()->GetWidget();
  if ( !strcmp (mb->GetValue(), "Axial") )   
    {
    if ( link )
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
    if ( link )
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
    if ( link )
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
  if ( !strcmp (mb->GetValue(), "Reformat") )   
    {
    if ( link )
      {
      nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
      for ( i=0; i<nnodes; i++)
        {
        snode = vtkMRMLSliceNode::SafeDownCast (
          this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
        snode->SetOrientationToReformat();
        }
      }
    else
      {
      this->SliceNode->SetOrientationToReformat();
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UpdateForegroundLayer ( int link )
{
  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  
  //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes
  //--- otherwise, just update the one we know about.
  if ( link )
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

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UpdateBackgroundLayer ( int link )
{
  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  
  //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes
  //--- otherwise, just update the one we know about.
  if ( link )
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

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::UpdateLabelLayer ( int link )
{

  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  
  //--- if slice viewers are linked, modify all Controller's SliceCompositeNodes
  //--- otherwise, just update the one we know about.
  if ( link )
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


//----------------------------------------------------------------------------
//void vtkSlicerSliceControllerWidget::RaiseVolumeDisplayPanel ( char *id )
//{
//  vtkSlicerVolumesGUI *vgui;
//  vtkSlicerApplication *app;
//  vtkSlicerApplicationGUI *appgui;
//    
//  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
//  vgui = vtkSlicerVolumesGUI::SafeDownCast ( app->GetModuleGUIByName ("Volumes") );
//  appgui = vgui->GetApplicationGUI ( );
//  appgui->SelectModule ( "Volumes" );
//
//  vtkMRMLNode *volumeNode = this->GetMRMLScene()->GetNodeByID ( id );
//  vgui->GetVolumeSelectorWidget()->SetSelected(volumeNode);
//  vgui->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID ( id );
//  vgui->GetVolumeDisplayWidget()->SetVolumeNode (vtkMRMLVolumeNode::SafeDownCast (this->GetMRMLScene()->GetNodeByID ( id )) );
//
//  vgui->GetDisplayFrame()->ExpandFrame();
//}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ToggleLabelOutline()
{
  int link = 0;

  //--- swallow the menu event
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->SetAbortFlag(1);
    }

  //
  // --- Find out whether SliceViewers are linked or unlinked
  // --- so we know how to handle control.
  //
  if ( this->SliceCompositeNode )
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }

  this->ToggleLabelOutline ( link );
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ToggleLabelOutline( int link)
{
  vtkSlicerSlicesGUI *ssgui;
  vtkSlicerSliceGUI *sgui;
  vtkSlicerApplication *app;
  vtkSlicerApplicationGUI *appGUI;
  int found = 0;
    
  // find the sliceGUI for this controller
  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  appGUI = ssgui->GetApplicationGUI ( );

  
  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
      
      if (sgui)
        {
        if (sgui->GetSliceNode() == this->SliceNode )
          {
          found = 1;
          break;
          }
        }
      }
    }
  else
    {
    return;
    }

  if ( found )
    {
    if ( link )
      {
      
      // First save all SliceNodes for undo: -- probably not necessary
      vtkCollection *nodes = vtkCollection::New();
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
        
        if (sgui)
          {
          nodes->AddItem ( sgui->GetSliceNode ( ) );
          }
        }
      
      this->MRMLScene->SaveStateForUndo ( nodes );
      nodes->Delete ( );

      // Set all linked slice nodes' slice layer logic's  to be the toggled state of this one.
      int val =  !this->SliceNode->GetUseLabelOutline(); 
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
        
        if (sgui)
          {
          vtkDebugMacro("Linked: using this slice node's label outline to toggle to: " << val);
          sgui->GetSliceNode()->SetUseLabelOutline(val);
          }
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // toggle the label map outline
      if (sgui)
        {
        int val = !this->SliceNode->GetUseLabelOutline(); 
        vtkDebugMacro("Not linked: using this slice node's label outline to toggle to: " << val);
        this->SliceNode->SetUseLabelOutline(val);        
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ToggleReformatWidget()
{
  int link;

  //--- swallow the menu event
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->SetAbortFlag(1);
    }

  //
  // --- Find out whether SliceViewers are linked or unlinked
  // --- so we know how to handle control.
  //
  if ( this->SliceCompositeNode )
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }


  this->ToggleReformatWidget ( link );
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ToggleReformatWidget( int link)
{
  vtkSlicerSlicesGUI *ssgui = 0;
  vtkSlicerSliceGUI *sgui = 0;
  vtkSlicerApplication *app = 0;
  vtkSlicerApplicationGUI *appGUI = 0;
  int found = 0;
    
  // find the sliceGUI for this controller
  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  appGUI = ssgui->GetApplicationGUI ( );


  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();

    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
        
      if (sgui)
        {
        if (sgui->GetSliceNode() == this->SliceNode )
          {
          found = 1;
          break;
          }
        }
      }
    }
  else
    {
    return;
    }
    
  if ( found )
    {
    if ( link )
      {
      // First save all SliceNodes for undo:
      vtkCollection *nodes = vtkCollection::New();
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
        
        if (sgui)
          nodes->AddItem ( sgui->GetSliceNode ( ) );
        }
      
      this->MRMLScene->SaveStateForUndo ( nodes );
      nodes->Delete ( );

      // When slice nodes are linked, only allow one slice node's reformat widget to be on at a time
      int val = this->SliceNode->GetWidgetVisible();
      // If slice node's reformat widget was on, just turn all of them off
      if (val == 1)
        {
        for (int i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            sgui = ssgui->GetFirstSliceGUI();
            layoutname = ssgui->GetFirstSliceGUILayoutName();
            }
          else
            {
            sgui = ssgui->GetNextSliceGUI(layoutname);
            layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
            }
          
          if (sgui)
            sgui->GetSliceNode()->SetWidgetVisible (!val);
          }
        }
      // If slice node's reformat widget was off, turn it on and turn all the other ones off
      else
        {
        for (int i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            sgui = ssgui->GetFirstSliceGUI();
            layoutname = ssgui->GetFirstSliceGUILayoutName();
            }
          else
            {
            sgui = ssgui->GetNextSliceGUI(layoutname);
            layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
            }
          
          if (sgui)
            {
            if ( strcmp(this->SliceNode->GetLayoutName(), layoutname) == 0 )
              sgui->GetSliceNode()->SetWidgetVisible (!val);
            else
              sgui->GetSliceNode()->SetWidgetVisible (val);
            }
          }
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // Toggle the visibility state of this slice node's reformat widget.
      this->SliceNode->SetWidgetVisible ( !(this->SliceNode->GetWidgetVisible()) );
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::FitSliceToBackground ( )
{
  int link;
  //--- swallow the menu event
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->SetAbortFlag(1);
    }

  //
  // --- Find out whether SliceViewers are linked or unlinked
  // --- so we know how to handle control.
  //
  if ( this->SliceCompositeNode && !this->MRMLScene->GetIsUpdating())
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }
  this->FitSliceToBackground ( link );
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::FitSliceToBackground ( int link )
{

  vtkSlicerSlicesGUI *ssgui;
  vtkSlicerSliceGUI *sgui;
  vtkSlicerApplication *app;
  vtkSlicerApplicationGUI *appGUI;
  int found = 0;
    
  // find the sliceGUI for this controller
  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  appGUI = ssgui->GetApplicationGUI ( );

  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
        
      if (sgui)
        {
        if (sgui->GetSliceNode() == this->SliceNode )
          {
          found = 1;
          break;
          }
        }
      }
    }
  else
    {
    return;
    }

  if ( found )
    {
    if ( link )
      {
      // First save all SliceNodes for undo:
      vtkCollection *nodes = vtkCollection::New();
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
            
        if (sgui)
          nodes->AddItem ( sgui->GetSliceNode ( ) );
        }
        
      this->MRMLScene->SaveStateForUndo ( nodes );
      nodes->Delete ( );
        
      // Now fit all Slices to background
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
            
        if (sgui)
          {
          int *rSize = sgui->GetSliceViewer()->GetRenderWidget()
            ->GetRenderer()->GetSize();
          sgui->GetLogic()->FitSliceToAll ( rSize[0], rSize[1] );
          sgui->GetSliceNode()->UpdateMatrices( );
          appGUI->GetSlicesControlGUI()->RequestFOVEntriesUpdate();
          }
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // Fit to the size of viewport 0 (for the LightBox)
      int *rSize = sgui->GetSliceViewer()->GetRenderWidget()
        ->GetRenderer()->GetSize();
      sgui->GetLogic()->FitSliceToAll ( rSize[0], rSize[1] );
      this->SliceNode->UpdateMatrices( );
      appGUI->GetSlicesControlGUI()->RequestFOVEntriesUpdate();      
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::RotateSliceToBackground ( )
{
  int link;
  //--- swallow the menu event
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->SetAbortFlag(1);
    }

  //
  // --- Find out whether SliceViewers are linked or unlinked
  // --- so we know how to handle control.
  //
  if ( this->SliceCompositeNode && !this->MRMLScene->GetIsUpdating())
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }
  this->RotateSliceToBackground ( link );
}


//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::RotateSliceToBackground ( int link )
{

  vtkSlicerSlicesGUI *ssgui;
  vtkSlicerSliceGUI *sgui;
  vtkSlicerApplication *app;
  vtkSlicerApplicationGUI *appGUI;
  int found = 0;
    
  // find the sliceGUI for this controller
  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  appGUI = ssgui->GetApplicationGUI ( );

  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
        
      if (sgui)
        {
        if (sgui->GetSliceNode() == this->SliceNode )
          {
          found = 1;
          break;
          }
        }
      }
    }
  else
    {
    return;
    }

  if ( found )
    {
    if ( link )
      {
      // First save all SliceNodes for undo:
      vtkCollection *nodes = vtkCollection::New();
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
            
        if (sgui)
          nodes->AddItem ( sgui->GetSliceNode ( ) );
        }
        
      this->MRMLScene->SaveStateForUndo ( nodes );
      nodes->Delete ( );
        
      // Now fit all Slices to background
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }
            
        if (sgui)
          {
          vtkMRMLVolumeNode *backgroundNode = sgui->GetLogic()->GetLayerVolumeNode(0);
          if ( backgroundNode )
            {
            sgui->GetSliceNode()->RotateToVolumePlane( backgroundNode );
            }
          }
        }
      }
    else
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      vtkMRMLVolumeNode *backgroundNode = sgui->GetLogic()->GetLayerVolumeNode(0);
      if ( backgroundNode )
        {
        sgui->GetSliceNode()->RotateToVolumePlane( backgroundNode );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ProcessWidgetEvents ( vtkObject *caller, 
  unsigned long event, void * vtkNotUsed(callData) ) 
{ 

  //
  // Is this Slice controller linked to others?
  //
  int link, i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  vtkMRMLSliceNode *snode;
  int numRows, numColumns;
  
  //
  // --- Find out whether SliceViewers are linked or unlinked
  // --- so we know how to handle control.
  //
  if ( this->SliceCompositeNode && !this->MRMLScene->GetIsUpdating())
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }

  // Is the slice node a CompareView node?
  bool isCompareView = false;
  if (this->SliceNode && (strncmp(this->SliceNode->GetSingletonTag(), "Compare", 7) == 0))
    {
    isCompareView = true;
    }

  
  //
  // --- Get a route to all SliceGUI's SliceNodes and SliceCompositeNodes in case of link
  //
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  vtkSlicerSlicesGUI *sgui0 = NULL;
  vtkMRMLLayoutNode *layout = NULL;
  if (app)
    {
    sgui0 = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName("Slices"));
    if (app->GetApplicationGUI())
      {
      layout = app->GetApplicationGUI()->GetGUILayoutNode();
      }
    }
  
  //
  // --- Set the slice node to null if it no longer exists in the MRML scene.
  //
  if (this->SliceNode != NULL &&
      this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    this->SetSliceNode(NULL);
    }
  //
  // --- Set the slice composite node to null if it no longer exists in the scene.
  //
  if (this->SliceCompositeNode != NULL &&
      this->MRMLScene->GetNodeByID(this->SliceCompositeNode->GetID()) == NULL)
    {
    this->SetSliceCompositeNode(NULL);
    }
  
  //
  // --- Update orientation if needed
  //
  if ( this->SliceNode != NULL && 
       vtkKWMenu::SafeDownCast(caller) == this->OrientationSelector->GetWidget()->GetWidget()->GetMenu() )
    {
    if (sgui0)
      this->UpdateOrientation ( link );
    }
  
  //
  // Update Foreground if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->ForegroundSelector )
    {
    if ( sgui0 )
      {
      this->UpdateForegroundLayer ( link );
      this->ForegroundSelector->SetBalloonHelpString ("Select the foreground");
      }
    }
  
  //
  // Update Background if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->BackgroundSelector )
    {
    if ( sgui0 )
      {
      this->UpdateBackgroundLayer ( link );
      this->BackgroundSelector->SetBalloonHelpString ("Select the background");
      }
    }
  
  
  //
  // Update Label layer if needed
  //
  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->LabelSelector )
    {
    if (sgui0)
      {
      this->UpdateLabelLayer ( link );
      this->LabelSelector->SetBalloonHelpString ("Select the label map");
      }
    }
  
  if ( !this->SliceNode)
    {
    return;
    }
  
  //
  // Was event invoked by other widgets? (button, scale, entry, menu)?
  //
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWScale *scale0 = vtkKWScale::SafeDownCast (caller);
//  vtkKWEntry *entry = vtkKWEntry::SafeDownCast(caller);
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast(caller);
  
  // Toggle the SliceNode's visibility.
  //--- if slice viewers are linked, modify all Controller's SliceNodes
  int vis = this->SliceNode->GetSliceVisible();
  if ( button == this->GetVisibilityToggle() && event == vtkKWPushButton::InvokedEvent )
    {
    if ( link )
      {
      nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
      
      // if compareview, send only compareview slices to 3D main viewer; otherwise, only send red, yellow, and green to 3D main viewer 
      if ( layout->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutCompareView 
           && ((strncmp(this->SliceNode->GetLayoutName(), "Compare", 7) == 0)
               || (strcmp(this->SliceNode->GetLayoutName(), "Red") == 0)))
        {
        for ( i=0; i<nnodes; i++)
          {
          snode = vtkMRMLSliceNode::SafeDownCast (
            this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
          
          if ((strncmp(snode->GetLayoutName(), "Compare", 7) == 0)
              || (strcmp(snode->GetLayoutName(), "Red") == 0))
            {
            this->MRMLScene->SaveStateForUndo ( snode );
            snode->SetSliceVisible ( !vis );
            }
          }
        }
      else
        {
        for ( i=0; i<nnodes; i++)
          {
          snode = vtkMRMLSliceNode::SafeDownCast (
            this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
          
          if (strncmp(snode->GetLayoutName(), "Compare", 7) != 0)
            {            
            this->MRMLScene->SaveStateForUndo ( snode );
            snode->SetSliceVisible ( !vis );
            }
          }
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
  if ( button == this->GetLinkButton() && event == vtkKWPushButton::InvokedEvent )
    {
    this->MRMLScene->SaveStateForUndo ( this->SliceNode );
    this->ToggleSlicesLink ( );
    }
  else if ( button == this->LabelOpacityButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->PopUpLabelOpacityScaleAndEntry();
    }
  else if ( button == this->LabelOpacityToggleButton &&  event == vtkKWPushButton::InvokedEvent )
    {
    float s = this->LabelOpacityScale->GetValue();
    if (s != 0.0)
      {
      // save this opacity
      this->LastLabelOpacity = s;
      }
    if (s == 0.0)
      {
      this->LabelOpacityScale->SetValue(this->LastLabelOpacity);
      }
    else
      {
      this->LabelOpacityScale->SetValue(0.0);
      }
    }
  else if ( button == this->FitToWindowButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->FitSliceToBackground ( link );
    }
  else if ( button == this->LabelOutlineToggleButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->ToggleLabelOutline ( link );
    }
  

  //
  // Raise volumes module if adjustment to Volume display is requested.
  //
  if ( menu == this->VolumeDisplayMenuButton->GetMenu() &&
       event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    char *id = NULL;
    if ( !strcmp (this->VolumeDisplayMenuButton->GetValue(), "Foreground volume") )
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // raise volumes module with foreground
      id = this->SliceCompositeNode->GetForegroundVolumeID( );
      /*if ( id )
        {
        this->RaiseVolumeDisplayPanel ( id );
        }*/
      }
    else if (!strcmp (this->VolumeDisplayMenuButton->GetValue(), "Background volume"))
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // raise volumes module with background
      id = this->SliceCompositeNode->GetBackgroundVolumeID( );
      /*if ( id )
        {
        this->RaiseVolumeDisplayPanel ( id );
        }*/
      }
    else if (!strcmp(this->VolumeDisplayMenuButton->GetValue(), "Label map"))
      {
      this->MRMLScene->SaveStateForUndo ( this->SliceNode );
      // raise volumes module with label
      id = this->SliceCompositeNode->GetLabelVolumeID( );
      /*if ( id )
        {
        this->RaiseVolumeDisplayPanel ( id );
        }*/
      }
    }
  else if ( menu == this->ForegroundMenuButton->GetMenu() &&
            event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    //
    // check to see if this layer's associated VolumeDisplayNode has
    // a different value for interpolation than checkbutton indicates.
    //
    int interp = this->ForegroundMenuButton->GetMenu()->GetItemSelectedState ( "interpolation" );
    if ( this->SliceLogic )
      {
      if ( this->SliceLogic->GetForegroundLayer() )
        {
        vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->SliceLogic->GetForegroundLayer()->GetVolumeDisplayNode());
        
        if ( displayNode )
          {
          if ( interp != displayNode->GetInterpolate() )
            {
            // save state for undo and modify the node's value to match GUI
            this->MRMLScene->SaveStateForUndo ( this->SliceLogic->GetForegroundLayer()->GetVolumeDisplayNode() );
            displayNode->SetInterpolate ( interp );
            // need this to propagate change thru to the VolumeDisplayWidget's GUI
            if ( this->SliceLogic->GetForegroundLayer()->GetVolumeNode() )
              {
              this->SliceLogic->GetForegroundLayer()->GetVolumeNode()->Modified();
              }
            }
          }
        }
      }
    // change other slice controller widgets if linked; go thru the SlicesGUI Map
    if ( link )
      {
      vtkSlicerSlicesGUI *ssgui = vtkSlicerSlicesGUI::SafeDownCast( app->GetModuleGUIByName("Slices") );
      vtkSlicerSliceGUI *sgui;
      if (ssgui)
        {
        const char *layoutname = NULL;
        int nSliceGUI = ssgui->GetNumberOfSliceGUI();
        for (i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            sgui = ssgui->GetFirstSliceGUI();
            layoutname = ssgui->GetFirstSliceGUILayoutName();
            }
          else
            {
            sgui = ssgui->GetNextSliceGUI(layoutname);
            layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
            }
          
          if ( sgui->GetLogic() )
            {
            if  ( sgui->GetLogic()->GetForegroundLayer() )
              {
              vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(sgui->GetLogic()->GetForegroundLayer()->GetVolumeDisplayNode());
              if ( displayNode )
                {
                if ( interp != displayNode->GetInterpolate() )
                  {
                  this->MRMLScene->SaveStateForUndo ( displayNode );
                  displayNode->SetInterpolate ( interp );
                  // need this to propagate change thru to the VolumeDisplayWidget's GUI
                  if ( sgui->GetLogic()->GetForegroundLayer()->GetVolumeNode() )
                    {
                    sgui->GetLogic()->GetForegroundLayer()->GetVolumeNode()->Modified();
                    }
                  }
                }
              }
            }
          }
        }
      }
    }  
  else if ( menu == this->BackgroundMenuButton->GetMenu() &&
            event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    // check to see if this layer's associated VolumeDisplayNode has
    // a different value for interpolation than checkbutton indicates.
    //
    int interp = this->BackgroundMenuButton->GetMenu()->GetItemSelectedState ( "interpolation" );
    if ( this->SliceLogic )
      {
      if ( this->SliceLogic->GetBackgroundLayer() )
        {
        vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->SliceLogic->GetBackgroundLayer()->GetVolumeDisplayNode());
        if ( displayNode )
          {
          if ( interp != displayNode->GetInterpolate() )
            {
            // save state for undo and modify the node's value to match GUI
            this->MRMLScene->SaveStateForUndo ( displayNode );
            displayNode->SetInterpolate ( interp );
            // need this to propagate change thru to the VolumeDisplayWidget's GUI
            if ( this->SliceLogic->GetBackgroundLayer()->GetVolumeNode() )
              {
              this->SliceLogic->GetBackgroundLayer()->GetVolumeNode()->Modified();
              }
            }
          }
        }
      }
    // change other slice controller widgets if linked; go thru the SlicesGUI Map
    if ( link )
      {
      vtkSlicerSlicesGUI *ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
      vtkSlicerSliceGUI *sgui;
      if (ssgui)
        {
        const char *layoutname = NULL;
        int nSliceGUI = ssgui->GetNumberOfSliceGUI();
        for (i = 0; i < nSliceGUI; i++)
          {
          if (i == 0)
            {
            sgui = ssgui->GetFirstSliceGUI();
            layoutname = ssgui->GetFirstSliceGUILayoutName();
            }
          else
            {
            sgui = ssgui->GetNextSliceGUI(layoutname);
            layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
            }
          
          if ( sgui->GetLogic() )
            {
            if  ( sgui->GetLogic()->GetBackgroundLayer() )
              {
              vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(sgui->GetLogic()->GetBackgroundLayer()->GetVolumeDisplayNode());
              if ( displayNode )
                {
                if ( interp != displayNode->GetInterpolate() )
                  {
                  this->MRMLScene->SaveStateForUndo ( displayNode );
                  displayNode->SetInterpolate ( interp );
                  // need this to propagate change thru to the VolumeDisplayWidget's GUI
                  if ( sgui->GetLogic()->GetBackgroundLayer()->GetVolumeNode() )
                    {
                    sgui->GetLogic()->GetBackgroundLayer()->GetVolumeNode()->Modified();
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  else if ( menu == this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Compositing")) &&
            event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    int cindex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Compositing");
    vtkKWMenu *cmenu = this->MoreMenuButton->GetMenu()->GetItemCascade(cindex);
    int numItems = cmenu->GetNumberOfItems();
    int found = 0;
    int item;
    for ( item=0; item < numItems; item++)
      {
      if ( cmenu->GetItemSelectedState(item) )
        {
        found = 1;
        break;
        }
      }
    //--- if one of the compositing items is selected,
    //--- check compositing node for same value.
    if ( found )
      {
      const char *lbstr = cmenu->GetItemLabel ( item );
      if ( !strcmp ( lbstr, "Alpha blend") )
        {
        if (link)
          {
          //--- check all linked nodes in case their compositing setting is stale.
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
              this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            
            //--- only update the node if the value is out of sync
            if ( cnode->GetCompositing() != vtkMRMLSliceCompositeNode::Alpha )
              {
              this->MRMLScene->SaveStateForUndo ( cnode );
              cnode->SetCompositing (vtkMRMLSliceCompositeNode::Alpha );
              }
            }
          }
        else
          {
          //--- only update the node if the value is out of sync
          if ( this->SliceCompositeNode->GetCompositing() != vtkMRMLSliceCompositeNode::Alpha )
            {
            this->MRMLScene->SaveStateForUndo( this->SliceCompositeNode );
            this->SliceCompositeNode->SetCompositing(vtkMRMLSliceCompositeNode::Alpha);
            }
          }
        }
      else if ( !strcmp ( lbstr, "Reverse alpha blend") )
        {
        if (link)
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
              this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            
            this->MRMLScene->SaveStateForUndo ( cnode );
            cnode->SetCompositing (vtkMRMLSliceCompositeNode::ReverseAlpha );
            }
          }
        else
          {
          this->MRMLScene->SaveStateForUndo( this->SliceCompositeNode );
          this->SliceCompositeNode
            ->SetCompositing(vtkMRMLSliceCompositeNode::ReverseAlpha);
          }
        }
      else if (!strcmp ( lbstr, "Add") )
        {
        if (link)
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
              this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            
            //--- only update the node if the value is out of sync
            if ( cnode->GetCompositing() != vtkMRMLSliceCompositeNode::Add )
              {
              this->MRMLScene->SaveStateForUndo ( cnode );
              cnode->SetCompositing (vtkMRMLSliceCompositeNode::Add );
              }
            }
          }
        else
          {
          //--- only update the node if the value is out of sync
          if ( this->SliceCompositeNode->GetCompositing() != vtkMRMLSliceCompositeNode::Add )
            {
            this->MRMLScene->SaveStateForUndo( this->SliceCompositeNode );
            this->SliceCompositeNode->SetCompositing(vtkMRMLSliceCompositeNode::Add);
            }
          }
        }
      else if (!strcmp ( lbstr, "Subtract") )
        {
        if (link)
          {
          nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
          for ( i=0; i<nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
              this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
            
            //--- only update the node if the value is out of sync
            if ( cnode->GetCompositing() != vtkMRMLSliceCompositeNode::Subtract )
              {
              this->MRMLScene->SaveStateForUndo ( cnode );
              cnode->SetCompositing (vtkMRMLSliceCompositeNode::Subtract );
              }
            }
          }
        else
          {
          //--- only update the node if the value is out of sync
          if ( this->SliceCompositeNode->GetCompositing() != vtkMRMLSliceCompositeNode::Subtract )
            {
            this->MRMLScene->SaveStateForUndo( this->SliceCompositeNode );
            this->SliceCompositeNode->SetCompositing(vtkMRMLSliceCompositeNode::Subtract);
            }
          }
        }
      }
    }
  else if ( menu == this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Slice spacing mode")) &&
            event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    int cindex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Slice spacing mode");
    vtkKWMenu *cmenu = this->MoreMenuButton->GetMenu()->GetItemCascade(cindex);
    int numItems = cmenu->GetNumberOfItems();
    int found = 0;
    int item;
    for ( item=0; item < numItems; item++)
      {
      if ( cmenu->GetItemSelectedState(item) )
        {
        found = 1;
        break;
        }
      }
    if ( found )
      {
      const char *lbstr = cmenu->GetItemLabel ( item );
      if ( !strcmp ( lbstr, "Automatic") )
        {
        if (this->SliceNode->GetSliceSpacingMode() != vtkMRMLSliceNode::AutomaticSliceSpacingMode)
          {
          this->MRMLScene->SaveStateForUndo( this->SliceNode );
          this->SliceNode->SetSliceSpacingModeToAutomatic();
          }
        }
      else if ( !strcmp( lbstr, "Manual") )
        {
        this->PopUpPrescribedSliceSpacingEntry();
        }
      }
    }
  else if ( menu == this->MoreMenuButton->GetMenu()->GetItemCascade (this->MoreMenuButton->GetMenu()->GetIndexOfItem("Lightbox view")) &&
            event == vtkKWMenu::MenuItemInvokedEvent && app )
    {
    int cindex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Lightbox view");
    vtkKWMenu *cmenu = this->MoreMenuButton->GetMenu()->GetItemCascade(cindex);
    int numItems = cmenu->GetNumberOfItems();
    int found = 0;
    int item;
    for ( item=0; item < numItems; item++)
      {
      if ( cmenu->GetItemSelectedState(item) )
        {
        found = 1;
        break;
        }
      }
    if ( found )
      {
      const char *lbstr = cmenu->GetItemLabel ( item );
      if ( !strcmp ( lbstr, "1x1 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1, 1);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 1);
          }
        }
      else if ( !strcmp ( lbstr, "2x2 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(2, 2);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(2, 2);
          }
        }
      else if ( !strcmp ( lbstr, "3x3 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(3, 3);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(3, 3);
          }
        }
      else if ( !strcmp ( lbstr, "6x6 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(6, 6);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(6, 6);
          }
        }
      else if ( !strcmp ( lbstr, "1x2 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1, 2);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 2);
          }
        }
      else if ( !strcmp ( lbstr, "1x3 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1,3);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 3 );
          }
        }
      else if ( !strcmp ( lbstr, "1x4 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1, 4);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 4);
          }
        }
      else if ( !strcmp ( lbstr, "1x6 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1, 6);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 6);
          }
        }
      else if ( !strcmp ( lbstr, "1x8 view") )
        {
        if ( link && sgui0 && isCompareView)
          {
          // apply this reformat to all slice MRML
          this->SliceViewerLayoutConfig(1, 8);
          }
        else
          {
          // apply this reformat to only this slice MRML
          this->SliceNode->SetLayoutGrid(1, 8);
          }
        }


      else if ( !strcmp (lbstr, "customized view" ) )
        {
        this->PopUpLightboxCustomLayoutFrame();
        }
      }
    }
  else if ( (menu == this->LightboxButton->GetMenu()  ||
             event == vtkKWMenu::MenuItemInvokedEvent) && app )
    {
    //
    // Lightbox view functionality developing here.
    //
    if ( !strcmp ( this->LightboxButton->GetValue(), "1x1 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all slice MRML
        this->SliceViewerLayoutConfig(1, 1);
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 1 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue(), "2x2 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig(2, 2);
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 2, 2 );
        }
      }
    else if  ( !strcmp ( this->LightboxButton->GetValue(), "3x3 view" ) )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig(3, 3);
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 3, 3 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "6x6 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig(6, 6);
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 6, 6 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "1x2 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig( 1, 2 );
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 2 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "1x3 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig( 1, 3 );
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 3 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "1x4 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig( 1, 4 );
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 4 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "1x6 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig( 1, 6 );
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 6 );
        }
      }
    else if ( !strcmp ( this->LightboxButton->GetValue (), "1x8 view") )
      {
      if ( link && sgui0 && isCompareView)
        {
        // apply this reformat to all CompareX slice MRMLs
        this->SliceViewerLayoutConfig( 1, 8 );
        }
      else
        {
        // apply this reformat to only this slice MRML
        this->SliceNode->SetLayoutGrid( 1, 8 );
        }
      }

    else if ( !strcmp ( this->LightboxButton->GetValue (), "customized view") )
      {
      // pop up a toplevel to specify NXM view
      PopUpLightboxCustomLayoutFrame();
      }
    }
  if ( button == this->LightboxApplyButton &&
       event == vtkKWPushButton::InvokedEvent )
    {
    numRows = this->LightboxRowsEntry->GetValueAsInt();
    numColumns = this->LightboxColumnsEntry->GetValueAsInt();
    if ( link && sgui0 && isCompareView)
      {
      // apply this reformat to all compare slice nodes
      this->SliceViewerLayoutConfig(numRows, numColumns);
      }
    else
      {
      // apply this reformat to only this slice MRML
      this->HideLightboxCustomLayoutFrame();
      this->SliceNode->SetLayoutGrid( numRows, numColumns );
      }
    this->HideLightboxCustomLayoutFrame();
    }

  if ( button == this->PrescribedSliceSpacingCancelButton &&
       event == vtkKWPushButton::InvokedEvent )
    {
    this->HidePrescribedSliceSpacingEntry();
    //--- node won't change, so
    //--- make sure to set the mode back to the node's value.
    int modeIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Slice spacing mode" );
    vtkKWMenu *smenu = this->MoreMenuButton->GetMenu()->GetItemCascade(modeIndex);
    if ((this->SliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode) &&
        (smenu->GetItemSelectedState("Automatic") == 0 ))
      {
      smenu->SelectItem ("Automatic");
      }
    else if  ((this->SliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode) &&
              (smenu->GetItemSelectedState("Manual") == 0 ))
      {
      smenu->SelectItem ("Manual");
      }
    }

  if ( button == this->PrescribedSliceSpacingApplyButton &&
       event == vtkKWPushButton::InvokedEvent )
    {
    if (this->SliceNode->GetSliceSpacingMode() != vtkMRMLSliceNode::PrescribedSliceSpacingMode)
      {
      this->MRMLScene->SaveStateForUndo( this->SliceNode );
      this->SliceNode->SetSliceSpacingModeToPrescribed();
      }

    std::stringstream ss;
    ss << this->PrescribedSliceSpacingEntry->GetWidget()->GetValue();
    double val;
    ss >> val;
    double *current = this->SliceNode->GetPrescribedSliceSpacing();
    double spacing[3];
    spacing[0] = current[0];
    spacing[1] = current[1];
    spacing[2] = val;
    this->SliceNode->SetPrescribedSliceSpacing(spacing);

    this->HidePrescribedSliceSpacingEntry();
    }
  
  //
  // Scales starting to move? save state for undo.
  //
  if ( this->OffsetScale == vtkKWScale::SafeDownCast( caller ) &&
       event == vtkKWScale::ScaleValueStartChangingEvent )
    {
    // set an undo state when the scale starts being dragged
    this->MRMLScene->SaveStateForUndo( this->SliceNode );
    this->OffsetScaleActive = true;

    // find the sliceGUI for this controller
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
    vtkSlicerSlicesGUI *ssgui = NULL;
    if (app!=NULL)
      {
      ssgui=vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
      }
    vtkSlicerSliceGUI *sgui=NULL; // slice GUI belonging to this widget
    if (ssgui!=NULL)
      {
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();    
      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }      
        if (sgui && sgui->GetSliceNode() == this->SliceNode )
          {
          // found - sgui is non-NULL, quit from the loop now
          break;
          }        
        // not found yet
        sgui=NULL;
        }

      // Set the focus to the renderer widget (that handles keyboard shortcuts and mouse wheel
      // events for changing slices)
      if ( sgui!=NULL && 
          sgui->GetSliceViewer()!=NULL &&
          sgui->GetSliceViewer()->GetRenderWidget()!=NULL &&
          sgui->GetSliceViewer()->GetRenderWidget()->GetVTKWidget()!=NULL )
        {
        sgui->GetSliceViewer()->GetRenderWidget()->GetVTKWidget()->Focus();
        }
      }
    }
  else if ( scale0 == this->LabelOpacityScale->GetWidget() 
            && event == vtkKWScale::ScaleValueStartChangingEvent )
    {
    if ( link && sgui0 ) 
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
    if ( link && sgui0 )
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

  //
  // Scales done moved or moving? update slice and snap to nearest integral slice location
  // - works around bugs described here:
  // http://www.na-mic.org/Bug/view.php?id=76
  // and here:
  // http://sourceforge.net/tracker/index.php?func=detail&aid=1899040&group_id=12997&atid=112997
  //

  // A scale changing event is only actioned on if the user is
  // interacting with the slider.  Otherwise, we rely on the entry
  // change event to manipulate the scale.
  if ( this->OffsetScale == vtkKWScale::SafeDownCast( caller ) &&
       this->OffsetScaleActive &&
       (event == vtkKWScale::ScaleValueChangedEvent ||
        event == vtkKWScale::ScaleValueChangingEvent) )
    {
    vtkKWScale *scale = vtkKWScale::SafeDownCast( caller );
    double value, offset;

    // get integer value from scale and convert to mm offset
    value = scale->GetValue();
    offset = this->OffsetScaleMin + (value * this->OffsetScaleResolution);

    // set the slice index entry to match the new offset
    this->SetSliceIndexEntryValueFromOffset(offset);

    // set the entry to match
    if (fabs(offset - this->OffsetEntry->GetValueAsDouble()) > 1.0e-6)
      {
      this->OffsetEntry->SetValueAsDouble(offset);
      modified = 1;
      }
 
    // if slice viewers are linked in CompareView layout mode,
    // modify all slice logic to synch all Compare Slice viewers
    // (don't sync on ScaleValueChangingEvent for performance reasons,
    // wait until the ScaleValueChangedEvent to propagate to the other viewers)
    if ( link && sgui0 && (event != vtkKWScale::ScaleValueChangingEvent) &&
         ((layout->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutCompareView) ||
          (layout->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView)) )
      {
      modified |= this->UpdateCompareView( offset );
      }
    else
      {
      // turn off linking for now and set it back to its original
      // value when done modifying the node
      this->SliceCompositeNode->SetLinkedControl(0);
      double oldValue = this->SliceLogic->GetSliceOffset();
      if (fabs(oldValue - offset) > 1.0e-6)
        {
        this->SliceLogic->SetSliceOffset( offset );
        modified = 1;
        }
      this->SliceCompositeNode->SetLinkedControl(link);
      }
    
    // end event of the user interactiving with the slider
    if (event == vtkKWScale::ScaleValueChangedEvent)
      {
      this->OffsetScaleActive = false;
      }
    }

  // An entry change event is only actioned if the user is not
  // interacting with the slider. The entry change event will modify
  // the scale.
  if ( !this->OffsetScaleActive && this->OffsetEntry == vtkKWEntry::SafeDownCast( caller ) )
    {
    vtkKWEntry *entry = vtkKWEntry::SafeDownCast( caller );
    double newValue = entry->GetValueAsDouble();
    double value = this->SliceLogic->GetSliceOffset();

    if ( fabs(value - newValue) > 1.0e-6 )
      {
      // if slice viewers are linked in CompareView layout mode,
      // modify all slice logic to synch all Compare Slice viewers
      if ( link && sgui0 &&
         ((layout->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutCompareView) ||
          (layout->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView)) )
        {
        modified |= this->UpdateCompareView( newValue );
        }
      else
        {
        this->SliceLogic->SetSliceOffset( newValue );
        modified = 1;
        }

      // convert to slice number
      int slice = static_cast<int> (0.5 + (newValue - this->OffsetScaleMin) / this->OffsetScaleResolution);
      this->OffsetScale->SetValue(slice);
      }
    }
  
  if ( modified )
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::SliceViewerLayoutConfig(int nRows, int nColumns)
{
  char *pTag = NULL;
  vtkMRMLSliceNode *snode;
  int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceNode");          
  for (int i=0; i<nnodes; i++)
    {
    snode = vtkMRMLSliceNode::SafeDownCast (
      this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceNode"));
    
    pTag = snode->GetSingletonTag();
    switch (pTag[0])
      {
      case 'C':
        snode->SetLayoutGrid( nRows, nColumns );
        break;
        //default:
      }
    }
}

//---------------------------------------------------------------------------
int vtkSlicerSliceControllerWidget::UpdateCompareView ( double newValue )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  vtkSlicerSlicesGUI *ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  vtkSlicerSliceGUI *sgui;
  int modified = 0;

  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    int i;
    for (i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
      
      if ( strncmp(layoutname, "Compare", 7) != 0
           && strcmp(layoutname, "Red") != 0 )
        continue;
      
      if ( sgui->GetLogic() &&  sgui->GetSliceNode() &&
           !strcmp(this->SliceNode->GetOrientationString(), sgui->GetSliceNode()->GetOrientationString()))
        {
        double oldValue = sgui->GetLogic()->GetSliceOffset();
        if (fabs(oldValue - newValue) > 1.0e-6)
          {
          // turn off linking while modifying the node
          int link 
            = sgui->GetLogic()->GetSliceCompositeNode()->GetLinkedControl();
          sgui->GetLogic()->GetSliceCompositeNode()->SetLinkedControl(0);
          sgui->GetLogic()->SetSliceOffset( newValue );
          sgui->GetLogic()->GetSliceCompositeNode()->SetLinkedControl(link);
          modified = 1;
          }
        }
      }
    }
  return (modified);
}

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::HideLightboxCustomLayoutFrame ( )
{
  if ( !this->LightboxTopLevel )
    {
    return;
    }
  this->LightboxTopLevel->Withdraw();

}

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PopUpLightboxCustomLayoutFrame()
{
  if ( !this->MoreMenuButton || !this->MoreMenuButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, px, py, ph;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  
//  vtkKWTkUtilities::GetMousePointerCoordinates(this->LightboxButton, &x, &y);
  vtkKWTkUtilities::GetMousePointerCoordinates(this->MoreMenuButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->MoreMenuButton, &px, &py);
  vtkKWTkUtilities::GetWidgetSize(this->MoreMenuButton, NULL, &ph);
 
  this->LightboxTopLevel->SetPosition(px-ph, py+ph);
  app->ProcessPendingEvents();
  this->LightboxTopLevel->DeIconify();
  this->LightboxTopLevel->Raise();

}


//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::HideLabelOpacityScaleAndEntry (  )
{
  int link;
  if ( this->SliceCompositeNode && !this->MRMLScene->GetIsUpdating())
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }

  this->HideLabelOpacityScaleAndEntry ( link );
}


//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::HideLabelOpacityScaleAndEntry ( int link )
{
  if ( !this->LabelOpacityTopLevel )
    {
    return;
    }

  vtkSlicerSliceGUI *sgui;
  vtkSlicerApplication *app;
  vtkSlicerSlicesGUI *ssgui;

  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );

  if ( link )
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
        
      if (sgui)
        {
        sgui->GetSliceController()->LabelOpacityTopLevel->Withdraw();
        }
      }
    }
  else
    {
    this->LabelOpacityTopLevel->Withdraw();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PopUpLabelOpacityScaleAndEntry ( )
{
  // check to see what the current link status is.
  //--- swallow the menu event
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->SetAbortFlag(1);
    }

  int link;
  if ( this->SliceCompositeNode && !this->MRMLScene->GetIsUpdating())
    {
    link = this->SliceCompositeNode->GetLinkedControl ( );
    }
  else
    {
    link = 0;
    }
  this->PopUpLabelOpacityScaleAndEntry ( link );
  
}

//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PopUpLabelOpacityScaleAndEntry ( int link )
{
  if ( !this->LabelOpacityButton || !this->LabelOpacityButton->IsCreated())
    {
    return;
    }

  vtkSlicerSlicesGUI *ssgui;
  vtkSlicerSliceGUI *sgui;
  vtkSlicerApplication *app;
  vtkSlicerApplicationGUI *appGUI;
  int found = 0;
  int x, y, px, py, pw, ph, scx, scy, sx, sy;  
    
  // find the sliceGUI for this controller
  app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
  appGUI = ssgui->GetApplicationGUI ( );

  if (ssgui)
    {
    const char *layoutname = NULL;
    int nSliceGUI = ssgui->GetNumberOfSliceGUI();
    for (int i = 0; i < nSliceGUI; i++)
      {
      if (i == 0)
        {
        sgui = ssgui->GetFirstSliceGUI();
        layoutname = ssgui->GetFirstSliceGUILayoutName();
        }
      else
        {
        sgui = ssgui->GetNextSliceGUI(layoutname);
        layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
        }
        
      if (sgui)
        {
        if (sgui->GetSliceNode() == this->SliceNode )
          {
          found = 1;
          break;
          }
        }
      }
    }
  else
    {
    return;
    }
  
  if ( found )
    {
    if ( link )
      {
      const char *layoutname = NULL;
      int nSliceGUI = ssgui->GetNumberOfSliceGUI();

      for (int i = 0; i < nSliceGUI; i++)
        {
        if (i == 0)
          {
          sgui = ssgui->GetFirstSliceGUI();
          layoutname = ssgui->GetFirstSliceGUILayoutName();
          }
        else
          {
          sgui = ssgui->GetNextSliceGUI(layoutname);
          layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
          }

        if ( sgui != NULL)
          {
//        vtkKWTkUtilities::GetMousePointerCoordinates(sgui->GetSliceController()->GetLabelOpacityButton(), &x, &y);
          vtkKWTkUtilities::GetWidgetCoordinates(sgui->GetSliceController()->GetMoreMenuButton(), &px, &py);
          vtkKWTkUtilities::GetWidgetSize(sgui->GetSliceController()->GetMoreMenuButton(), &pw, &ph);
          vtkKWTkUtilities::GetWidgetRelativeCoordinates(sgui->GetSliceController()->GetLabelOpacityScale()->GetScale(), &sx, &sy);
          sscanf(this->Script("%s coords %g", sgui->GetSliceController()->GetLabelOpacityScale()->GetScale()->GetWidgetName(),
                              sgui->GetSliceController()->GetLabelOpacityScale()->GetScale()->GetValue()), "%d %d", &scx, &scy);
          // position the popup window in each slice controller
//        x -= sx + scx;
          x = px - pw - 3;
          y = py;
          if (py <= y && y <= (py + ph -1))
            {
            y = py + ph - 3;
            }
          else
            {
            y -= sy + scy;
            }
          sgui->GetSliceController()->GetLabelOpacityTopLevel()->SetPosition(x, y);
          app->ProcessPendingEvents();
          sgui->GetSliceController()->GetLabelOpacityTopLevel()->DeIconify();
          sgui->GetSliceController()->GetLabelOpacityTopLevel()->Raise();
          }
        }
      }
    else
      {
      vtkKWTkUtilities::GetWidgetCoordinates(this->GetMoreMenuButton(), &px, &py);
      vtkKWTkUtilities::GetWidgetSize(this->GetMoreMenuButton(), &pw, &ph);
      vtkKWTkUtilities::GetWidgetRelativeCoordinates(this->LabelOpacityScale->GetScale(), &sx, &sy);
      sscanf(this->Script("%s coords %g", this->LabelOpacityScale->GetScale()->GetWidgetName(),
                          this->LabelOpacityScale->GetScale()->GetValue()), "%d %d", &scx, &scy);
      // position the popup window in this slice controller 
      x = px - pw - 3;
      y = py;
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
    }
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
void vtkSlicerSliceControllerWidget::ProcessMRMLEvents ( vtkObject *caller, unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) ) 
{ 
  if (this->SliceNode != NULL && this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    // jvm std::cerr << "Setting slice node to null @a" << std::endl;
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

  std::ostringstream sstring;
  sstring << this->SliceNode->GetLayoutName() << ". Click to shrink/expand";
  this->ColorCodeButton->SetBalloonHelpString(sstring.str().c_str());

  // 
  // Update the menu to match the node
  //
  vtkKWMenuButton *mb = this->OrientationSelector->GetWidget()->GetWidget();
  mb->SetValue( this->SliceNode->GetOrientationString() );

  //
  // Make the Balloon help hint about which way scrolling works.
  //
  if ( !(strcmp(this->SliceNode->GetOrientationString(), "Axial")))
    {
    // Orientation is Axial: I <----> S
    this->OffsetScale->SetBalloonHelpString ( "I <-----> S" );
    }
  else if ( !(strcmp(this->SliceNode->GetOrientationString(), "Sagittal")))
    {
    // Orientation is Sagittal: L <----> R
    this->OffsetScale->SetBalloonHelpString ( "L <-----> R" );
    }
  else if ( !(strcmp(this->SliceNode->GetOrientationString(), "Coronal")))
    {
    // Orientation is Coronal: P <----> A
    this->OffsetScale->SetBalloonHelpString ( "P <-----> A" );
    }
  else
    {
    // Orientation is Oblique: make tooltip null
    this->OffsetScale->SetBalloonHelpString ( "Oblique" ) ;
    }

  //
  // Set the scale increments to match the z spacing (rotated
  // into slice space)
  //
  const double *sliceSpacing;
  sliceSpacing = this->SliceLogic->GetLowestVolumeSliceSpacing();

  this->OffsetScale->SetResolution(1);
  this->OffsetScaleResolution = sliceSpacing[2];

  //
  // Set the scale range to match the field of view
  // - calculate the number of slices in the current range
  //
  double sliceBounds[6];
  this->SliceLogic->GetLowestVolumeSliceBounds(sliceBounds);
  int slices = static_cast<int> (0.5 + ((sliceBounds[5] - sliceBounds[4]) / sliceSpacing[2]));

  this->OffsetScaleMin = sliceBounds[4];
  double min, max;
  this->OffsetScale->GetRange(min, max);
  if ( min != 0 || max != slices )
    {
    this->OffsetScale->SetRange(0, slices);
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
  // Update the label opacity toggle button to match the logic state
  //
  if (this->LabelOpacityScale->GetValue() > 0)
    {
    this->LabelOpacityToggleButton->SetImageToIcon(this->GetVisibilityIcons()->GetVisibleIcon ( ));
    }
  else
    {
    this->LabelOpacityToggleButton->SetImageToIcon(this->GetVisibilityIcons()->GetInvisibleIcon ( ));
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
  // get offset value in mm and check for slice number
  //
  double value = this->GetOffsetScale()->GetValue();
  double offset = this->OffsetScaleMin + (value * this->OffsetScaleResolution);
  double newOffset = this->SliceLogic->GetSliceOffset();
  int slice = static_cast<int> ( 0.5 + ((newOffset - this->OffsetScaleMin) / this->OffsetScaleResolution) );
  if ( offset != newOffset )
    {
    this->OffsetScale->SetValue( slice );
    }
  this->OffsetEntry->SetValueAsDouble(newOffset);

  // set the slice index entry to match the new offset
  SetSliceIndexEntryValueFromOffset(newOffset);

  //
  // when the composite node changes, update the menus to match
  //
  if ( this->SliceCompositeNode != NULL && caller == this->SliceCompositeNode )
    {
    //--- find out what the current menu selection is....
    std::string lbstr;
    if ( this->MoreMenuButton != NULL && this->MoreMenuButton->GetMenu() )
      {
      int cindex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Compositing");
      vtkKWMenu *cmenu = this->MoreMenuButton->GetMenu()->GetItemCascade(cindex);
      if ( cmenu )
        {
        int numItems = cmenu->GetNumberOfItems();
        int found = 0;
        int item;
        for ( item=0; item < numItems; item++)
          {
          if ( cmenu->GetItemSelectedState(item) )
            {
            found = 1;
            break;
            }
          }
        if ( found )
          {
          lbstr = cmenu->GetItemLabel ( item );
          }
        }
      //---
      //--- update the GUI if node's compositing mode setting has changed.
      //---
      if (( this->SliceCompositeNode->GetCompositing() == vtkMRMLSliceCompositeNode::Alpha ) &&
          ( strcmp (lbstr.c_str(), "Alpha blend")) )
        {
        cmenu->SelectItem ("Alpha blend" );
        }
      else if (( this->SliceCompositeNode->GetCompositing() == vtkMRMLSliceCompositeNode::Add ) &&
               ( strcmp (lbstr.c_str(), "Add")) )
        {
        cmenu->SelectItem ("Add" );
        }
      else if (( this->SliceCompositeNode->GetCompositing() == vtkMRMLSliceCompositeNode::Subtract ) &&
               ( strcmp (lbstr.c_str(), "Subtract")) )
        {
        cmenu->SelectItem ("Subtract" );
        }
      }
    }

  //if ( caller == this->SliceCompositeNode )
  //  {
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
  //}
    
  vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(caller);
  if ( snode != NULL && snode == this->GetSliceNode() )
    {
    int showIndex;
    int hideIndex;
    const char *imageName;
    const char *show = "Show reformat widget";
    const char *hide = "Hide reformat widget";
    showIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( show );
    hideIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( hide );
    imageName = "ReformatImage";
    if ( hideIndex >= 0 && snode->GetWidgetVisible() == 0)
      {
      //--- if the GUI gives option to hide the widget, but widget is already invisible,
      //--- update the GUI to give the option to show the widget.
      vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceWidgetOnIcon() );
      this->MoreMenuButton->GetMenu()->SetItemImage ( hideIndex, imageName);
      this->MoreMenuButton->GetMenu()->SetItemLabel ( hideIndex, show );
      }
    else if ( showIndex >= 0 && snode->GetWidgetVisible() == 1)
      {
      //--- if the GUI gives option to show the widget, but widget is already visible,
      //--- update the GUI to give the option to hide the widget.
      vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceWidgetOffIcon() );
      this->MoreMenuButton->GetMenu()->SetItemImage ( showIndex, imageName);
      this->MoreMenuButton->GetMenu()->SetItemLabel ( showIndex, hide );
      }

    const char *labelOutlineOn = "Show label volume outlines";
    const char *labelOutlineOff = "Don't show label volume outlines";
    int onIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( labelOutlineOn );
    int offIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ( labelOutlineOff );
    imageName = "SliceLabelOutlineImage";
    if (offIndex >= 0 && snode->GetUseLabelOutline() == 0)
      {
      vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceLabelOutlineOnIcon() );
      this->MoreMenuButton->GetMenu()->SetItemImage ( offIndex, imageName);
      this->MoreMenuButton->GetMenu()->SetItemLabel ( offIndex, labelOutlineOn );
      }
    else if (onIndex >= 0 && snode->GetUseLabelOutline() == 1)
      {
      vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName,  this->SliceControlIcons->GetSliceLabelOutlineOffIcon() );
      this->MoreMenuButton->GetMenu()->SetItemImage ( onIndex, imageName);
      this->MoreMenuButton->GetMenu()->SetItemLabel ( onIndex, labelOutlineOff );
      }

    // update the slice spacing mode if required
    int modeIndex = this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Slice spacing mode" );
    vtkKWMenu *smenu = this->MoreMenuButton->GetMenu()->GetItemCascade(modeIndex);
    if (smenu && (snode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode))
      {
      if (smenu->GetItemSelectedState("Automatic") == 0 )
        {
        smenu->SelectItem( "Automatic" );
        }
      this->MoreMenuButton->GetMenu()->SetItemImageToIcon(modeIndex, this->SliceControlIcons->GetAutomaticSliceSpacingModeIcon());
      }
    else if (smenu && (snode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode))
      {
      if (smenu->GetItemSelectedState("Manual") == 0)
        {
        smenu->SelectItem( "Manual" );
        }
      this->MoreMenuButton->GetMenu()->SetItemImageToIcon(modeIndex, this->SliceControlIcons->GetManualSliceSpacingModeIcon());
      }
                                                                       
    // update the slice spacing entry
    this->PrescribedSliceSpacingEntry->GetWidget()->SetValue(snode->GetPrescribedSliceSpacing()[2]);

    //--- update widgets that configure lightbox.
    int lbRows = snode->GetLayoutGridRows();
    int lbCols = snode->GetLayoutGridColumns ( );
    vtkKWMenu *cmenu = this->MoreMenuButton->GetMenu()->
      GetItemCascade ( this->MoreMenuButton->GetMenu()->GetIndexOfItem ("Lightbox view"));
    if ( lbRows != this->LightboxRows || lbCols != this->LightboxColumns)
      {
      //--- update "Lightbox view" cascade menu inside MoreMenu
      //--- and update LightboxButton
      int item;
      if ( lbRows == 1 && lbCols == 1 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x1 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }
        }
      else if ( lbRows == 2 && lbCols == 2 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "2x2 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      else if ( lbRows == 3 && lbCols == 3 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "3x3 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      else if ( lbRows == 6 && lbCols == 6 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "6x6 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      else if ( lbRows == 1 && lbCols == 2 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x2 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      else if ( lbRows == 1 && lbCols == 3 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x3 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      else if ( lbRows == 1 && lbCols == 4 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x4 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }
        }

      else if ( lbRows == 1 && lbCols == 6 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x6 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }
        }
      else if ( lbRows == 1 && lbCols == 8 )
        {
        // button
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "1x8 view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }
        }
      else
        {
        // custom lightbox and entry widgets that specify it
        item = this->LightboxButton->GetMenu()->GetIndexOfItem ( "customized view");
        if ( this->LightboxButton->GetMenu()->GetItemSelectedState ( item ) == 0 )
          {
          this->LightboxButton->GetMenu()->SelectItem ( item );
          if ( this->LightboxRowsEntry->GetValueAsInt() != lbRows )
            {
            this->LightboxRowsEntry->SetValueAsInt ( lbRows);
            }
          if ( this->LightboxColumnsEntry->GetValueAsInt() != lbCols )
            {
            this->LightboxColumnsEntry->SetValueAsInt ( lbCols);
            }
          }
        // more menu
        if ( cmenu )
          {
          item = cmenu->GetIndexOfItem ( this->LightboxButton->GetValue() );
          if ( cmenu->GetItemSelectedState ( item ) == 0 )
            {
            cmenu->SelectItem ( item );
            }
          }

        }
      this->LightboxRows = lbRows;
      this->LightboxColumns = lbCols;
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
      this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinDown );
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
      this->ColorCodeButton->SetImageToPredefinedIcon (vtkKWIcon::IconSpinUp );
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

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::PopUpPrescribedSliceSpacingEntry()
{
  if ( !this->MoreMenuButton || !this->MoreMenuButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, position the popup

  int x, y, px, py, ph;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  
//  vtkKWTkUtilities::GetMousePointerCoordinates(this->LightboxButton, &x, &y);
  vtkKWTkUtilities::GetMousePointerCoordinates(this->MoreMenuButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->MoreMenuButton, &px, &py);
  vtkKWTkUtilities::GetWidgetSize(this->MoreMenuButton, NULL, &ph);
 
  this->PrescribedSliceSpacingTopLevel->SetPosition(px-ph, py+ph);
  app->ProcessPendingEvents();
  this->PrescribedSliceSpacingTopLevel->DeIconify();
  this->PrescribedSliceSpacingTopLevel->Raise();
  
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::HidePrescribedSliceSpacingEntry()
{
  if ( !this->PrescribedSliceSpacingTopLevel )
    {
    return;
    }
  this->PrescribedSliceSpacingTopLevel->Withdraw();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::SetSliceIndexEntryValueFromOffset(double sliceOffset)
{
  int sliceIndex=this->SliceLogic->GetSliceIndexFromOffset(sliceOffset);
  if (sliceIndex>0)
    {
    this->SliceIndexEntry->SetWidth(4);
    this->SliceIndexEntry->SetValueAsInt(sliceIndex);
    }
  else
    {
    if (sliceIndex==vtkMRMLSliceLogic::SLICE_INDEX_ROTATED)
      {
      // reformatted slice
      this->SliceIndexEntry->SetWidth(4);
      this->SliceIndexEntry->SetValue("R");
      }
    else if (sliceIndex==vtkMRMLSliceLogic::SLICE_INDEX_OUT_OF_VOLUME)
      {
      // out of volume
      this->SliceIndexEntry->SetWidth(4);
      this->SliceIndexEntry->SetValue("O");
      }
    else
      {
      // no volume or unknown status
      this->SliceIndexEntry->SetValue("");
      this->SliceIndexEntry->SetWidth(0);
      }
    }
}
