#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWScaleWithEntry.h"

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
  this->TranslationScaleLR = NULL;
  this->TranslationScalePA = NULL;
  this->TranslationScaleIS = NULL;
  this->ProcessingCallback = false;
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
  if (this->TranslationScaleLR)
    {
    this->TranslationScaleLR->SetParent(NULL);
    this->TranslationScaleLR->Delete();
    this->TranslationScaleLR = NULL;
    } 
  if (this->TranslationScalePA)
    {
    this->TranslationScalePA->SetParent(NULL);
    this->TranslationScalePA->Delete();
    this->TranslationScalePA = NULL;
    } 
  if (this->TranslationScaleIS)
    {
    this->TranslationScaleIS->SetParent(NULL);
    this->TranslationScaleIS->Delete();
    this->TranslationScaleIS = NULL;
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
  if (this->ProcessingCallback)
    {
    return;
    }

  this->ProcessingCallback = true;

  if (this->TransformEditSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
       && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());

    if (node != NULL)
      {
      // will update when the node value changes
      vtkMatrix4x4 *matrix = node->GetMatrixTransformToParent();
      this->MatrixWidget->EnabledOn();
      this->MatrixWidget->SetAndObserveMatrix4x4(matrix);
      this->MatrixWidget->UpdateWidget();
      this->MatrixWidget->GetMatrix4x4()->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }
    else
      {
      this->MatrixWidget->EnabledOff();
      }
    }
  else if ( this->IdentityButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
    if (node != NULL)
      {
      this->MRMLScene->SaveStateForUndo(node);
      }

    this->MatrixWidget->GetMatrix4x4()->Identity();
    this->MatrixWidget->UpdateWidget();
    }
  else if ( this->InvertButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
    if (node != NULL)
      {
      this->MRMLScene->SaveStateForUndo(node);
      }

    this->MatrixWidget->GetMatrix4x4()->Invert();
    this->MatrixWidget->UpdateWidget();
    }
    this->ProcessingCallback = false;

} 



//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if (this->MatrixWidget->GetMatrix4x4() == vtkMatrix4x4::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent)
    {
    this->UpdateTranslationSliders();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTransformEditorWidget::RemoveWidgetObservers ( ) {

  this->TransformEditSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->IdentityButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InvertButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  if ( this->MatrixWidget->GetMatrix4x4() )
    {
    this->MatrixWidget->GetMatrix4x4()->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
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

    // Translation FRAME            
    vtkKWFrameWithLabel *translateFrame = vtkKWFrameWithLabel::New ( );
    translateFrame->SetParent ( transformFrame->GetFrame() );
    translateFrame->Create ( );
    translateFrame->SetLabelText ("Translation");
    //transformFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   translateFrame->GetWidgetName() );
    
    this->TranslationScaleLR =  vtkKWScaleWithEntry::New() ;
    this->TranslationScaleLR->SetParent( translateFrame->GetFrame() );
    this->TranslationScaleLR->Create();
    this->TranslationScaleLR->SetLabelText("LR");
    this->TranslationScaleLR->SetWidth ( 20 );
    this->TranslationScaleLR->SetRange(-200, 200);
    this->TranslationScaleLR->SetStartCommand(this, "TransformChangingCallback");
    this->TranslationScaleLR->SetCommand(this, "TransformChangingCallback");
    this->TranslationScaleLR->SetEndCommand(this, "TransformChangedCallback");
    this->TranslationScaleLR->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->TranslationScaleLR->GetWidgetName());

    this->TranslationScalePA =  vtkKWScaleWithEntry::New() ;
    this->TranslationScalePA->SetParent( translateFrame->GetFrame() );
    this->TranslationScalePA->Create();
    this->TranslationScalePA->SetRange(-200, 200);
    this->TranslationScalePA->SetLabelText("PA");
    this->TranslationScalePA->SetWidth ( 20 );
    this->TranslationScalePA->SetStartCommand(this, "TransformChangingCallback");
    this->TranslationScalePA->SetCommand(this, "TransformChangingCallback");
    this->TranslationScalePA->SetEndCommand(this, "TransformChangedCallback");
    this->TranslationScalePA->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->TranslationScalePA->GetWidgetName());

    this->TranslationScaleIS =  vtkKWScaleWithEntry::New() ;
    this->TranslationScaleIS->SetParent( translateFrame->GetFrame() );
    this->TranslationScaleIS->Create();
    this->TranslationScaleIS->SetRange(-200, 200);
    this->TranslationScaleIS->SetLabelText("IS");
    this->TranslationScaleIS->SetWidth ( 20 );
    this->TranslationScaleIS->SetStartCommand(this, "TransformChangingCallback");
    this->TranslationScaleIS->SetCommand(this, "TransformChangingCallback");
    this->TranslationScaleIS->SetEndCommand(this, "TransformChangedCallback");
    this->TranslationScaleIS->SetEntryCommand(this, "TransformChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->TranslationScaleIS->GetWidgetName());

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
    translateFrame->Delete();
}

void vtkSlicerTransformEditorWidget::TransformChangedCallback(double)
{
 if (this->ProcessingCallback)
    {
    return;
    }

  vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
  if (node != NULL)
    {
    this->ProcessingCallback = true;
    // will update when the node value changes
    vtkMatrix4x4 *matrix = node->GetMatrixTransformToParent();
    matrix->SetElement(0,3, this->TranslationScaleLR->GetValue());
    matrix->SetElement(1,3, this->TranslationScalePA->GetValue());
    matrix->SetElement(2,3, this->TranslationScaleIS->GetValue());
    this->MatrixWidget->EnabledOn();
    this->MatrixWidget->UpdateWidget();
    this->ProcessingCallback = false;
    }
}

void vtkSlicerTransformEditorWidget::TransformChangingCallback(double val)
{
  if (this->ProcessingCallback)
    {
    return;
    }

  vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
  if (node != NULL)
    {
    this->MRMLScene->SaveStateForUndo(node);
    this->TransformChangedCallback(val);
    }
}

void vtkSlicerTransformEditorWidget::UpdateTranslationSliders()
{
   if (this->MatrixWidget->GetMatrix4x4() != NULL)
      {
      this->TranslationScaleLR->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(0,3));
      this->TranslationScalePA->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(1,3));
      this->TranslationScaleIS->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(2,3));
      }
}
