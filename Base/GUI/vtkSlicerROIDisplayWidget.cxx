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
vtkCxxRevisionMacro ( vtkSlicerROIDisplayWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerROIDisplayWidget::vtkSlicerROIDisplayWidget ( )
{
    //Set Standard Clipping Colors
    ColorsClippingHandles[0][0]=1;
    ColorsClippingHandles[0][1]=0;
    ColorsClippingHandles[0][2]=1;

    ColorsClippingHandles[1][0]=1;
    ColorsClippingHandles[1][1]=0;
    ColorsClippingHandles[1][2]=0;

    ColorsClippingHandles[2][0]=1;
    ColorsClippingHandles[2][1]=1;
    ColorsClippingHandles[2][2]=1;

    ColorsClippingHandles[3][0]=.89;
    ColorsClippingHandles[3][1]=.6;
    ColorsClippingHandles[3][2]=.07;

    ColorsClippingHandles[4][0]=0;
    ColorsClippingHandles[4][1]=0;
    ColorsClippingHandles[4][2]=1;

    ColorsClippingHandles[5][0]=0;
    ColorsClippingHandles[5][1]=0;
    ColorsClippingHandles[5][2]=0;

    this->ROINode = NULL;

    this->XRange = NULL;
    this->YRange = NULL;
    this->ZRange = NULL;
    
    this->XLabel = NULL;
    this->YLabel = NULL;
    this->ZLabel = NULL;
    
    
    this->VisibilityToggle = NULL;
    this->VisibilityIcons = NULL;
    this->VisibilityLabel = NULL;

    this->InteractiveButton = vtkKWCheckButtonWithLabel::New();
    
    this->ProcessingMRMLEvent = 0;
    this->ProcessingWidgetEvent = 0;

    this->XRangeExtent[0] = -100;
    this->XRangeExtent[1] = 100;
    this->YRangeExtent[0] = -100;
    this->YRangeExtent[1] = 100;
    this->ZRangeExtent[0] = -100;
    this->ZRangeExtent[1] = 100;

    this->XResolution = 1;
    this->YResolution = 1;
    this->ZResolution = 1;

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
  if ( this->VisibilityLabel ) {
    this->VisibilityLabel->SetParent(NULL);
    this->VisibilityLabel->Delete();
    this->VisibilityLabel = NULL;
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
  
  vtkSetAndObserveMRMLNodeMacro(this->ROINode, NULL);
  this->SetMRMLScene ( NULL );
  
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
void vtkSlicerROIDisplayWidget::ProcessWidgetEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData))
{
  if (this->ProcessingMRMLEvent != 0 || this->ProcessingWidgetEvent != 0)
    {
    vtkDebugMacro("ProcessMRMLEvents already processing " << this->ProcessingMRMLEvent);
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
    
    if (event == vtkKWPushButton::InvokedEvent && 
        this->VisibilityToggle == vtkKWPushButton::SafeDownCast(caller))
      {
      int vis = ROINode->GetVisibility();
      if (vis) 
        {
        vis = 0;
        }
      else
        {
        vis = 1;
        }
      if (vis)
        {
        this->GetVisibilityToggle()->SetImageToIcon(
              this->GetVisibilityIcons()->GetVisibleIcon());
        this->ROINode->SetVisibility(1);
        }
      else
        {
        this->GetVisibilityToggle()->SetImageToIcon(
          this->GetVisibilityIcons()->GetInvisibleIcon());
        this->ROINode->SetVisibility(0);
        }
      }
    else if (event == vtkKWCheckButton::SelectedStateChangedEvent && 
            this->InteractiveButton->GetWidget() == vtkKWCheckButton::SafeDownCast(caller) )
      {
      this->ROINode->SetInteractiveMode(this->InteractiveButton->GetWidget()->GetSelectedState());
      }
    else if (event == vtkKWScale::ScaleValueChangingEvent || 
             event == vtkKWScale::ScaleValueChangedEvent)
      {
      double bounds[6];
      XRange->GetRange(bounds[0],bounds[3]);
      YRange->GetRange(bounds[1],bounds[4]);
      ZRange->GetRange(bounds[2],bounds[5]);

      if (this->GetInteractiveMode() || 
          event == vtkKWScale::ScaleValueChangedEvent )
        {
        this->ROINode->DisableModifiedEventOn();
        this->ROINode->SetXYZ(0.5*(bounds[3]+bounds[0]),0.5*(bounds[4]+bounds[1]),0.5*(bounds[5]+bounds[2]));
        this->ROINode->SetRadiusXYZ(0.5*(bounds[3]-bounds[0]),0.5*(bounds[4]-bounds[1]),0.5*(bounds[5]-bounds[2]));
        this->ROINode->DisableModifiedEventOff();
        this->ROINode->InvokePendingModifiedEvent();
        }
      }

    }// if (this->ROINode != NULL) 
  
  
  this->ProcessingWidgetEvent = 0;

}


//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::ProcessMRMLEvents(vtkObject *caller,
                                                  unsigned long event,
                                                  void *vtkNotUsed(callData))
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
  double *xyz = this->ROINode->GetXYZ();
  double *rxyz = this->ROINode->GetRadiusXYZ();
  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[  i] = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  
  XRange->SetRange(bounds[0],bounds[3]);
  YRange->SetRange(bounds[1],bounds[4]);
  ZRange->SetRange(bounds[2],bounds[5]);
  
  this->XRange->SetResolution(this->XResolution);
  this->YRange->SetResolution(this->YResolution);
  this->ZRange->SetResolution(this->ZResolution);
      
  int vis = this->ROINode->GetVisibility();

  if (vis > 0)
    {
    this->GetVisibilityToggle()->SetImageToIcon(
      this->GetVisibilityIcons()->GetVisibleIcon());
    }
  else
    {
    this->GetVisibilityToggle()->SetImageToIcon(
      this->GetVisibilityIcons()->GetInvisibleIcon());
    }
  this->InteractiveButton->GetWidget()->SetSelectedState(this->ROINode->GetInteractiveMode());
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
void vtkSlicerROIDisplayWidget::SetXRangeExtent(double r0, double r1)
{
  XRangeExtent[0] = r0;
  XRangeExtent[1] = r1;
  this->XRange->SetWholeRange(XRangeExtent[0],XRangeExtent[1]);
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetYRangeExtent(double r0, double r1)
{
  YRangeExtent[0] = r0;
  YRangeExtent[1] = r1;
  this->YRange->SetWholeRange(YRangeExtent[0],YRangeExtent[1]);
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetZRangeExtent(double r0, double r1)
{
  ZRangeExtent[0] = r0;
  ZRangeExtent[1] = r1;
  this->ZRange->SetWholeRange(ZRangeExtent[0],ZRangeExtent[1]);

}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetXResolution(double r)
{
  this->XResolution = r;
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetYResolution(double r)
{
  this->YResolution = r;
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::SetZResolution(double r)
{
  this->ZResolution = r;
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
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 displayFrame->GetWidgetName());

  // XPosition frame
  vtkKWFrame *XPositionFrame = vtkKWFrame::New();
  XPositionFrame->SetParent ( displayFrame );
  XPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -expand y -fill x -pady 0 -in %s",
                XPositionFrame->GetWidgetName(),
                displayFrame->GetWidgetName());

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
  this->XRange->SetWholeRange(XRangeExtent[0],XRangeExtent[1]);
  this->XRange->SetRange(XRangeExtent[0]+(XRangeExtent[1]-XRangeExtent[0])*.4,
                         XRangeExtent[0]+(XRangeExtent[1]-XRangeExtent[0])*.6);
  this->XRange->SetOrientationToHorizontal ();
  this->XRange->SetResolution(this->XResolution);
  this->XRange->SetEntriesWidth(4);
  this->XRange->SetWidth(120);
  this->XRange->SetSlider1Color(this->ColorsClippingHandles[0]);
  this->XRange->SetSlider2Color(this->ColorsClippingHandles[1]);

  this->Script("pack %s %s -side left -anchor w -expand y -fill x -padx 2 -pady 2 -in %s", 
    this->XLabel->GetWidgetName(),
    this->XRange->GetWidgetName(),
    XPositionFrame->GetWidgetName());

  // YPosition frame
  vtkKWFrame *YPositionFrame = vtkKWFrame::New();
  YPositionFrame->SetParent ( displayFrame );
  YPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -expand y -pady 0 -in %s",
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
  this->YRange->SetWholeRange(YRangeExtent[0],YRangeExtent[1]);
  this->YRange->SetRange(YRangeExtent[0]+(YRangeExtent[1]-YRangeExtent[0])*.4,
                         YRangeExtent[0]+(YRangeExtent[1]-YRangeExtent[0])*.6);
  this->YRange->SetOrientationToHorizontal ();
  this->YRange->SetResolution(this->YResolution);
  this->YRange->SetEntriesWidth(4);
  this->YRange->SetWidth(120);
  this->YRange->SetSlider1Color(this->ColorsClippingHandles[2]);
  this->YRange->SetSlider2Color(this->ColorsClippingHandles[3]);

  this->Script("pack %s %s -side left -anchor w -expand y -fill x -padx 2 -pady 2 -in %s", 
    this->YLabel->GetWidgetName(),
    this->YRange->GetWidgetName(),
    YPositionFrame->GetWidgetName());

  // ZPosition frame
  vtkKWFrame *ZPositionFrame = vtkKWFrame::New();
  ZPositionFrame->SetParent ( displayFrame );
  ZPositionFrame->Create ( );
  this->Script ("pack %s -side top -anchor nw -fill x -expand y -pady 0 -in %s",
                ZPositionFrame->GetWidgetName(),
                displayFrame->GetWidgetName());
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
  this->ZRange->SetWholeRange(ZRangeExtent[0],ZRangeExtent[1]);
  this->ZRange->SetRange(ZRangeExtent[0]+(ZRangeExtent[1]-ZRangeExtent[0])*.4,
                         ZRangeExtent[0]+(ZRangeExtent[1]-ZRangeExtent[0])*.6);
  this->ZRange->SetOrientationToHorizontal ();
  this->ZRange->SetResolution(this->ZResolution);
  this->ZRange->SetEntriesWidth(4);
  this->ZRange->SetWidth(120);
  this->ZRange->SetSlider1Color(this->ColorsClippingHandles[4]);
  this->ZRange->SetSlider2Color(this->ColorsClippingHandles[5]);

  this->Script("pack %s %s -side left -anchor w -expand y -fill x -padx 2 -pady 2 -in %s", 
    this->ZLabel->GetWidgetName(),
    this->ZRange->GetWidgetName(),
    ZPositionFrame->GetWidgetName());

  
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

  this->VisibilityLabel = vtkKWLabel::New();
  this->VisibilityLabel->SetParent (scaleFrame);
  this->VisibilityLabel->Create();
  this->VisibilityLabel->SetText("Display clipping box");

  this->InteractiveButton->SetParent ( scaleFrame );
  this->InteractiveButton->Create ( );
  this->InteractiveButton->SetLabelText("Interactive Mode");
  this->InteractiveButton->SetBalloonHelpString("Enable interactive updates.");

  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2 -in %s", 
               this->VisibilityLabel->GetWidgetName(),
               this->VisibilityToggle->GetWidgetName(), 
               this->InteractiveButton->GetWidgetName(),
               scaleFrame->GetWidgetName() );

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
  this->InteractiveButton->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerROIDisplayWidget::RemoveWidgetObservers ( )
{
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
  this->InteractiveButton->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}
