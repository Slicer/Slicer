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

    this->VisibilityButton = NULL;

    this->GlyphDisplayWidget = NULL;
    this->GeometryMenu = NULL;
    this->DisplayFrame = NULL;
    this->GeometryColorMenu = NULL;

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
  


  if (this->VisibilityButton)
    {
    this->VisibilityButton->SetParent(NULL);
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }

  if (this->GeometryMenu)
    {
    this->GeometryMenu->SetParent(NULL);
    this->GeometryMenu->Delete();
    this->GeometryMenu = NULL;
    }
    
  if (this->GlyphDisplayWidget)
    {
    this->GlyphDisplayWidget->SetParent(NULL);
    this->GlyphDisplayWidget->Delete();
    this->GlyphDisplayWidget = NULL;
    }
    
  if (this->DisplayFrame)
    {
    this->DisplayFrame->SetParent(NULL);
    this->DisplayFrame->Delete();
    this->DisplayFrame = NULL;
    }
  if (this->GeometryColorMenu)
    {
    this->GeometryColorMenu->SetParent(NULL);
    this->GeometryColorMenu->Delete();
    this->GeometryColorMenu = NULL;
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
  
  if (vtkKWMenu::SafeDownCast(caller) == this->GeometryMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    this->CurrentGeometry = this->GeometryMenu->GetWidget()->GetValue();
    std::string label = this->CurrentGeometry + std::string(" Display");
    this->DisplayFrame->SetLabelText (label.c_str());
    this->UpdateWidget();
    return;
    }

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
    this->UpdateWidget();
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
      vtkMRMLFiberBundleDisplayNode* dnode = this->GetCurrentDisplayNode();

      if (dnode)
        {
        this->MRMLScene->SaveStateForUndo(dnode);
        }
    }


  // process  color menu events
  if (vtkKWMenu::SafeDownCast(caller) == this->GeometryColorMenu->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode* propNode = this->GetCurrentDTDisplayPropertyNode();
    if (propNode)
      {
      propNode->SetColorGlyphBy(this->GeometryColorMap[std::string(this->GeometryColorMenu->GetWidget()->GetValue())]);
      return;
      }
    }
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
      vtkMRMLFiberBundleDisplayNode* dnode = this->GetCurrentDisplayNode();

      if (dnode != NULL)
        {
        // set and observe it's colour node id
        if (dnode->GetColorNodeID() == NULL ||
            strcmp(dnode->GetColorNodeID(), color->GetID()) != 0)
          {
          dnode->SetAndObserveColorNodeID(color->GetID());
          }
        }           
      }
    return;
    }
  this->UpdateMRML();

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

  vtkMRMLFiberBundleDisplayNode* dnode = this->GetCurrentDisplayNode();

  vtkMRMLDiffusionTensorDisplayPropertiesNode* propNode = this->GetCurrentDTDisplayPropertyNode();

  if (propNode)
    {
    // Set color by value
    this->GeometryColorMenu->GetWidget()->SetValue(propNode->GetColorGlyphByAsString());
    }

  // common props
  if ( dnode )
    {
    this->VisibilityButton->GetWidget()->SetSelectedState(dnode->GetVisibility());
    this->ClippingButton->GetWidget()->SetSelectedState(dnode->GetClipping());
    this->OpacityScale->GetWidget()->SetValue(dnode->GetOpacity());
    this->ChangeColorButton->SetColor(dnode->GetColor());
    if (dnode->GetColorNode() != NULL)
      {
      vtkMRMLColorNode *color =
        vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
      if (color == NULL ||
          strcmp(dnode->GetColorNodeID(), color->GetID()) != 0)
        {
        this->ColorSelectorWidget->SetSelected(dnode->GetColorNode());
        }
      }
    else
      {
      // clear the selection
      this->ColorSelectorWidget->SetSelected(NULL);
      }
    }

  if ( this->FiberBundleTubeDisplayNode )
    {
    // TODO props for lines
    } 
  if ( this->FiberBundleTubeDisplayNode )
    {
    // TODO props for tubes
    } 
  if ( this->FiberBundleGlyphDisplayNode )
    {
    // TODO props for glyps
    vtkMRMLDiffusionTensorDisplayPropertiesNode *dpnode = 
      vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast( this->FiberBundleGlyphDisplayNode->GetDTDisplayPropertiesNode() );
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

  vtkMRMLFiberBundleDisplayNode* dnode = this->GetCurrentDisplayNode();

  vtkMRMLDiffusionTensorDisplayPropertiesNode* propNode = this->GetCurrentDTDisplayPropertyNode();

  if (propNode)
    {
    propNode->SetColorGlyphBy(this->GeometryColorMap[std::string(this->GeometryColorMenu->GetWidget()->GetValue())]);
    }

  if (dnode )
    {
    dnode->SetClipping(this->ClippingButton->GetWidget()->GetSelectedState());
    dnode->SetOpacity(this->OpacityScale->GetWidget()->GetValue());
    dnode->SetColor(this->ChangeColorButton->GetColor());
    dnode->SetVisibility(this->VisibilityButton->GetWidget()->GetSelectedState());
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

  this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeometryMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeometryColorMenu->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
  this->FiberBundleSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->FiberBundleSelectorWidget->SetLabelText( "FiberBundle Select: ");
  this->FiberBundleSelectorWidget->SetBalloonHelpString("select a fiberBundle from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->FiberBundleSelectorWidget->GetWidgetName());

  // geometry menu

  this->GeometryMenu = vtkKWMenuButtonWithLabel::New();
  this->GeometryMenu->SetParent( fiberBundleDisplayFrame );
  this->GeometryMenu->Create();
  this->GeometryMenu->GetWidget()->GetMenu()->AddRadioButton("Tube");
  this->GeometryMenu->GetWidget()->GetMenu()->AddRadioButton("Line");
  this->GeometryMenu->GetWidget()->GetMenu()->AddRadioButton("Glyph");
  this->GeometryMenu->GetWidget()->SetValue("Tube");
  this->CurrentGeometry = "Tube";
  
  // pack geom menu
  this->GeometryMenu->SetLabelText("Display For:");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->GeometryMenu->GetWidgetName());


  this->DisplayFrame = vtkKWFrameWithLabel::New ( );
  this->DisplayFrame->SetParent ( fiberBundleDisplayFrame );
  std::string label = this->CurrentGeometry + std::string("Display");
  this->DisplayFrame->SetLabelText (label.c_str());
  this->DisplayFrame->Create ( );
  
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->DisplayFrame->GetWidgetName() );

  this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->VisibilityButton->SetParent ( this->DisplayFrame->GetFrame());
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Visibility");
  this->VisibilityButton->SetBalloonHelpString("set fiberBundle visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );

  // color by menu

  vtkKWMenuButtonWithLabel *colorMenuButton = 
    vtkKWMenuButtonWithLabel::New();

  this->GeometryColorMenu = colorMenuButton;
  colorMenuButton->SetParent( this->DisplayFrame->GetFrame() );
  colorMenuButton->Create();
  // initialize color menu
  //Create dummy display properties node to init variables
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = 
    vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  int initIdx = propNode->GetFirstColorGlyphBy();
  int endIdx = propNode->GetLastColorGlyphBy();
  int currentVal = propNode->GetColorGlyphBy();
  this->GeometryColorMap.clear();
  for (int k=initIdx ; k<=endIdx ; k++)
    {
    propNode->SetColorGlyphBy(k);
    const char *tag = propNode->GetColorGlyphByAsString();
    this->GeometryColorMap[std::string(tag)]=k;
    colorMenuButton->GetWidget()->GetMenu()->AddRadioButton(tag);
    }
  // init to class default value
  propNode->SetColorGlyphBy(currentVal);
  colorMenuButton->GetWidget()->SetValue(propNode->GetColorGlyphByAsString());

  propNode->Delete();

  // pack color menu
  colorMenuButton->SetLabelText("Color By Scalar");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               colorMenuButton->GetWidgetName());

  // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( this->DisplayFrame->GetFrame() );
  this->ColorSelectorWidget->Create ( );
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
  this->ColorSelectorWidget->ShowHiddenOn();
  this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ColorSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ColorSelectorWidget->SetLabelText( "Scalar Color Map");
  this->ColorSelectorWidget->SetBalloonHelpString("select a color node from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ColorSelectorWidget->GetWidgetName());
  
  this->ClippingButton = vtkKWCheckButtonWithLabel::New();
  this->ClippingButton->SetParent ( this->DisplayFrame->GetFrame() );
  this->ClippingButton->Create ( );
  this->ClippingButton->SetLabelText("Clipping");
  this->ClippingButton->SetBalloonHelpString("set fiberBundle clipping with RGB slice planes.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ClippingButton->GetWidgetName() );
  
  this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent ( this->DisplayFrame->GetFrame() );
  this->OpacityScale->Create ( );
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->GetWidget()->SetRange(0,1);
  this->OpacityScale->GetWidget()->SetResolution(0.1);
  this->OpacityScale->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->OpacityScale->GetWidgetName() );

  this->ChangeColorButton = vtkKWChangeColorButton::New();
  this->ChangeColorButton->SetParent ( this->DisplayFrame->GetFrame() );
  this->ChangeColorButton->Create ( );
  this->ChangeColorButton->SetColor(0.0, 1.0, 0.0);
  this->ChangeColorButton->LabelOutsideButtonOn();
  this->ChangeColorButton->SetLabelPositionToRight();
  this->ChangeColorButton->SetBalloonHelpString("set fiberBundle opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ChangeColorButton->GetWidgetName() );

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
   

  this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeometryMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeometryColorMenu->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );


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

//---------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkSlicerFiberBundleDisplayWidget::GetCurrentDisplayNode()
{
  vtkMRMLFiberBundleDisplayNode *dnode = NULL;
  if (this->CurrentGeometry == "Line") 
    {
    dnode = this->FiberBundleLineDisplayNode;
    }
  else if (this->CurrentGeometry == "Tube") 
    {
    dnode = this->FiberBundleTubeDisplayNode;
    }
  else if (this->CurrentGeometry == "Glyph") 
    {
    dnode = this->FiberBundleGlyphDisplayNode;
    }
  
  return dnode;
}

//---------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkSlicerFiberBundleDisplayWidget::GetCurrentDTDisplayPropertyNode()
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode *dpnode = NULL;
  vtkMRMLFiberBundleDisplayNode *dnode = this->GetCurrentDisplayNode();
  if (dnode)
    {
    dpnode = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast( dnode->GetDTDisplayPropertiesNode() );
    }
  return dpnode;
}

void vtkSlicerFiberBundleDisplayWidget::SetTractVisibility(int visibility)
  {
  this->VisibilityButton->GetWidget()->SetSelectedState(visibility);
  }
