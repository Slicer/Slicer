#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMatrix4x4.h"

#include "vtkMRMLLinearTransformNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTransformEditorWidget );
vtkCxxRevisionMacro ( vtkSlicerTransformEditorWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTransformEditorWidget::vtkSlicerTransformEditorWidget ( )
{
    this->TransformEditSelectorWidget = NULL;
    this->MatrixWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformEditorWidget::~vtkSlicerTransformEditorWidget ( )
{
  if (this->TransformEditSelectorWidget)
    {
    this->TransformEditSelectorWidget->Delete();
    this->TransformEditSelectorWidget = NULL;
    }  
  if (this->MatrixWidget)
    {
    this->MatrixWidget->Delete();
    this->MatrixWidget = NULL;
    }  

  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  if (this->TransformEditSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());

    if (node != NULL)
      {
      this->MatrixWidget->EnabledOn();
      this->MatrixWidget->SetMatrix4x4(node->GetMatrixTransformToParent());
      this->MatrixWidget->UpdateWidget();
      }
    else
      {
      this->MatrixWidget->EnabledOff();
      }

    return;
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
}

//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::RemoveWidgetObservers ( ) {
  this->TransformEditSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  
}


//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::CreateWidget ( )
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
    // MODULE FRAME            
    vtkKWFrameWithLabel *transformFrame = vtkKWFrameWithLabel::New ( );
    transformFrame->SetParent ( this->GetParent() );
    transformFrame->Create ( );
    transformFrame->SetLabelText ("Transform Editor");
    //transformFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   transformFrame->GetWidgetName() );
    
    this->TransformEditSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->TransformEditSelectorWidget->SetParent ( transformFrame->GetFrame() );
    this->TransformEditSelectorWidget->Create ( );
    this->TransformEditSelectorWidget->AddNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, "LinearTransform");
    this->TransformEditSelectorWidget->SetMRMLScene(this->GetMRMLScene());    
    this->TransformEditSelectorWidget->SetNewNodeEnabled(1);
    this->TransformEditSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->TransformEditSelectorWidget->SetBorderWidth(2);
    // this->TransformEditSelectorWidget->SetReliefToGroove();
    this->TransformEditSelectorWidget->SetPadX(2);
    this->TransformEditSelectorWidget->SetPadY(2);
    this->TransformEditSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TransformEditSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TransformEditSelectorWidget->SetLabelText( "Transform Node: ");
    this->TransformEditSelectorWidget->SetBalloonHelpString("select a node from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->TransformEditSelectorWidget->GetWidgetName());


    this->MatrixWidget = vtkKWMatrix4x4::New();
    this->MatrixWidget->SetParent( transformFrame->GetFrame() );
    this->MatrixWidget->Create();
    this->Script("pack %s -side top -anchor e -padx 2 -pady 2", 
                this->MatrixWidget->GetWidgetName());


    this->TransformEditSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                           (vtkCommand *)this->GUICallbackCommand );  

    transformFrame->Delete();
    
}





