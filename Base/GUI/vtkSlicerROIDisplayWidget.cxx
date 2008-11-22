#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkSlicerROIDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"

#include "vtkMRMLROINode.h"



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerROIDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerROIDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerROIDisplayWidget::vtkSlicerROIDisplayWidget ( )
{

    this->ROINode = NULL;

    this->XRange = NULL;
    this->YRange = NULL;
    this->ZRange = NULL;
    
    this->XLabel = NULL;
    this->YLabel = NULL;
    this->ZLabel = NULL;
    
    
    this->VisibilityToggle = NULL;
    this->VisibilityIcons = NULL;

    this->InteractiveButton = vtkKWCheckButtonWithLabel::New();
    
    this->BoxWidget = vtkSlicerBoxWidget2::New();
    this->BoxWidgetRepresentation = vtkSlicerBoxRepresentation::New();
    this->BoxWidget->SetRepresentation(this->BoxWidgetRepresentation);
    this->BoxWidget->SetPriority(1);
    this->BoxWidgetRepresentation->SetPlaceFactor( 1.25 );

    this->ProcessingMRMLEvent = 0;
    this->ProcessingWidgetEvent = 0;

}


//---------------------------------------------------------------------------
vtkSlicerROIDisplayWidget::~vtkSlicerROIDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if ( this->VisibilityToggle )
    {
    this->VisibilityToggle->SetParent ( NULL );
    this->VisibilityToggle->Delete();
    this->VisibilityToggle = NULL;
    }
  if ( this->VisibilityIcons ) {
    this->VisibilityIcons->Delete  ( );
    this->VisibilityIcons = NULL;
    }
  if ( this->InteractiveButton ) {
    this->InteractiveButton->SetParent ( NULL );
    this->InteractiveButton->Delete  ( );
    this->InteractiveButton = NULL;
    }
   
  if (this->XRange) {
    this->XRange->SetParent(NULL);
    this->XRange->Delete();
    this->XRange = NULL;
    }
  if (this->XLabel) {
    this->XLabel->SetParent(NULL);
    this->XLabel->Delete();
    this->XLabel = NULL;
    }
  if (this->YRange) {
    this->YRange->SetParent(NULL);
    this->YRange->Delete();
    this->YRange = NULL;
    }
  if (this->YLabel) {
    this->YLabel->SetParent(NULL);
    this->YLabel->Delete();
    this->YLabel = NULL;
    }
  if (this->ZRange) {
    this->ZRange->SetParent(NULL);
    this->ZRange->Delete();
    this->ZRange = NULL;
    }
  if (this->ZLabel) {
    this->ZLabel->SetParent(NULL);
    this->ZLabel->Delete();
    this->ZLabel = NULL;
    }
  
  this->BoxWidget->SetRepresentation(NULL);
  this->BoxWidgetRepresentation->Delete();
  this->BoxWidget->Delete();

  vtkSetAndObserveMRMLNodeMacro(this->ROINode, NULL);
  this->SetMRMLScene ( NULL );
  
}

//---------------------------------------------------------------------------
void 
vtkSlicerROIDisplayWidget::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  this->BoxWidget->SetInteractor(interactor);
}


//---------------------------------------------------------------------------
void 
vtkSlicerROIDisplayWidget::SetInteractiveMode(int val)
{
  this->InteractiveButton->GetWidget()->SetSelectedState(val);
}

//---------------------------------------------------------------------------
int 
vtkSlicerROIDisplayWidget::GetInteractiveMode()
{
  return this->InteractiveButton->GetWidget()->GetSelectedState();
}

//---------------------------------------------------------------------------
void 
vtkSlicerROIDisplayWidget::SetBounds(double *bounds)
{
  this->BoxWidgetRepresentation->PlaceWidget(bounds);
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerROIDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetROINode ( vtkMRMLROINode *node )
{ 
  // 
  // Set the member variables and do a first process
  //
  vtkSetAndObserveMRMLNodeMacro(this->ROINode, node);

  if ( node )
    {
    this->UpdateWidget();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->ProcessingMRMLEvent != 0 || this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing " << this->ProcessingMRMLEvent);
    return;
    }

  if (!this->GetInteractiveMode() && (event == vtkKWScale::ScaleValueChangingEvent || 
                                      event == vtkKWScale::ScaleValueStartChangingEvent) )
    {
    return;
    }
  if (!this->GetInteractiveMode() && event == vtkCommand::InteractionEvent)
    {
    return;
    }
  
  this->ProcessingWidgetEvent = event;
  
  
  if (this->ROINode != NULL && 
      event != vtkKWScale::ScaleValueChangingEvent && 
      event != vtkKWScale::ScaleValueChangedEvent &&
      event != vtkCommand::InteractionEvent)
    {
    if (this->MRMLScene->GetNodeByID(this->ROINode->GetID()))
      {
      this->MRMLScene->SaveStateForUndo(this->ROINode);
      }
    }
  
  if (this->ROINode != NULL) 
    {
    
    if (event == vtkKWPushButton::InvokedEvent && this->VisibilityToggle == vtkKWPushButton::SafeDownCast(caller))
      {
      int vis = this->VisibilityToggle->GetState();
      this->ROINode->SetVisibility(vis);
      if (vis)
        {
        this->BoxWidget->On();
        }
      else
        {
        this->BoxWidget->Off();
        }
      }
    else if (event == vtkCommand::InteractionEvent || event == vtkCommand::EndInteractionEvent)
      {
      vtkSlicerBoxWidget2 *boxWidget = reinterpret_cast<vtkSlicerBoxWidget2*>(caller);
      vtkSlicerBoxRepresentation *boxRep = 
        reinterpret_cast<vtkSlicerBoxRepresentation*>(boxWidget->GetRepresentation());
      //boxRep->GetTransform(this->Transform);
      double* bounds = boxRep->GetBounds();
      XRange->SetRange(bounds[0],bounds[3]);
      YRange->SetRange(bounds[1],bounds[4]);
      ZRange->SetRange(bounds[2],bounds[5]);
      
      this->ROINode->DisableModifiedEventOn();
      this->ROINode->SetXYZ(0.5*(bounds[3]+bounds[0]),0.5*(bounds[4]+bounds[1]),0.5*(bounds[5]+bounds[2]));
      this->ROINode->SetRadiusXYZ(0.5*(bounds[3]-bounds[0]),0.5*(bounds[4]-bounds[1]),0.5*(bounds[5]-bounds[2]));
      this->ROINode->DisableModifiedEventOff();
      this->ROINode->InvokePendingModifiedEvent();
      }
    else if (event == vtkKWScale::ScaleValueChangingEvent || 
             event == vtkKWScale::ScaleValueChangedEvent)
      {
      double bounds[6];
      XRange->GetRange(bounds[0],bounds[3]);
      YRange->GetRange(bounds[1],bounds[4]);
      ZRange->GetRange(bounds[2],bounds[5]);
      
      this->ROINode->DisableModifiedEventOn();
      this->ROINode->SetXYZ(0.5*(bounds[3]+bounds[0]),0.5*(bounds[4]+bounds[1]),0.5*(bounds[5]+bounds[2]));
      this->ROINode->SetRadiusXYZ(0.5*(bounds[3]-bounds[0]),0.5*(bounds[4]-bounds[1]),0.5*(bounds[5]-bounds[2]));
      this->ROINode->DisableModifiedEventOff();
      this->ROINode->InvokePendingModifiedEvent();
      
      this->SetBounds(bounds);
      }
    }
  
  
  this->ProcessingWidgetEvent = 0;

}


//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if (this->ProcessingMRMLEvent != 0 || this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing " << this->ProcessingMRMLEvent);
    return;
    }
  if ( !this->ROINode )
    {
    return;
    }
  
  this->ProcessingMRMLEvent = event;
  

  if (this->ROINode == vtkMRMLROINode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidget();
    }

  this->ProcessingMRMLEvent = 0;
}

void vtkSlicerROIDisplayWidget::UpdateWidget()
{
  float *xyz = this->ROINode->GetXYZ();
  float *rxyz = this->ROINode->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  
  XRange->SetRange(bounds[0],bounds[3]);
  YRange->SetRange(bounds[1],bounds[4]);
  ZRange->SetRange(bounds[2],bounds[5]);
  this->SetBounds(bounds);
  
  int vis = this->ROINode->GetVisibility();
  this->VisibilityToggle->SetState(vis);
  if (vis)
    {
    this->BoxWidget->On();
    }
  else
    {
    this->BoxWidget->Off();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::RemoveMRMLObservers ( )
{
  if (this->ROINode)
    {
    this->ROINode->RemoveObservers(vtkCommand::ModifiedEvent,
                                     (vtkCommand *)this->MRMLCallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  // ---
  // DISPLAY FRAME
  vtkKWFrame *displayFrame = vtkKWFrame::New ( );
  displayFrame->SetParent ( this );
  displayFrame->Create ( );
  //displayFrame->SetLabelText ("ROI Display");
  //displayFrame->ExpandFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                 displayFrame->GetWidgetName());

  // XPosition frame
  vtkKWFrame *XPositionFrame = vtkKWFrame::New();
  XPositionFrame->SetParent ( displayFrame );
  XPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
    XPositionFrame->GetWidgetName());

  // x position label
  this->XLabel = vtkKWLabel::New();
  this->XLabel->SetParent ( XPositionFrame );
  this->XLabel->Create();
  this->XLabel->SetWidth(8);
  this->XLabel->SetText("L-R Range:");

  // x position scale
  this->XRange = vtkKWRange::New();
  this->XRange->SetParent( XPositionFrame );
  this->XRange->Create();
  this->XRange->SymmetricalInteractionOff();
  this->XRange->SetBalloonHelpString ( "Set Left-Right postion of the ROI BOX in RAS coordinates");
  this->XRange->SetRange(-256.0, 256.0);
  this->XRange->SetOrientationToHorizontal ();
  this->XRange->SetResolution(1);
  this->XRange->SetEntriesWidth(4);

  this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2 -in %s", 
    this->XLabel->GetWidgetName(), this->XRange->GetWidgetName() );

  // YPosition frame
  vtkKWFrame *YPositionFrame = vtkKWFrame::New();
  YPositionFrame->SetParent ( displayFrame );
  YPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
    YPositionFrame->GetWidgetName(),
    displayFrame->GetWidgetName());

  // Y position label
  this->YLabel = vtkKWLabel::New();
  this->YLabel->SetParent ( YPositionFrame );
  this->YLabel->Create();
  this->YLabel->SetWidth(8);
  this->YLabel->SetText("P-A Range:");

  // Y position scale
  this->YRange = vtkKWRange::New();
  this->YRange->SetParent( YPositionFrame );
  this->YRange->Create();
  this->YRange->SymmetricalInteractionOff();
  this->YRange->SetBalloonHelpString ( "Set Posterior-Anterior postion of the ROI BOX in RAS coordinates");
  this->YRange->SetRange(-256.0, 256.0);
  this->YRange->SetOrientationToHorizontal ();
  this->YRange->SetResolution(1);
  this->YRange->SetEntriesWidth(4);

  this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2 -in %s", 
    this->YLabel->GetWidgetName(), this->YRange->GetWidgetName() );

  // ZPosition frame
  vtkKWFrame *ZPositionFrame = vtkKWFrame::New();
  ZPositionFrame->SetParent ( displayFrame );
  ZPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               ZPositionFrame->GetWidgetName());
  // Z position label
  this->ZLabel = vtkKWLabel::New();
  this->ZLabel->SetParent ( ZPositionFrame );
  this->ZLabel->Create();
  this->ZLabel->SetWidth(8);
  this->ZLabel->SetText("I-S Range:");

  // Z position scale
  this->ZRange = vtkKWRange::New();
  this->ZRange->SetParent( ZPositionFrame );
  this->ZRange->Create();
  this->ZRange->SymmetricalInteractionOff();
  this->ZRange->SetBalloonHelpString ( "Set Interior-Superior postion of the ROI BOX in RAS coordinates");
  this->ZRange->SetRange(-256.0, 256.0);
  this->ZRange->SetOrientationToHorizontal ();
  this->ZRange->SetResolution(1);
  this->ZRange->SetEntriesWidth(4);

  this->Script("pack %s %s -side left -anchor w -padx 2 -pady 2 -in %s", 
    this->ZLabel->GetWidgetName(), this->ZRange->GetWidgetName() );

  
  // scale frame
  vtkKWFrame *scaleFrame = vtkKWFrame::New();
  scaleFrame->SetParent ( displayFrame );
  scaleFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
                scaleFrame->GetWidgetName(),
                displayFrame->GetWidgetName());

  // visibility
  this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );
  this->VisibilityToggle = vtkKWPushButton::New();
  this->VisibilityToggle->SetParent ( scaleFrame );
  this->VisibilityToggle->Create ( );
  this->VisibilityToggle->SetReliefToFlat ( );
  this->VisibilityToggle->SetOverReliefToNone ( );
  this->VisibilityToggle->SetBorderWidth ( 0 );
  this->VisibilityToggle->SetImageToIcon ( this->VisibilityIcons->GetInvisibleIcon ( ) );        
  this->VisibilityToggle->SetBalloonHelpString ( "Toggles ROI list visibility in the MainViewer." );
  this->VisibilityToggle->SetText ("Visibility");

  this->InteractiveButton->SetParent ( scaleFrame );
  this->InteractiveButton->Create ( );
  this->InteractiveButton->SetLabelText("Interactive Mode");
  this->InteractiveButton->SetBalloonHelpString("Enable interactive updates.");

  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2 -in %s", 
               this->VisibilityToggle->GetWidgetName(), 
               this->InteractiveButton->GetWidgetName(),
               this->ROITextScale->GetWidgetName(),scaleFrame->GetWidgetName() );

  displayFrame->Delete ();
  scaleFrame->Delete ();
  XPositionFrame->Delete ();
  YPositionFrame->Delete ();
  ZPositionFrame->Delete ();

  this->AddWidgetObservers();
  
  return;
  
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::AddWidgetObservers ( )
{
  this->BoxWidget->AddObserver(vtkCommand::InteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  this->BoxWidget->AddObserver(vtkCommand::EndInteractionEvent, (vtkCommand *)this->GUICallbackCommand);

  this->XRange->AddObserver(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->XRange->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->XRange->AddObserver(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->YRange->AddObserver(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->YRange->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->YRange->AddObserver(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->ZRange->AddObserver(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ZRange->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ZRange->AddObserver(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->VisibilityToggle->AddObserver (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::RemoveWidgetObservers ( )
{

  this->BoxWidget->RemoveObservers(vtkCommand::InteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  this->BoxWidget->RemoveObservers(vtkCommand::EndInteractionEvent, (vtkCommand *)this->GUICallbackCommand);
  
  this->XRange->RemoveObservers(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->XRange->RemoveObservers(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->XRange->RemoveObservers(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->YRange->RemoveObservers(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->YRange->RemoveObservers(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->YRange->RemoveObservers(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->ZRange->RemoveObservers(vtkKWRange::RangeValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ZRange->RemoveObservers(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ZRange->RemoveObservers(vtkKWRange::RangeValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->VisibilityToggle->RemoveObservers (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

}
