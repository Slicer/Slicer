#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerFiberBundleDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerFiberBundleDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerFiberBundleDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayWidget::vtkSlicerFiberBundleDisplayWidget ( )
{

    this->FiberBundleNodeID = NULL;
    this->FiberBundleDisplayNodeID = NULL;

    this->FiberBundleSelectorWidget = NULL;
    this->VisibilityButton = NULL;
    this->ScalarVisibilityButton = NULL;
    this->ColorSelectorWidget = NULL;
    this->ClippingButton = NULL;
    this->OpacityScale = NULL;
    this->SurfaceMaterialPropertyWidget = NULL;
    
}


//---------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayWidget::~vtkSlicerFiberBundleDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if (this->FiberBundleSelectorWidget)
    {
    this->FiberBundleSelectorWidget->SetParent(NULL);
    this->FiberBundleSelectorWidget->Delete();
    this->FiberBundleSelectorWidget = NULL;
    }
  if (this->VisibilityButton)
    {
    this->VisibilityButton->SetParent(NULL);
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }
  if (this->ScalarVisibilityButton)
    {
    this->ScalarVisibilityButton->SetParent(NULL);
    this->ScalarVisibilityButton->Delete();
    this->ScalarVisibilityButton = NULL;
    }
   if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  if (this->ClippingButton)
    {
    this->ClippingButton->SetParent(NULL);
    this->ClippingButton->Delete();
    this->ClippingButton = NULL;
    }
  if (this->OpacityScale)
    {
    this->OpacityScale->SetParent(NULL);
    this->OpacityScale->Delete();
    this->OpacityScale = NULL;
    }
  if (this->SurfaceMaterialPropertyWidget)
    {
    this->SurfaceMaterialPropertyWidget->SetParent(NULL);
    this->SurfaceMaterialPropertyWidget->Delete();
    this->SurfaceMaterialPropertyWidget = NULL;
    }
  if (this->ChangeColorButton)
    {
    this->ChangeColorButton->SetParent(NULL);
    this->ChangeColorButton->Delete();
    this->ChangeColorButton= NULL;
    }
  
  this->SetMRMLScene ( NULL );
  this->SetFiberBundleNodeID (NULL);
  this->SetFiberBundleDisplayNodeID (NULL);
  
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerFiberBundleDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "FiberBundleNode ID: " << this->GetFiberBundleNodeID() << "\n";
    os << indent << "FiberBundleDisplayNode ID: " << this->GetFiberBundleDisplayNodeID() << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::SetFiberBundleNode ( vtkMRMLFiberBundleNode *fiberBundleNode )
{ 
  vtkDebugWithObjectMacro(this,"Setting fiber bundle node" <<  fiberBundleNode->GetID());


  // Select this fiberBundle node
  this->FiberBundleSelectorWidget->SetSelected(fiberBundleNode); 

  // 
  // Set the member variables and do a first process
  //
  this->RemoveMRMLObservers();

  this->SetFiberBundleNodeID( fiberBundleNode->GetID() );
  this->SetFiberBundleDisplayNodeID( fiberBundleNode->GetDisplayNodeID() );

  this->AddMRMLObservers();

  if ( fiberBundleNode )
    {
    this->ProcessMRMLEvents(fiberBundleNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkDebugWithObjectMacro(this,"Process Widget Events");

  //
  // process fiberBundle selector events
  //
  vtkSlicerNodeSelectorWidget *fiberBundleSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (fiberBundleSelector == this->FiberBundleSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLFiberBundleNode *fiberBundle = 
        vtkMRMLFiberBundleNode::SafeDownCast(this->FiberBundleSelectorWidget->GetSelected());

    if (fiberBundle != NULL)
      {
      this->SetFiberBundleNode(fiberBundle);
      }

    return;
    }
  
  if (this->FiberBundleDisplayNodeID != NULL && 
    !(vtkKWSurfaceMaterialPropertyWidget::SafeDownCast(caller) == this->SurfaceMaterialPropertyWidget && event == this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent()) &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangingEvent) &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangedEvent))
    {
    this->MRMLScene->SaveStateForUndo(this->MRMLScene->GetNodeByID(this->FiberBundleDisplayNodeID));
    }
  
  this->UpdateMRML();

  if ((event == this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent() ||
       event == this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent() ||
       event == vtkKWChangeColorButton::ColorChangedEvent) &&
      this->FiberBundleDisplayNodeID != NULL)
    {
      vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->FiberBundleDisplayNodeID);
      if (node != NULL)
        {
        node->Modified();
        }
    }
  //
  // process color selector events
  //
  vtkSlicerNodeSelectorWidget *colSelector = 
    vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (colSelector == this->ColorSelectorWidget && 
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->FiberBundleDisplayNodeID != NULL) 
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      // get the fiberBundle display node
      vtkMRMLFiberBundleDisplayNode *displayNode = NULL;
      if (this->FiberBundleDisplayNodeID != NULL)
        {
        displayNode = 
          vtkMRMLFiberBundleDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->FiberBundleDisplayNodeID) );
        if (displayNode != NULL)
          {
          // set and observe it's colour node id
          if (displayNode->GetColorNodeID() == NULL ||
              strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
            {
            // there's a change, set it
            displayNode->SetAndObserveColorNodeID(color->GetID());
            }
          }        
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  vtkDebugWithObjectMacro(this,"Process MRML Events");

  if ( !this->FiberBundleNodeID )
    {
    return;
    }

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(caller);
  
  // if this event comes from our fiberBundleNode, it is not null, and has been modified
  if (fiberBundleNode == this->MRMLScene->GetNodeByID(this->FiberBundleNodeID) && 
      fiberBundleNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLFiberBundleDisplayNode *displayNode = fiberBundleNode->GetDisplayNode();
    

    if (displayNode != NULL && this->FiberBundleDisplayNodeID != NULL)
      {
      // stop observing display node
      this->RemoveMRMLObservers();

      this->SetFiberBundleDisplayNodeID(displayNode->GetID());

      // set the color node selector to reflect the volume's color node
      if (displayNode->GetColorNode() != NULL)
        {
        this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
        }
      else
        {
        vtkWarningMacro("Slicer FiberBundle Display Widget cannot set the color selector widget, as the fiberBundle's display node has no color node set");
        }

      // start observing display node again
      this->AddMRMLObservers();

      }
    }
  
  this->UpdateWidget();
  
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::AddMRMLObservers ( )
{
  if ( !this->FiberBundleNodeID )
    {
    return;
    }

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->FiberBundleNodeID));
  
  if (fiberBundleNode != NULL)
    {
    vtkMRMLFiberBundleDisplayNode *displayNode = fiberBundleNode->GetDisplayNode();
    
    if (displayNode != NULL)
      {
        displayNode->AddObserver(vtkCommand::ModifiedEvent,
                                 (vtkCommand *)this->MRMLCallbackCommand );      
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::RemoveMRMLObservers ( )
{
  if ( !this->FiberBundleNodeID )
    {
    return;
    }

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->FiberBundleNodeID));
  
  if (fiberBundleNode != NULL)
    {
    vtkMRMLFiberBundleDisplayNode *displayNode = fiberBundleNode->GetDisplayNode();
    
    if (displayNode != NULL)
      {
        displayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                                        (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::UpdateWidget()
{
  
  if ( this->FiberBundleDisplayNodeID )
    {
    vtkMRMLFiberBundleDisplayNode *displayNode = 
      vtkMRMLFiberBundleDisplayNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->FiberBundleDisplayNodeID));
    if (displayNode != NULL) 
      {
      this->VisibilityButton->GetWidget()->SetSelectedState(displayNode->GetVisibility());
      this->ScalarVisibilityButton->GetWidget()->SetSelectedState(displayNode->GetScalarVisibility());
      this->ClippingButton->GetWidget()->SetSelectedState(displayNode->GetClipping());
      this->OpacityScale->GetWidget()->SetValue(displayNode->GetOpacity());
      if (this->SurfaceMaterialPropertyWidget->GetProperty() == NULL)
        {
        vtkProperty *prop = vtkProperty::New();
        this->SurfaceMaterialPropertyWidget->SetProperty(prop);
        prop->Delete();
        }
        
      this->SurfaceMaterialPropertyWidget->GetProperty()->SetAmbient(displayNode->GetAmbient());
      this->SurfaceMaterialPropertyWidget->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
      this->SurfaceMaterialPropertyWidget->GetProperty()->SetSpecular(displayNode->GetSpecular());
      this->SurfaceMaterialPropertyWidget->GetProperty()->SetSpecularPower(displayNode->GetPower());
      this->ChangeColorButton->SetColor(displayNode->GetColor());
      this->SurfaceMaterialPropertyWidget->Update();
      }
    
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::UpdateMRML()
{
  
  if ( this->FiberBundleDisplayNodeID )
    {
    vtkMRMLFiberBundleDisplayNode *displayNode = 
      vtkMRMLFiberBundleDisplayNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->FiberBundleDisplayNodeID));
    if (displayNode != NULL) 
      {
      displayNode->SetVisibility(this->VisibilityButton->GetWidget()->GetSelectedState());
      displayNode->SetScalarVisibility(this->ScalarVisibilityButton->GetWidget()->GetSelectedState());
      displayNode->SetClipping(this->ClippingButton->GetWidget()->GetSelectedState());
      displayNode->SetOpacity(this->OpacityScale->GetWidget()->GetValue());
      displayNode->SetAmbient(this->SurfaceMaterialPropertyWidget->GetProperty()->GetAmbient());
      displayNode->SetDiffuse(this->SurfaceMaterialPropertyWidget->GetProperty()->GetDiffuse());
      displayNode->SetSpecular(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecular());
      displayNode->SetPower(this->SurfaceMaterialPropertyWidget->GetProperty()->GetSpecularPower());
      displayNode->SetColor(this->ChangeColorButton->GetColor());

      }
    
    return;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::RemoveWidgetObservers ( ) {
  this->FiberBundleSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ScalarVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ClippingButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );
  
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::CreateWidget ( )
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
  vtkKWFrame *fiberBundleDisplayFrame = vtkKWFrame::New ( );
  fiberBundleDisplayFrame->SetParent ( this->GetParent() );
  fiberBundleDisplayFrame->Create ( );
/*
  fiberBundleDisplayFrame->SetLabelText ("Display");
  fiberBundleDisplayFrame->CollapseFrame ( );
*/
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 fiberBundleDisplayFrame->GetWidgetName() );

  this->FiberBundleSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->FiberBundleSelectorWidget->SetParent ( fiberBundleDisplayFrame );
  this->FiberBundleSelectorWidget->Create ( );
  this->FiberBundleSelectorWidget->SetNodeClass("vtkMRMLFiberBundleNode", NULL, NULL, NULL);
  this->FiberBundleSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->FiberBundleSelectorWidget->SetBorderWidth(2);
  // this->FiberBundleSelectorWidget->SetReliefToGroove();
  this->FiberBundleSelectorWidget->SetPadX(2);
  this->FiberBundleSelectorWidget->SetPadY(2);
  this->FiberBundleSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->FiberBundleSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->FiberBundleSelectorWidget->SetLabelText( "FiberBundle Select: ");
  this->FiberBundleSelectorWidget->SetBalloonHelpString("select a fiberBundle from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->FiberBundleSelectorWidget->GetWidgetName());

  this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->VisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Visibility");
  this->VisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );

  this->ScalarVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->ScalarVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->ScalarVisibilityButton->Create ( );
  this->ScalarVisibilityButton->SetLabelText("Scalar Visibility");
  this->ScalarVisibilityButton->SetBalloonHelpString("set fiberBundle scalar visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ScalarVisibilityButton->GetWidgetName() );

  // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( fiberBundleDisplayFrame );
  this->ColorSelectorWidget->Create ( );
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->ColorSelectorWidget->ShowHiddenOn();
  this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ColorSelectorWidget->SetBorderWidth(2);
  // this->ColorSelectorWidget->SetReliefToGroove();
  this->ColorSelectorWidget->SetPadX(2);
  this->ColorSelectorWidget->SetPadY(2);
  this->ColorSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ColorSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->ColorSelectorWidget->SetLabelText( "Scalar Color Map Select: ");
  this->ColorSelectorWidget->SetBalloonHelpString("select a color node from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ColorSelectorWidget->GetWidgetName());
  
  this->ClippingButton = vtkKWCheckButtonWithLabel::New();
  this->ClippingButton->SetParent ( fiberBundleDisplayFrame );
  this->ClippingButton->Create ( );
  this->ClippingButton->SetLabelText("Clipping");
  this->ClippingButton->SetBalloonHelpString("set fiberBundle clipping with RGB slice planes.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ClippingButton->GetWidgetName() );
  
  this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent ( fiberBundleDisplayFrame );
  this->OpacityScale->Create ( );
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->GetWidget()->SetRange(0,1);
  this->OpacityScale->GetWidget()->SetResolution(0.1);
  this->OpacityScale->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->OpacityScale->GetWidgetName() );

  this->ChangeColorButton = vtkKWChangeColorButton::New();
  this->ChangeColorButton->SetParent ( fiberBundleDisplayFrame );
  this->ChangeColorButton->Create ( );
  this->ChangeColorButton->SetColor(0.0, 1.0, 0.0);
  this->ChangeColorButton->LabelOutsideButtonOn();
  this->ChangeColorButton->SetLabelPositionToRight();
  this->ChangeColorButton->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ChangeColorButton->GetWidgetName() );

  this->SurfaceMaterialPropertyWidget = vtkKWSurfaceMaterialPropertyWidget::New();
  this->SurfaceMaterialPropertyWidget->SetParent ( fiberBundleDisplayFrame );
  this->SurfaceMaterialPropertyWidget->Create ( );
  this->SurfaceMaterialPropertyWidget->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SurfaceMaterialPropertyWidget->GetWidgetName() );

  // add observers
  this->FiberBundleSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ScalarVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ClippingButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );

  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
   
  fiberBundleDisplayFrame->Delete();
    
}
