#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerModelDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWScale.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

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
    this->OpacityScale = NULL;
    this->SurfaceMaterialPropertyWidget = NULL;
    
}


//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::~vtkSlicerModelDisplayWidget ( )
{
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
  
  if (modelNode == this->MRMLScene->GetNodeByID(this->ModelNodeID) && 
      modelNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLModelDisplayNode *displayNode = modelNode->GetDisplayNode();
    

    if (displayNode != NULL && this->ModelDisplayNodeID != NULL)
      {
      this->RemoveMRMLObservers();
      this->SetModelDisplayNodeID(displayNode->GetID());
      this->AddMRMLObservers();
      }
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
  
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->RemoveObservers(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );
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
  vtkKWFrameWithLabel *modelDisplayFrame = vtkKWFrameWithLabel::New ( );
  modelDisplayFrame->SetParent ( this->GetParent() );
  modelDisplayFrame->Create ( );
  modelDisplayFrame->SetLabelText ("Display");
  modelDisplayFrame->CollapseFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 modelDisplayFrame->GetWidgetName() );

  this->ModelSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ModelSelectorWidget->SetParent ( modelDisplayFrame->GetFrame() );
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
  this->VisibilityButton->SetParent ( modelDisplayFrame->GetFrame() );
  this->VisibilityButton->Create ( );
  this->VisibilityButton->SetLabelText("Visibility");
  this->VisibilityButton->SetBalloonHelpString("set model visibility.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->VisibilityButton->GetWidgetName() );
  
  this->OpacityScale = vtkKWScaleWithLabel::New();
  this->OpacityScale->SetParent ( modelDisplayFrame->GetFrame() );
  this->OpacityScale->Create ( );
  this->OpacityScale->SetLabelText("Opacity");
  this->OpacityScale->GetWidget()->SetRange(0,1);
  this->OpacityScale->GetWidget()->SetResolution(0.1);
  this->OpacityScale->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->OpacityScale->GetWidgetName() );

  this->ChangeColorButton = vtkKWChangeColorButton::New();
  this->ChangeColorButton->SetParent ( modelDisplayFrame->GetFrame() );
  this->ChangeColorButton->Create ( );
  this->ChangeColorButton->SetColor(0.0, 1.0, 0.0);
  this->ChangeColorButton->LabelOutsideButtonOn();
  this->ChangeColorButton->SetLabelPositionToRight();
  this->ChangeColorButton->SetBalloonHelpString("set model opacity value.");
  this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                 this->ChangeColorButton->GetWidgetName() );

  this->SurfaceMaterialPropertyWidget = vtkKWSurfaceMaterialPropertyWidget::New();
  this->SurfaceMaterialPropertyWidget->SetParent ( modelDisplayFrame->GetFrame() );
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
  
  this->ChangeColorButton->AddObserver(vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangedEvent(), (vtkCommand *)this->GUICallbackCommand );
  this->SurfaceMaterialPropertyWidget->AddObserver(this->SurfaceMaterialPropertyWidget->GetPropertyChangingEvent(), (vtkCommand *)this->GUICallbackCommand );

  modelDisplayFrame->Delete();
    
}
