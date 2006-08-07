#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkImageData.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWScale.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"

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
  this->VisibilityIcons = NULL;
  this->SliceNode = NULL;
  this->SliceCompositeNode = NULL;
  this->ScaleFrame = NULL;
  this->ColorCodeFrame = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceControllerWidget::~vtkSlicerSliceControllerWidget ( ){

    if ( this->OffsetScale ) {
        this->OffsetScale->Delete ( );
        this->OffsetScale = NULL;
    }
    if ( this->OrientationMenu ) {
        this->OrientationMenu->Delete ( );
        this->OrientationMenu = NULL;
    }
    if ( this->ForegroundSelector ) {
        this->ForegroundSelector->Delete ( );
        this->ForegroundSelector = NULL;
    }
    if ( this->BackgroundSelector ) {
        this->BackgroundSelector->Delete ( );
        this->BackgroundSelector = NULL;
    }
    if ( this->LabelSelector ) {
        this->LabelSelector->Delete ( );
        this->LabelSelector = NULL;
    }
    if ( this->VisibilityToggle ) {
        this->VisibilityToggle->Delete  ( );
        this->VisibilityToggle = NULL;
    }
    if ( this->VisibilityIcons ) {
        this->VisibilityIcons->Delete  ( );
        this->VisibilityIcons = NULL;
    }
    if ( this->ScaleFrame )
      {
        this->ScaleFrame->Delete ( );
        this->ScaleFrame = NULL;
      }
    if ( this->ColorCodeFrame )
      {
        this->ColorCodeFrame->Delete ( );
        this->ColorCodeFrame = NULL;
      }

    this->SetSliceNode ( NULL );
    this->SetSliceCompositeNode ( NULL );
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


    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueChangedEvent, this->GUICallbackCommand );
    this->OffsetScale->GetWidget()->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, this->GUICallbackCommand );
    this->VisibilityToggle->RemoveObservers ( vtkKWPushButton::InvokedEvent, this->GUICallbackCommand );
        
}



//---------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ApplyColorCode ( double *c )
{
  this->ColorCodeFrame->SetBackgroundColor (c[0], c[1], c[2] );
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
    
    //
    // A stripe that color codes the SliceGUI this controller belongs to.
    //
    this->ColorCodeFrame = vtkKWFrame::New ( );
    this->ColorCodeFrame->SetParent ( this );
    this->ColorCodeFrame->Create ( );
    this->ColorCodeFrame->SetHeight ( 7 );
    
    //
    // Orientation  (TODO: make this into a vtkSlicerOrientationWidget)
    //
    this->OrientationMenu = vtkKWMenuButtonWithSpinButtonsWithLabel::New ();
    this->OrientationMenu->SetParent ( this );
    this->OrientationMenu->Create ( );    
    this->OrientationMenu->SetLabelWidth(3);

    this->OrientationMenu->SetLabelText ( "Or: ");
    vtkKWMenuButton *mb = this->OrientationMenu->GetWidget()->GetWidget();
    mb->SetWidth ( 8 );
    mb->GetMenu()->AddRadioButton ( "Axial" );
    mb->GetMenu()->AddRadioButton ( "Sagittal" );
    mb->GetMenu()->AddRadioButton ( "Coronal" );
    mb->SetValue ("Axial");    

    //
    // Foreground, Background, and Label selections
    //
    this->ForegroundSelector = vtkSlicerNodeSelectorWidget::New();
    this->ForegroundSelector->SetParent ( this );
    this->ForegroundSelector->Create ( );
    this->ForegroundSelector->NoneEnabledOn();
    this->ForegroundSelector->SetLabelText ("Fg:");
    this->ForegroundSelector->SetLabelWidth(3);
    this->ForegroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->ForegroundSelector->SetMRMLScene( this->MRMLScene );
    this->ForegroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);

    this->BackgroundSelector = vtkSlicerNodeSelectorWidget::New();
    this->BackgroundSelector->SetParent ( this );
    this->BackgroundSelector->Create ( );
    this->BackgroundSelector->NoneEnabledOn();
    this->BackgroundSelector->SetLabelText ("Bg:");
    this->BackgroundSelector->SetLabelWidth(3);
    this->BackgroundSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->BackgroundSelector->SetMRMLScene( this->MRMLScene );
    this->BackgroundSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);

    this->LabelSelector = vtkSlicerNodeSelectorWidget::New();
    this->LabelSelector->SetParent ( this );
    this->LabelSelector->Create ( );
    this->LabelSelector->NoneEnabledOn();
    this->LabelSelector->SetLabelText ("Lb:");
    this->LabelSelector->SetLabelWidth(3);
    this->LabelSelector->SetNodeClass ("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->LabelSelector->SetMRMLScene( this->MRMLScene );
    this->LabelSelector->GetWidget()->GetWidget()->SetMaximumLabelWidth(10);

    //
    // Create the frame to contain scale and visibility toggle
    //
    this->ScaleFrame = vtkKWFrame::New ();
    this->ScaleFrame->SetParent ( this );
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
    // Create a scale to control the slice number displayed
    //
    this->OffsetScale = vtkKWScaleWithEntry::New();
    this->OffsetScale->SetParent ( this->ScaleFrame );
    this->OffsetScale->Create();
    this->OffsetScale->RangeVisibilityOff ( );
    this->OffsetScale->SetEntryWidth(8);
    this->OffsetScale->SetLabelPositionToLeft();
            
    this->Script ( "grid %s -sticky ew -columnspan 2", this->ColorCodeFrame->GetWidgetName ( ) );
    this->Script("grid %s %s -sticky ew", 
                 this->OrientationMenu->GetWidgetName(), this->ForegroundSelector->GetWidgetName());
    this->Script("grid %s %s -sticky ew", 
            this->LabelSelector->GetWidgetName(), this->BackgroundSelector->GetWidgetName());
    this->Script ( "grid %s -sticky ew -columnspan 2", this->ScaleFrame->GetWidgetName ( ) );
    this->Script ("pack %s -side left -expand n -padx 1", this->VisibilityToggle->GetWidgetName ( ) );
    this->Script("pack %s -side left -fill x -expand y", this->OffsetScale->GetWidgetName());
    this->Script("grid columnconfigure %s 0 -weight 1", this->GetWidgetName());
    this->Script("grid columnconfigure %s 1 -weight 1", this->GetWidgetName());

    // put observers on widgets
    this->AddWidgetObservers();
}

//----------------------------------------------------------------------------
void vtkSlicerSliceControllerWidget::ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData ) 
{ 
   if (this->SliceNode != NULL && this->MRMLScene->GetNodeByID(this->SliceNode->GetID()) == NULL)
    {
    this->SetSliceNode(NULL);
    }

  if (this->SliceCompositeNode != NULL && this->MRMLScene->GetNodeByID(this->SliceCompositeNode->GetID()) == NULL)
    {
    this->SetSliceCompositeNode(NULL);
    }

// Update orientation if needed
  if ( vtkKWMenu::SafeDownCast(caller) == this->OrientationMenu->GetWidget()->GetWidget()->GetMenu() )
    {
    vtkKWMenuButton *mb = this->OrientationMenu->GetWidget()->GetWidget();
    if ( !strcmp (mb->GetValue(), "Axial") )   
      {
      this->SliceNode->SetOrientationToAxial();
      }
    if ( !strcmp (mb->GetValue(), "Sagittal") )   
      {
      this->SliceNode->SetOrientationToSagittal();
      }
    if ( !strcmp (mb->GetValue(), "Coronal") )   
      {
      this->SliceNode->SetOrientationToCoronal();
      }
    }


  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->ForegroundSelector )
    {
    if  (this->ForegroundSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetForegroundVolumeID( 
              this->ForegroundSelector->GetSelected()->GetID() );
      } 
    else if (this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetForegroundVolumeID( NULL );
      }
    }

  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->BackgroundSelector )
    {
    if  (this->BackgroundSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetBackgroundVolumeID( 
              this->BackgroundSelector->GetSelected()->GetID() );
      } 
    else if (this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetBackgroundVolumeID( NULL );
      }
    }

  if ( vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->LabelSelector )
    {
    if  (this->LabelSelector->GetSelected() != NULL && this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetLabelVolumeID( 
              this->LabelSelector->GetSelected()->GetID() );
      } 
    else if (this->SliceCompositeNode != NULL)
      {
      this->SliceCompositeNode->SetLabelVolumeID( NULL );
      }
    }

  if ( !this->SliceNode)
    {
    return;
    }
  
 
  vtkKWPushButton *toggle = vtkKWPushButton::SafeDownCast ( caller );
  // Toggle the SliceNode's visibility.
  if ( toggle == this->GetVisibilityToggle() &&
       event == vtkKWPushButton::InvokedEvent )
    {
    this->MRMLScene->SaveStateForUndo ( this->SliceNode );
    this->SliceNode->SetSliceVisible ( ! this->SliceNode->GetSliceVisible() ); 
    }

  if ( this->OffsetScale->GetWidget() == vtkKWScale::SafeDownCast( caller ) &&
          event == vtkKWScale::ScaleValueStartChangingEvent )
    {
    // set an undo state when the scale starts being dragged
    this->MRMLScene->SaveStateForUndo( this->SliceNode );
    }
  
  int modified = 0;

  //
  // Set the Offset from the Scale
  // - get the current translation in RAS space and convert it to Slice space
  //   by transforming it by the invers of the upper 3x3 of SliceToRAS
  // - replace the z value of the translation with the new value given by the slider
  // - this preserves whatever translation was already in place
  //

  vtkMatrix4x4 *sliceToRAS = vtkMatrix4x4::New();
  sliceToRAS->DeepCopy( this->SliceNode->GetSliceToRAS() );
  for (int i = 0; i < 3; i++)
    {
    sliceToRAS->SetElement( i, 3, 0.0 );  // Zero out the tranlation portion
    }
  sliceToRAS->Invert();
  double v1[4], v2[4];
  for (int i = 0; i < 4; i++)
    { // get the translation back as a vector
    v1[i] = this->SliceNode->GetSliceToRAS()->GetElement( i, 3 );
    }
  // bring the translation into slice space
  // and overwrite the z part
  sliceToRAS->MultiplyPoint(v1, v2);
  v2[2] = (double) this->OffsetScale->GetValue();
  // Now bring the new translation vector back into RAS space
  sliceToRAS->Invert();
  sliceToRAS->MultiplyPoint(v2, v1);
  for (int i = 0; i < 4; i++)
    {
    sliceToRAS->SetElement( i, 3, v1[i] );
    }
 
  // if the translation has changed, update the rest of the matrices
  if ( sliceToRAS->GetElement( 0, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 0, 3 ) ||
       sliceToRAS->GetElement( 1, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 1, 3 ) ||
       sliceToRAS->GetElement( 2, 3 ) != this->SliceNode->GetSliceToRAS()->GetElement( 2, 3 ) )
    {
    this->SliceNode->GetSliceToRAS()->DeepCopy( sliceToRAS );
    this->SliceNode->UpdateMatrices();
    modified = 1;
    }
  sliceToRAS->Delete();


  if ( modified )
    {
    this->Modified();
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
  // Set the scale value to match the offset
  //
  // Since translation is a scalar multiple of the Pz column of the
  // SliceToRAS upper 3x3, find a non-zero entry in that column
  // and calculate the scalar from that.
#if 0
  vtkMatrix4x4 *m = this->SliceNode->GetSliceToRAS();
  int i;
  double s;
  for (i = 0; i < 3; i++)
    {
    if ( m->GetElement( i, 2 ) != 0.0 )
      { 
      s = m->GetElement( i, 3 ) / m->GetElement( i, 2 ); 
      }
    }
#endif

  //
  // Set the scale value to match the offset
  //
  // - get the current translation in RAS space and convert it to Slice space
  //   by transforming it by the invers of the upper 3x3 of SliceToRAS
  //
  vtkMatrix4x4 *sliceToRAS = vtkMatrix4x4::New();
  sliceToRAS->DeepCopy( this->SliceNode->GetSliceToRAS() );
  for (int i = 0; i < 3; i++)
    {
    sliceToRAS->SetElement( i, 3, 0.0 );  // Zero out the tranlation portion
    }
  sliceToRAS->Invert();
  double v1[4], v2[4];
  for (int i = 0; i < 4; i++)
    { // get the translation back as a vector
    v1[i] = this->SliceNode->GetSliceToRAS()->GetElement( i, 3 );
    }
  // bring the translation into slice space
  // and overwrite the z part
  sliceToRAS->MultiplyPoint(v1, v2);

  if ( v2[2] != (double) this->OffsetScale->GetValue() )
    {
    this->OffsetScale->SetValue( v2[2] );
    modified = 1;
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

    node = this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetBackgroundVolumeID() );
    if ( node )
      {
      this->BackgroundSelector->SetSelected(node);
      }

    node = this->MRMLScene->GetNodeByID( this->SliceCompositeNode->GetLabelVolumeID() );
    if ( node )
      {
      this->LabelSelector->SetSelected(node);
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
void vtkSlicerSliceControllerWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

