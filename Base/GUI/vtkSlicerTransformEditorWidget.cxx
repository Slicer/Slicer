#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
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
  this->IdentityButton = NULL;
  this->InvertButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformEditorWidget::~vtkSlicerTransformEditorWidget ( )
{
  if (this->TransformEditSelectorWidget)
    {
    this->TransformEditSelectorWidget->SetParent(NULL);
    this->TransformEditSelectorWidget->Delete();
    this->TransformEditSelectorWidget = NULL;
    }  
  if (this->MatrixWidget)
    {
    this->MatrixWidget->SetParent(NULL);
    this->MatrixWidget->Delete();
    this->MatrixWidget = NULL;
    }  

  if (this->IdentityButton)
    {
    this->IdentityButton->SetParent(NULL);
    this->IdentityButton->Delete();
    this->IdentityButton = NULL;
    } 
  if (this->InvertButton)
    {
    this->InvertButton->SetParent(NULL);
    this->InvertButton->Delete();
    this->InvertButton = NULL;
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
      // TODO: there should be a observer added to the transform node so the matrix
      // will update when the node value changes
      this->MatrixWidget->EnabledOn();
      this->MatrixWidget->SetAndObserveMatrix4x4(node->GetMatrixTransformToParent());
      this->MatrixWidget->UpdateWidget();
      }
    else
      {
      this->MatrixWidget->EnabledOff();
      }

    return;
    }
  else if ( this->IdentityButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
  {
    this->MatrixWidget->GetMatrix4x4()->Identity();
    this->MatrixWidget->UpdateWidget();
  }
  else if ( this->InvertButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
  {
    this->MatrixWidget->GetMatrix4x4()->Invert();
    this->MatrixWidget->UpdateWidget();
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
  this->IdentityButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InvertButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
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

    this->IdentityButton = vtkKWPushButton::New();
    this->IdentityButton->SetParent( transformFrame->GetFrame() );
    this->IdentityButton->Create();
    this->IdentityButton->SetText("Identity");
    this->IdentityButton->SetWidth ( 8 );
    this->Script("pack %s -side left -anchor e -padx 20 -pady 10", 
                  this->IdentityButton->GetWidgetName());

    this->InvertButton = vtkKWPushButton::New();
    this->InvertButton->SetParent( transformFrame->GetFrame() );
    this->InvertButton->Create();
    this->InvertButton->SetText("Invert");
    this->InvertButton->SetWidth ( 8 );
    this->Script("pack %s -side left -anchor e -padx 20 -pady 10", 
                  this->InvertButton->GetWidgetName());

    this->TransformEditSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                           (vtkCommand *)this->GUICallbackCommand );  
    this->IdentityButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->InvertButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );


    transformFrame->Delete();
    
}





