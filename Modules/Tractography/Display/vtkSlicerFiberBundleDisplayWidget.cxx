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

    this->FiberBundleNode = NULL;
    this->FiberBundleLineDisplayNode = NULL;
    this->FiberBundleTubeDisplayNode = NULL;
    this->FiberBundleGlyphDisplayNode = NULL;

    this->FiberBundleSelectorWidget = NULL;
    this->ColorSelectorWidget = NULL;
    this->ClippingButton = NULL;
    this->OpacityScale = NULL;

    this->LineVisibilityButton = NULL;
    this->TubeVisibilityButton = NULL;
    this->GlyphVisibilityButton = NULL;

    this->GlyphDisplayWidget = NULL;

    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;
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
  if (this->ChangeColorButton)
    {
    this->ChangeColorButton->SetParent(NULL);
    this->ChangeColorButton->Delete();
    this->ChangeColorButton= NULL;
    }
  


  if (this->LineVisibilityButton)
    {
    this->LineVisibilityButton->SetParent(NULL);
    this->LineVisibilityButton->Delete();
    this->LineVisibilityButton = NULL;
    }
  if (this->TubeVisibilityButton)
    {
    this->TubeVisibilityButton->SetParent(NULL);
    this->TubeVisibilityButton->Delete();
    this->TubeVisibilityButton = NULL;
    }
  if (this->GlyphVisibilityButton)
    {
    this->GlyphVisibilityButton->SetParent(NULL);
    this->GlyphVisibilityButton->Delete();
    this->GlyphVisibilityButton = NULL;
    }

  if (this->GlyphDisplayWidget)
    {
    this->GlyphDisplayWidget->SetParent(NULL);
    this->GlyphDisplayWidget->Delete();
    this->GlyphDisplayWidget = NULL;
    }

  vtkSetAndObserveMRMLNodeMacro(this->FiberBundleNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->FiberBundleLineDisplayNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->FiberBundleTubeDisplayNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->FiberBundleGlyphDisplayNode, NULL);
  this->SetMRMLScene ( NULL );
  
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerFiberBundleDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::SetFiberBundleNode ( vtkMRMLFiberBundleNode *fiberBundleNode )
{ 
  if (fiberBundleNode == this->FiberBundleNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->FiberBundleNode, fiberBundleNode);

  if (this->FiberBundleNode )
    {
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleLineDisplayNode, this->FiberBundleNode->AddLineDisplayNode());
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleTubeDisplayNode, this->FiberBundleNode->AddTubeDisplayNode());
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleGlyphDisplayNode, this->FiberBundleNode->AddGlyphDisplayNode());
    this->UpdateWidget();
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
  vtkSlicerNodeSelectorWidget *fiberBundleSelector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

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
    
  if (!this->SyncSceneNodes())
    {
    return;
    }
  
  if (this->FiberBundleNode != NULL  &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangingEvent) &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangedEvent))
    {
      if (this->FiberBundleLineDisplayNode)
      {
      this->MRMLScene->SaveStateForUndo(this->FiberBundleLineDisplayNode);
      }
      if (this->FiberBundleTubeDisplayNode)
      {
      this->MRMLScene->SaveStateForUndo(this->FiberBundleTubeDisplayNode);
      }
      if (this->FiberBundleGlyphDisplayNode)
      {
      this->MRMLScene->SaveStateForUndo(this->FiberBundleGlyphDisplayNode);
      }
    }
  this->UpdateMRML();

  //
  // process color selector events
  //
  vtkSlicerNodeSelectorWidget *colSelector = 
    vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (colSelector == this->ColorSelectorWidget && 
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) 
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      // get the fiberBundle display node
      // TODO add separate colors for tube and glyph
      if (this->FiberBundleLineDisplayNode != NULL)
        {
        // set and observe it's colour node id
        if (this->FiberBundleLineDisplayNode->GetColorNodeID() == NULL ||
            strcmp(this->FiberBundleLineDisplayNode->GetColorNodeID(), color->GetID()) != 0)
          {
          this->FiberBundleLineDisplayNode->SetAndObserveColorNodeID(color->GetID());
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

  if ( !this->FiberBundleNode )
    {
    return;
    }

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(caller);
  
  // if this event comes from our fiberBundleNode, it is not null, and has been modified
  if (fiberBundleNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    this->SetFiberBundleNode(fiberBundleNode);
    }
  
  this->UpdateWidget();
  
}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::AddMRMLObservers ( )
{
  if (this->FiberBundleLineDisplayNode != NULL)
    {
    this->FiberBundleLineDisplayNode->AddObserver(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }
  if (this->FiberBundleTubeDisplayNode != NULL)
    {
    this->FiberBundleTubeDisplayNode->AddObserver(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }
  if (this->FiberBundleGlyphDisplayNode != NULL)
    {
    this->FiberBundleGlyphDisplayNode->AddObserver(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }

}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::RemoveMRMLObservers ( )
{
  if (this->FiberBundleLineDisplayNode != NULL)
    {
    this->FiberBundleLineDisplayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }
  if (this->FiberBundleTubeDisplayNode != NULL)
    {
    this->FiberBundleTubeDisplayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }
  if (this->FiberBundleGlyphDisplayNode != NULL)
    {
    this->FiberBundleGlyphDisplayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                            (vtkCommand *)this->MRMLCallbackCommand );      
    }

}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::UpdateWidget()
{
  if (!this->SyncSceneNodes())
    {
    return;
    }

  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }
  this->UpdatingWidget = 1;
  
  // Select this fiberBundle node
  this->FiberBundleSelectorWidget->SetSelected(this->FiberBundleNode); 

  if ( this->FiberBundleLineDisplayNode )
    {
    this->ClippingButton->GetWidget()->SetSelectedState(this->FiberBundleLineDisplayNode->GetClipping());
    this->OpacityScale->GetWidget()->SetValue(this->FiberBundleLineDisplayNode->GetOpacity());

    this->ChangeColorButton->SetColor(this->FiberBundleLineDisplayNode->GetColor());

    if (this->FiberBundleLineDisplayNode->GetColorNode() != NULL)
      {
      this->ColorSelectorWidget->SetSelected(this->FiberBundleLineDisplayNode->GetColorNode());
      }

    this->LineVisibilityButton->GetWidget()->SetSelectedState(this->FiberBundleLineDisplayNode->GetVisibility());
    } 
  if ( this->FiberBundleTubeDisplayNode )
    {
    this->TubeVisibilityButton->GetWidget()->SetSelectedState(this->FiberBundleTubeDisplayNode->GetVisibility());
    // TODO color node, opacity and color for tubes
    } 
  if ( this->FiberBundleGlyphDisplayNode )
    {
    this->GlyphVisibilityButton->GetWidget()->SetSelectedState(this->FiberBundleGlyphDisplayNode->GetVisibility());
    // TODO color node, opacity and color for glyps
    vtkMRMLDiffusionTensorDisplayPropertiesNode *dpnode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast( this->FiberBundleGlyphDisplayNode->GetFiberGlyphDTDisplayPropertiesNode() );
    this->GlyphDisplayWidget->SetDiffusionTensorDisplayPropertiesNode(dpnode);

    // TODO glyph widget
    } 
    
   this->UpdatingWidget = 0;

}

//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::UpdateMRML()
{
  if (!this->SyncSceneNodes())
    {
    return;
    }

  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;


  if ( this->FiberBundleLineDisplayNode )
    {
    this->FiberBundleLineDisplayNode->SetClipping(this->ClippingButton->GetWidget()->GetSelectedState());
    this->FiberBundleLineDisplayNode->SetOpacity(this->OpacityScale->GetWidget()->GetValue());
    this->FiberBundleLineDisplayNode->SetColor(this->ChangeColorButton->GetColor());

    this->FiberBundleLineDisplayNode->SetVisibility(this->LineVisibilityButton->GetWidget()->GetSelectedState());
    }
  if ( this->FiberBundleTubeDisplayNode )
    {
     // TODO color node, opacity and color for tubes
    this->FiberBundleTubeDisplayNode->SetVisibility(this->TubeVisibilityButton->GetWidget()->GetSelectedState());
    }
  if ( this->FiberBundleGlyphDisplayNode )
    {
    // TODO color node, opacity and color for glyphs
    this->FiberBundleGlyphDisplayNode->SetVisibility(this->GlyphVisibilityButton->GetWidget()->GetSelectedState());
    }
      
    this->UpdatingMRML = 0;
    
}


//---------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayWidget::RemoveWidgetObservers ( ) {
  this->FiberBundleSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  this->ClippingButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ChangeColorButton->RemoveObservers(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->LineVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  // TO DO glyph widget
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

  this->LineVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->LineVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->LineVisibilityButton->Create ( );
  this->LineVisibilityButton->SetLabelText("Line Visibility");
  this->LineVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->LineVisibilityButton->GetWidgetName() );
  this->TubeVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->TubeVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->TubeVisibilityButton->Create ( );
  this->TubeVisibilityButton->SetLabelText("Tube Visibility");
  this->TubeVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->TubeVisibilityButton->GetWidgetName() );
  this->GlyphVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->GlyphVisibilityButton->SetParent ( fiberBundleDisplayFrame );
  this->GlyphVisibilityButton->Create ( );
  this->GlyphVisibilityButton->SetLabelText("Glyph Visibility");
  this->GlyphVisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->GlyphVisibilityButton->GetWidgetName() );

  this->GlyphDisplayWidget = vtkSlicerDiffusionTensorGlyphDisplayWidget::New();
  this->GlyphDisplayWidget->SetParent ( fiberBundleDisplayFrame );
  this->GlyphDisplayWidget->Create ( );
  this->GlyphDisplayWidget->SetBalloonHelpString("set glyph display parameters.");
  this->GlyphDisplayWidget->SetMRMLScene(this->GetMRMLScene());
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->GlyphDisplayWidget->GetWidgetName() );

  // add observers
  this->FiberBundleSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->ClippingButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
   

  this->LineVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TubeVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GlyphVisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );


  // TODO glyph widget
  // TODO color node, opacity and color for glyphs and tubes



  fiberBundleDisplayFrame->Delete();
    
}

//---------------------------------------------------------------------------
bool vtkSlicerFiberBundleDisplayWidget::SyncSceneNodes()
{
  bool inSync = true;
  if (this->FiberBundleNode && vtkMRMLFiberBundleNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->FiberBundleNode->GetID() ) ) == NULL )
    {
     vtkSetAndObserveMRMLNodeMacro(this->FiberBundleNode, NULL);
     inSync = false;
    }
  if (this->FiberBundleLineDisplayNode && vtkMRMLFiberBundleLineDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->FiberBundleLineDisplayNode->GetID() ) ) == NULL )
    {
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleLineDisplayNode, NULL);
    inSync = false;
    }
  if (this->FiberBundleTubeDisplayNode && vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->FiberBundleTubeDisplayNode->GetID() ) ) == NULL )
    {
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleTubeDisplayNode, NULL);
    inSync = false;
    }
  if (this->FiberBundleGlyphDisplayNode && vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->FiberBundleGlyphDisplayNode->GetID() ) ) == NULL )
    {
    vtkSetAndObserveMRMLNodeMacro(this->FiberBundleGlyphDisplayNode, NULL);
    inSync = false;
    }
  return inSync;
}
