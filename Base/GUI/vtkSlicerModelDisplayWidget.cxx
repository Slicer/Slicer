#include "vtkObject.h"
#include "vtkObjectFactory.h"

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
    
}


//---------------------------------------------------------------------------
vtkSlicerModelDisplayWidget::~vtkSlicerModelDisplayWidget ( )
{
  if (this->ModelSelectorWidget)
    {
    this->ModelSelectorWidget->Delete();
    this->ModelSelectorWidget = NULL;
    }
  if (this->VisibilityButton)
    {
    this->VisibilityButton->Delete();
    this->VisibilityButton = NULL;
    }
  if (this->OpacityScale)
    {
    this->OpacityScale->Delete();
    this->OpacityScale = NULL;
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
  
  if (this->ModelDisplayNodeID != NULL)
    {
    this->MRMLScene->SaveStateForUndo(this->MRMLScene->GetNodeByID(this->ModelDisplayNodeID));
    }
  
  this->UpdateMRML();
  
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
      }
    
    return;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelDisplayWidget::RemoveWidgetObservers ( ) {
  this->ModelSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  this->VisibilityButton->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
//this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  /*
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand );
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand );
  */

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

  // add observers
  this->ModelSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
  //this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OpacityScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->VisibilityButton->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  /*
  if (this->MRMLScene != NULL)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand );
    }
  */
  modelDisplayFrame->Delete();
    
}
