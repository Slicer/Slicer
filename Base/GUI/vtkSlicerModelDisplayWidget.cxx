#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerModelDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"

//#include "vtkMRMLColorProceduralFreeSurferNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerModelDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::vtkSlicerModelDisplayWidget ( )
{

    this->ModelNodeID = NULL;
    this->ModelDisplayNodeID = NULL;

    this->ModelSelectorWidget = NULL;
    this->VisibilityButton = NULL;
    this->ScalarVisibilityButton = NULL;
    this->ColorSelectorWidget = NULL;
    this->ClippingButton = NULL;
    this->OpacityScale = NULL;
    this->SurfaceMaterialPropertyWidget = NULL;
    
}


//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::~vtkSlicerModelDisplayWidget ( )
{
  this->RemoveMRMLObservers();
  this->RemoveWidgetObservers();

  if (this->ModelSelectorWidget)
    {
    this->ModelSelectorWidget->SetParent(NULL);
    this->ModelSelectorWidget->Delete();
    this->ModelSelectorWidget = NULL;
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
  this->SetModelNodeID (NULL);
  this->SetModelDisplayNodeID (NULL);
  
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerModelDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "ModelNode ID: " << this->GetModelNodeID() << "\n";
    os << indent << "ModelDisplayNode ID: " << this->GetModelDisplayNodeID() << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::SetModelNode ( vtkMRMLModelNode *modelNode )
{ 
  // Select this model node
  this->ModelSelectorWidget->SetSelected(modelNode); 


  // 
  // Set the member variables and do a first process
  //
  this->RemoveMRMLObservers();

  this->SetModelNodeID( modelNode->GetID() );
  this->SetModelDisplayNodeID( modelNode->GetDisplayNodeID() );

  this->AddMRMLObservers();

  if ( modelNode )
    {
    this->ProcessMRMLEvents(modelNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  //
  // process model selector events
  //
  vtkSlicerNodeSelectorWidget *modelSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (modelSelector == this->ModelSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLModelNode *model = 
        vtkMRMLModelNode::SafeDownCast(this->ModelSelectorWidget->GetSelected());

    if (model != NULL)
      {
      this->SetModelNode(model);
      }

    return;
    }
  
  if (this->ModelDisplayNodeID != NULL && 
    !(vtkKWSurfaceMaterialPropertyWidget::SafeDownCast(caller) == this->SurfaceMaterialPropertyWidget && event == this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent()) &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangingEvent) &&
    !(vtkKWScale::SafeDownCast(caller) == this->OpacityScale->GetWidget() && event == vtkKWScale::ScaleValueChangedEvent))
    {
    this->MRMLScene->SaveStateForUndo(this->MRMLScene->GetNodeByID(this->ModelDisplayNodeID));
    }
  
  this->UpdateMRML();

  if ((event == this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent() ||
       event == this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent() ||
       event == vtkKWChangeColorButton::ColorChangedEvent) &&
      this->ModelDisplayNodeID != NULL)
    {
      vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->ModelDisplayNodeID);
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
      this->ModelDisplayNodeID != NULL) 
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      // get the model display node
      vtkMRMLModelDisplayNode *displayNode = NULL;
      if (this->ModelDisplayNodeID != NULL)
        {
        displayNode = 
          vtkMRMLModelDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->ModelDisplayNodeID) );
        if (displayNode != NULL)
          {
          if (displayNode->GetColorNodeID() == NULL)
            {
            vtkWarningMacro("Model display node doesn't have a color node, setting a default.\n");
            //displayNode->SetDefaultColorMap();
            vtkSlicerColorLogic *colorLogic = vtkSlicerColorGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Color"))->GetLogic();
            if (colorLogic)
              {
              displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultModelColorNodeID());
              }
            }
          // set and observe it's colour node id
          if (displayNode->GetColorNodeID() == NULL ||
              strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
            {
            // there's a change, set it
            displayNode->SetAndObserveColorNodeID(color->GetID());
            }
          else
            {
            //std::cout << "Display node's color node is not null and it's the same, so not setting it\n";
            }      
          }        
        }
      else
        {
        //std::cout << "Display node is null, can't set it's color id\n";
        }  
      }
    else
      {
      //std::cout << "Color node from the widget is null, can't set the display node's color id\n";
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if ( !this->ModelNodeID )
    {
    return;
    }

  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(caller);

  if (modelNode != NULL &&
      modelNode == this->MRMLScene->GetNodeByID(this->ModelNodeID) && 
      event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLModelDisplayNode *displayNode = modelNode->GetDisplayNode();
    

    if (displayNode != NULL && this->ModelDisplayNodeID != NULL)
      {
      this->RemoveMRMLObservers();
      this->SetModelDisplayNodeID(displayNode->GetID());
      // set the color node selector to reflect the volume's color node
      if (displayNode->GetColorNode() != NULL)
        {
        this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
        }
      else
        {
        vtkWarningMacro("Slicer Model Display Widget cannot set the color selector widget, as the model's display node has no color node set");
        }
      this->AddMRMLObservers();
      }
    }

  vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(caller);
  // did the display node change for the model currently in the widget?
  if (modelDisplayNode != NULL &&
      this->MRMLScene->GetNodeByID(this->ModelNodeID) != NULL &&
      vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ModelNodeID))->GetDisplayNode() == modelDisplayNode &&
      event == vtkCommand::ModifiedEvent)
    {
    // update the color node selector
    this->ColorSelectorWidget->SetSelected(modelDisplayNode->GetColorNode());
    }
  this->UpdateWidget();
  
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::AddMRMLObservers ( )
{
  if ( !this->ModelNodeID )
    {
    return;
    }

  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ModelNodeID));
  
  if (modelNode != NULL)
    {
    vtkMRMLModelDisplayNode *displayNode = modelNode->GetDisplayNode();
    
    if (displayNode != NULL)
      {
        displayNode->AddObserver(vtkCommand::ModifiedEvent,
                                 (vtkCommand *)this->MRMLCallbackCommand );      
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::RemoveMRMLObservers ( )
{
  if ( !this->ModelNodeID )
    {
    return;
    }

  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ModelNodeID));
  
  if (modelNode != NULL)
    {
    vtkMRMLModelDisplayNode *displayNode = modelNode->GetDisplayNode();
    
    if (displayNode != NULL)
      {
        displayNode->RemoveObservers(vtkCommand::ModifiedEvent,
                                        (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::UpdateWidget()
{
  
  if ( this->ModelDisplayNodeID )
    {
    vtkMRMLModelDisplayNode *displayNode = 
      vtkMRMLModelDisplayNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->ModelDisplayNodeID));
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
void vtkSlicerModelDisplayWidget::UpdateMRML()
{
  
  if ( this->ModelDisplayNodeID )
    {
    vtkMRMLModelDisplayNode *displayNode = 
      vtkMRMLModelDisplayNode::SafeDownCast(this->MRMLScene->GetNodeByID(
                                              this->ModelDisplayNodeID));
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
void vtkSlicerModelDisplayWidget::RemoveWidgetObservers ( ) {
  this->ModelSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
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
void vtkSlicerModelDisplayWidget::CreateWidget ( )
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
  vtkKWFrame *modelDisplayFrame = vtkKWFrame::New ( );
  modelDisplayFrame->SetParent ( this->GetParent() );
  modelDisplayFrame->Create ( );
/*
  modelDisplayFrame->SetLabelText ("Display");
  modelDisplayFrame->CollapseFrame ( );
*/
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 modelDisplayFrame->GetWidgetName() );

  this->ModelSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ModelSelectorWidget->SetParent ( modelDisplayFrame );
  this->ModelSelectorWidget->Create ( );
  this->ModelSelectorWidget->SetNodeClass("vtkMRMLModelNode", NULL, NULL, NULL);
  this->ModelSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ModelSelectorWidget->SetBorderWidth(2);
  // this->ModelSelectorWidget->SetReliefToGroove();
  this->ModelSelectorWidget->SetPadX(2);
  this->ModelSelectorWidget->SetPadY(2);
  this->ModelSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ModelSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->ModelSelectorWidget->SetLabelText( "Model Select: ");
  this->ModelSelectorWidget->SetBalloonHelpString("select a model from the current mrml scene.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->ModelSelectorWidget->GetWidgetName());

  this->VisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->VisibilityButton->SetParent ( modelDisplayFrame );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Visibility");
  this->VisibilityButton->SetBalloonHelpString("set model visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );

  this->ScalarVisibilityButton = vtkKWCheckButtonWithLabel::New();
  this->ScalarVisibilityButton->SetParent ( modelDisplayFrame );
  this->ScalarVisibilityButton->Create ( );
  this->ScalarVisibilityButton->SetLabelText("Scalar Visibility");
  this->ScalarVisibilityButton->SetBalloonHelpString("set model scalar visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ScalarVisibilityButton->GetWidgetName() );

  // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( modelDisplayFrame );
  this->ColorSelectorWidget->Create ( );
  this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorProceduralFreeSurferNode", NULL, NULL, NULL);
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
  // disable this until FreeSurfer nodes are supported
  this->ColorSelectorWidget->EnabledOff();
  
  this->ClippingButton = vtkKWCheckButtonWithLabel::New();
  this->ClippingButton->SetParent ( modelDisplayFrame );
  this->ClippingButton->Create ( );
  this->ClippingButton->SetLabelText("Clipping");
  this->ClippingButton->SetBalloonHelpString("set model clipping with RGB slice planes.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ClippingButton->GetWidgetName() );
  
  this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent ( modelDisplayFrame );
  this->OpacityScale->Create ( );
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->GetWidget()->SetRange(0,1);
  this->OpacityScale->GetWidget()->SetResolution(0.1);
  this->OpacityScale->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->OpacityScale->GetWidgetName() );

  this->ChangeColorButton = vtkKWChangeColorButton::New();
  this->ChangeColorButton->SetParent ( modelDisplayFrame );
  this->ChangeColorButton->Create ( );
  this->ChangeColorButton->SetColor(0.0, 1.0, 0.0);
  this->ChangeColorButton->LabelOutsideButtonOn();
  this->ChangeColorButton->SetLabelPositionToRight();
  this->ChangeColorButton->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ChangeColorButton->GetWidgetName() );

  this->SurfaceMaterialPropertyWidget = vtkKWSurfaceMaterialPropertyWidget::New();
  this->SurfaceMaterialPropertyWidget->SetParent ( modelDisplayFrame );
  this->SurfaceMaterialPropertyWidget->Create ( );
  this->SurfaceMaterialPropertyWidget->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->SurfaceMaterialPropertyWidget->GetWidgetName() );

  // add observers
  this->ModelSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
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
   
  modelDisplayFrame->Delete();
    
}
