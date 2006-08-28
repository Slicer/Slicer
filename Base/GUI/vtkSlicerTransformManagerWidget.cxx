#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformManagerWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLTransformNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTransformManagerWidget );
vtkCxxRevisionMacro ( vtkSlicerTransformManagerWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTransformManagerWidget::vtkSlicerTransformManagerWidget ( )
{

    this->NodeSelectorWidget = NULL;
    this->TransformSelectorWidget = NULL;
    this->AddTransformButton = NULL;
    this->RemoveTransformButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformManagerWidget::~vtkSlicerTransformManagerWidget ( )
{
  if (this->NodeSelectorWidget)
    {
    this->NodeSelectorWidget->SetParent(NULL);
    this->NodeSelectorWidget->Delete();
    this->NodeSelectorWidget = NULL;
    }
  if (this->TransformSelectorWidget)
    {
    this->TransformSelectorWidget->SetParent(NULL);
    this->TransformSelectorWidget->Delete();
    this->TransformSelectorWidget = NULL;
    }
  if (this->AddTransformButton)
    {
    this->AddTransformButton->SetParent(NULL);
    this->AddTransformButton->Delete();
    this->AddTransformButton = NULL;
    }  
  if (this->RemoveTransformButton)
    {
    this->RemoveTransformButton->SetParent(NULL);
    this->RemoveTransformButton->Delete();
    this->RemoveTransformButton = NULL;
    }  

  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerTransformManagerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerTransformManagerWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->NodeSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->NodeSelectorWidget->GetSelected());
    if (node != NULL)
      {
      if (node->GetTransformNodeID() != NULL)
        {
        this->AddTransformButton->EnabledOff();
        this->RemoveTransformButton->EnabledOn();
        this->TransformSelectorWidget->SetSelected(node->GetParentTransformNode());
        }
      else
        {
        this->RemoveTransformButton->EnabledOff();
        this->AddTransformButton->EnabledOn();
        }
      }
    else
      {
      this->AddTransformButton->EnabledOff();
      this->RemoveTransformButton->EnabledOff();
      }
      return;
    }    
  if (this->AddTransformButton == vtkKWPushButton::SafeDownCast(caller) &&
      event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->NodeSelectorWidget->GetSelected());
    vtkMRMLTransformableNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->TransformSelectorWidget->GetSelected());
    if (node != NULL && tnode != NULL)
      {
      node->SetAndObserveTransformNodeID(tnode->GetID());
      this->AddTransformButton->EnabledOff();
      this->RemoveTransformButton->EnabledOn();
      }
    return;
    }
  if (this->RemoveTransformButton == vtkKWPushButton::SafeDownCast(caller) &&
      event ==  vtkKWPushButton::InvokedEvent)
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->NodeSelectorWidget->GetSelected());
    if (node != NULL)
      {
      node->SetAndObserveTransformNodeID(NULL);
      this->AddTransformButton->EnabledOn();
      this->RemoveTransformButton->EnabledOff();
      }
    return;
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerTransformManagerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
}

//---------------------------------------------------------------------------
void vtkSlicerTransformManagerWidget::RemoveWidgetObservers ( ) {
  this->NodeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->TransformSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddTransformButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->RemoveTransformButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
}


//---------------------------------------------------------------------------
void vtkSlicerTransformManagerWidget::CreateWidget ( )
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
    // Widget FRAME            
    vtkKWFrameWithLabel *transformFrame = vtkKWFrameWithLabel::New ( );
    transformFrame->SetParent ( this->GetParent() );
    transformFrame->Create ( );
    transformFrame->SetLabelText ("Transform Manager");
    //transformFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   transformFrame->GetWidgetName() );
    
    this->NodeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->NodeSelectorWidget->SetParent ( transformFrame->GetFrame() );
    this->NodeSelectorWidget->Create ( );
    this->NodeSelectorWidget->SetNodeClass("vtkMRMLTransformableNode",NULL, NULL, NULL);
    this->NodeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->NodeSelectorWidget->SetNewNodeEnabled(0);
    this->NodeSelectorWidget->SetBorderWidth(2);
    // this->NodeSelectorWidget->SetReliefToGroove();
    this->NodeSelectorWidget->SetPadX(2);
    this->NodeSelectorWidget->SetPadY(2);
    this->NodeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->NodeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->NodeSelectorWidget->SetLabelText( "Node to Transform: ");
    this->NodeSelectorWidget->SetBalloonHelpString("select a node from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->NodeSelectorWidget->GetWidgetName());

    this->TransformSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->TransformSelectorWidget->SetParent ( transformFrame->GetFrame() );
    this->TransformSelectorWidget->Create ( );
    this->TransformSelectorWidget->AddNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
    this->TransformSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->TransformSelectorWidget->SetNewNodeEnabled(0);
    this->TransformSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->TransformSelectorWidget->SetBorderWidth(2);
    // this->TransformSelectorWidget->SetReliefToGroove();
    this->TransformSelectorWidget->SetPadX(2);
    this->TransformSelectorWidget->SetPadY(2);
    this->TransformSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TransformSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TransformSelectorWidget->SetLabelText( "Transform Node: ");
    this->TransformSelectorWidget->SetBalloonHelpString("select a node from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->TransformSelectorWidget->GetWidgetName());

    this->AddTransformButton = vtkKWPushButton::New();
    this->AddTransformButton->SetParent( transformFrame->GetFrame() );
    this->AddTransformButton->Create();
    this->AddTransformButton->SetText("Add Transform");
    //this->AddTransformButton->SetWidth ( 8 );
    this->Script("pack %s -side left -anchor e -padx 2 -pady 2", 
                this->AddTransformButton->GetWidgetName());


    this->RemoveTransformButton = vtkKWPushButton::New();
    this->RemoveTransformButton->SetParent( transformFrame->GetFrame() );
    this->RemoveTransformButton->Create();
    this->RemoveTransformButton->SetText("Remove Transform");
    //this->RemoveTransformButton->SetWidth ( 8 );
    this->Script("pack %s -side right -anchor e -padx 20 -pady 2", 
                this->RemoveTransformButton->GetWidgetName());

    // add observers
    this->NodeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                           (vtkCommand *)this->GUICallbackCommand );  
    this->TransformSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                               (vtkCommand *)this->GUICallbackCommand );
    this->AddTransformButton->AddObserver(vtkKWPushButton::InvokedEvent, 
                                               (vtkCommand *)this->GUICallbackCommand );
    this->RemoveTransformButton->AddObserver(vtkKWPushButton::InvokedEvent, 
                                               (vtkCommand *)this->GUICallbackCommand );
    transformFrame->Delete();
    
}





