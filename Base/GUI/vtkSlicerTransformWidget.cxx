#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMatrix4x4.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLFiducialNode.h"
#include "vtkMRMLTransformNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTransformWidget );
vtkCxxRevisionMacro ( vtkSlicerTransformWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTransformWidget::vtkSlicerTransformWidget ( )
{

    this->NodeSelectorWidget = NULL;
    this->TransformSelectorWidget = NULL;
    this->TransformEditSelectorWidget = NULL;
    this->AddTransformButton = NULL;
    this->RemoveTransformButton = NULL;
    this->MatrixWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformWidget::~vtkSlicerTransformWidget ( )
{
  if (this->NodeSelectorWidget)
    {
    this->NodeSelectorWidget->Delete();
    this->NodeSelectorWidget = NULL;
    }
  if (this->TransformSelectorWidget)
    {
    this->TransformSelectorWidget->Delete();
    this->TransformSelectorWidget = NULL;
    }
  if (this->TransformEditSelectorWidget)
    {
    this->TransformEditSelectorWidget->Delete();
    this->TransformEditSelectorWidget = NULL;
    }  
  if (this->AddTransformButton)
    {
    this->AddTransformButton->Delete();
    this->AddTransformButton = NULL;
    }  
  if (this->RemoveTransformButton)
    {
    this->RemoveTransformButton->Delete();
    this->RemoveTransformButton = NULL;
    }  

  if (this->MatrixWidget)
    {
    this->MatrixWidget->Delete();
    this->MatrixWidget = NULL;
    }  

  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerTransformWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerTransformWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  vtkSlicerNodeSelectorWidget *volSelector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (volSelector == this->NodeSelectorWidget && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    }       
} 



//---------------------------------------------------------------------------
void vtkSlicerTransformWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
}

//---------------------------------------------------------------------------
void vtkSlicerTransformWidget::RemoveWidgetObservers ( ) {
    this->NodeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->TransformSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->TransformEditSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->AddTransformButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->RemoveTransformButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerTransformWidget::CreateWidget ( )
{
    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *transformFrame = vtkKWFrameWithLabel::New ( );
    transformFrame->SetParent ( this->GetParent() );
    transformFrame->Create ( );
    transformFrame->SetLabelText ("Transform Editor");
    transformFrame->SetDefaultLabelFontWeightToNormal( );
    transformFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   transformFrame->GetWidgetName() );
    
    this->NodeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->NodeSelectorWidget->SetParent ( transformFrame->GetFrame() );
    this->NodeSelectorWidget->Create ( );
    this->NodeSelectorWidget->AddNodeClass("vtkMRMLVolumeNode");
    this->NodeSelectorWidget->AddNodeClass("vtkMRMLModelNode");
    this->NodeSelectorWidget->AddNodeClass("vtkMRMLFiducialNode");
    this->NodeSelectorWidget->AddNodeClass("vtkMRMLTransformNode");
    this->NodeSelectorWidget->SetNewNodeEnabled(0);
    this->NodeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
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
    this->TransformSelectorWidget->AddNodeClass("vtkMRMLLinearTransformNode");
    this->TransformSelectorWidget->SetNewNodeEnabled(0);
    this->TransformSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->TransformSelectorWidget->SetBorderWidth(2);
    // this->TransformSelectorWidget->SetReliefToGroove();
    this->TransformSelectorWidget->SetPadX(2);
    this->TransformSelectorWidget->SetPadY(2);
    this->TransformSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TransformSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TransformSelectorWidget->SetLabelText( "Node to Transform: ");
    this->TransformSelectorWidget->SetBalloonHelpString("select a node from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->TransformSelectorWidget->GetWidgetName());

    this->AddTransformButton = vtkKWPushButton::New();
    this->AddTransformButton->SetParent( transformFrame->GetFrame() );
    this->AddTransformButton->Create();
    this->AddTransformButton->SetText("Add Transform To Parent");
    this->AddTransformButton->SetWidth ( 8 );
    this->Script("pack %s -side top -anchor e -padx 20 -pady 2", 
                this->AddTransformButton->GetWidgetName());


    this->RemoveTransformButton = vtkKWPushButton::New();
    this->RemoveTransformButton->SetParent( transformFrame->GetFrame() );
    this->RemoveTransformButton->Create();
    this->RemoveTransformButton->SetText("Remove Transform To Parent");
    this->RemoveTransformButton->SetWidth ( 8 );
    this->Script("pack %s -side top -anchor e -padx 20 -pady 2", 
                this->RemoveTransformButton->GetWidgetName());


    this->TransformEditSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->TransformEditSelectorWidget->SetParent ( transformFrame->GetFrame() );
    this->TransformEditSelectorWidget->Create ( );
    this->TransformEditSelectorWidget->AddNodeClass("vtkMRMLLinearTransformNode");
    this->TransformEditSelectorWidget->SetNewNodeEnabled(1);
    this->TransformEditSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->TransformEditSelectorWidget->SetBorderWidth(2);
    // this->TransformEditSelectorWidget->SetReliefToGroove();
    this->TransformEditSelectorWidget->SetPadX(2);
    this->TransformEditSelectorWidget->SetPadY(2);
    this->TransformEditSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TransformEditSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TransformEditSelectorWidget->SetLabelText( "Node to Transform: ");
    this->TransformEditSelectorWidget->SetBalloonHelpString("select a node from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->TransformEditSelectorWidget->GetWidgetName());


    this->MatrixWidget = vtkKWMatrix4x4::New();
    this->MatrixWidget->SetParent( transformFrame->GetFrame() );
    this->MatrixWidget->Create();
    this->Script("pack %s -side top -anchor e -padx 20 -pady 2", 
                this->MatrixWidget->GetWidgetName());


    this->NodeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                           (vtkCommand *)this->GUICallbackCommand );  
    this->TransformSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                               (vtkCommand *)this->GUICallbackCommand );

    transformFrame->Delete();
    
}





