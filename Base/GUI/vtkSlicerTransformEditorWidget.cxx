#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWMatrix4x4.h"
#include "vtkTransform.h"

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

  this->RotationScaleLR = NULL;
  this->RotationScalePA = NULL;
  this->RotationScaleIS = NULL;

  this->RotationCoordinateSystemMenu = NULL;

  this->ProcessingCallback = false;

  this->RotationAxis = -1;

  this->RotationMatrix = vtkMatrix4x4::New();

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

  if (this->RotationScaleLR)
    {
    this->RotationScaleLR->SetParent(NULL);
    this->RotationScaleLR->Delete();
    this->RotationScaleLR = NULL;
    } 
  if (this->RotationScalePA)
    {
    this->RotationScalePA->SetParent(NULL);
    this->RotationScalePA->Delete();
    this->RotationScalePA = NULL;
    } 
  if (this->RotationScaleIS)
    {
    this->RotationScaleIS->SetParent(NULL);
    this->RotationScaleIS->Delete();
    this->RotationScaleIS = NULL;
    } 

  if (this->RotationCoordinateSystemMenu)
    {
    this->RotationCoordinateSystemMenu->SetParent(NULL);
    this->RotationCoordinateSystemMenu->Delete();
    this->RotationCoordinateSystemMenu = NULL;
    }

  if (this->RotationMatrix)
    {
    this->RotationMatrix->Delete();
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
void vtkSlicerTransformEditorWidget::UpdateMatrix ()
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
      this->UpdateTranslationSliders();
      this->ResetRotationSliders(-1);
      }
    else
      {
      this->MatrixWidget->EnabledOff();
      }
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

  if ( ( this->TransformEditSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
         && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
       || this->MatrixWidget->GetMatrix4x4() == NULL ) 
    {
    this->UpdateMatrix();
    }
  else if ( this->IdentityButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
    if (node != NULL)
      {
      this->MRMLScene->SaveStateForUndo(node);
      }
    if (this->MatrixWidget->GetMatrix4x4() == NULL)
      {
      this->UpdateMatrix();
      }

    this->MatrixWidget->GetMatrix4x4()->Identity();
    this->MatrixWidget->UpdateWidget();
    this->RotationAxis = -1;
    this->ResetRotationSliders(this->RotationAxis);
    }
  else if ( this->InvertButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
    if (node != NULL)
      {
      this->MRMLScene->SaveStateForUndo(node);
      }

    if (this->MatrixWidget->GetMatrix4x4() == NULL)
      {
      this->UpdateMatrix();
      }

    this->MatrixWidget->GetMatrix4x4()->Invert();
    this->MatrixWidget->UpdateWidget();
    this->RotationAxis = -1;  
    this->ResetRotationSliders(this->RotationAxis);
    }
  else if (this->RotationCoordinateSystemMenu == vtkKWMenuButtonWithLabel::SafeDownCast(caller))
    {
    vtkKWMenuButton *mb = this->RotationCoordinateSystemMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Global") )   
      {
      this->RotationAxis = -1;
      this->ResetRotationSliders(this->RotationAxis);
      }
    else if ( !strcmp (mb->GetValue(), "Local") )   
      {
      this->RotationAxis = -1;  
      this->ResetRotationSliders(this->RotationAxis);

      }
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
  this->RotationCoordinateSystemMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);

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

    // Rotation FRAME            
    vtkKWFrameWithLabel *rotateFrame = vtkKWFrameWithLabel::New ( );
    rotateFrame->SetParent ( transformFrame->GetFrame() );
    rotateFrame->Create ( );
    rotateFrame->SetLabelText ("Rotation");
    //rotateFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   rotateFrame->GetWidgetName() );
    
    this->RotationScaleLR =  vtkKWScaleWithEntry::New() ;
    this->RotationScaleLR->SetParent( rotateFrame->GetFrame() );
    this->RotationScaleLR->Create();
    this->RotationScaleLR->SetLabelText("LR");
    this->RotationScaleLR->SetWidth ( 20 );
    this->RotationScaleLR->SetRange(-200, 200);
    this->RotationScaleLR->SetStartCommand(this, "RotationLRChangingCallback");
    this->RotationScaleLR->SetCommand(this, "RotationLRChangingCallback");
    this->RotationScaleLR->SetEndCommand(this, "RotationLRChangedCallback");
    this->RotationScaleLR->SetEntryCommand(this, "RotationLRChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->RotationScaleLR->GetWidgetName());

    this->RotationScalePA =  vtkKWScaleWithEntry::New() ;
    this->RotationScalePA->SetParent( rotateFrame->GetFrame() );
    this->RotationScalePA->Create();
    this->RotationScalePA->SetRange(-200, 200);
    this->RotationScalePA->SetLabelText("PA");
    this->RotationScalePA->SetWidth ( 20 );
    this->RotationScalePA->SetStartCommand(this, "RotationPAChangingCallback");
    this->RotationScalePA->SetCommand(this, "RotationPAChangingCallback");
    this->RotationScalePA->SetEndCommand(this, "RotationPAChangedCallback");
    this->RotationScalePA->SetEntryCommand(this, "RotationPAChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->RotationScalePA->GetWidgetName());

    this->RotationScaleIS =  vtkKWScaleWithEntry::New() ;
    this->RotationScaleIS->SetParent( rotateFrame->GetFrame() );
    this->RotationScaleIS->Create();
    this->RotationScaleIS->SetRange(-200, 200);
    this->RotationScaleIS->SetLabelText("IS");
    this->RotationScaleIS->SetWidth ( 20 );
    this->RotationScaleIS->SetStartCommand(this, "RotationISChangingCallback");
    this->RotationScaleIS->SetCommand(this, "RotationISChangingCallback");
    this->RotationScaleIS->SetEndCommand(this, "RotationISChangedCallback");
    this->RotationScaleIS->SetEntryCommand(this, "RotationISChangedCallback");
    this->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                  this->RotationScaleIS->GetWidgetName());

    this->RotationCoordinateSystemMenu = vtkKWMenuButtonWithLabel::New();
    this->RotationCoordinateSystemMenu->SetParent(transformFrame->GetFrame());
    this->RotationCoordinateSystemMenu->Create();
    this->RotationCoordinateSystemMenu->SetLabelWidth(16);
    this->RotationCoordinateSystemMenu->SetLabelText("Coordinate Reference:");
    this->RotationCoordinateSystemMenu->GetWidget()->GetMenu()->AddRadioButton ( "Global");
    this->RotationCoordinateSystemMenu->GetWidget()->GetMenu()->AddRadioButton ( "Local");
    this->RotationCoordinateSystemMenu->GetWidget()->SetValue ( "Global" );
    this->Script(
                 "pack %s -side left -anchor e -expand n -fill x -padx 5 -pady 10", 
                 this->RotationCoordinateSystemMenu->GetWidgetName());




    this->IdentityButton = vtkKWPushButton::New();
    this->IdentityButton->SetParent( transformFrame->GetFrame() );
    this->IdentityButton->Create();
    this->IdentityButton->SetText("Identity");
    this->IdentityButton->SetWidth ( 8 );
    this->Script("pack %s -side left -anchor e -padx 5 -pady 10", 
                  this->IdentityButton->GetWidgetName());

    this->InvertButton = vtkKWPushButton::New();
    this->InvertButton->SetParent( transformFrame->GetFrame() );
    this->InvertButton->Create();
    this->InvertButton->SetText("Invert");
    this->InvertButton->SetWidth ( 8 );
    this->Script("pack %s -side left -anchor e -padx 5 -pady 10", 
                  this->InvertButton->GetWidgetName());

    this->TransformEditSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                           (vtkCommand *)this->GUICallbackCommand );  
    this->IdentityButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->InvertButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->RotationCoordinateSystemMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, this->GUICallbackCommand);

    transformFrame->Delete();
    translateFrame->Delete();
    rotateFrame->Delete();
}

void vtkSlicerTransformEditorWidget::TransformChangedCallback(double)
{
 if (this->ProcessingCallback)
    {
    return;
    }
  this->ProcessingCallback = true;

  vtkTransform* transform = this->GetCurrentTransformInCoordinateSystem();
  // will update when the node value changes
  vtkMatrix4x4 *matrix = transform->GetMatrix();

  transform->SetMatrix(matrix);

  double transLR = this->TranslationScaleLR->GetValue() - matrix->GetElement(0,3);
  double transPA = this->TranslationScalePA->GetValue() - matrix->GetElement(1,3);
  double transIS = this->TranslationScaleIS->GetValue() - matrix->GetElement(2,3);

  transform->Translate(transLR, transPA, transIS);

  matrix = transform->GetMatrix();

  if (this->MatrixWidget->GetMatrix4x4() == NULL)
    {
    this->UpdateMatrix();
    }

  this->MatrixWidget->EnabledOn();
  if ( this->MatrixWidget->GetMatrix4x4() != NULL )
    {
    this->MatrixWidget->GetMatrix4x4()->DeepCopy(matrix);

    this->MatrixWidget->UpdateWidget();
    }

  transform->Delete();

  this->RotationAxis = -1;
  this->ResetRotationSliders(this->RotationAxis);

  this->ProcessingCallback = false;
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

////----------------------------
void vtkSlicerTransformEditorWidget::RotationLRChangedCallback(double value)
{
 this->RotationChangedCallback(0, value);
}

void vtkSlicerTransformEditorWidget::RotationLRChangingCallback(double value)
{
 this->RotationChangingCallback(0, value);
}

void vtkSlicerTransformEditorWidget::RotationPAChangedCallback(double value)
{
 this->RotationChangedCallback(1, value);
}

void vtkSlicerTransformEditorWidget::RotationPAChangingCallback(double value)
{
 this->RotationChangingCallback(1, value);
}

void vtkSlicerTransformEditorWidget::RotationISChangedCallback(double value)
{
 this->RotationChangedCallback(2, value);
}

void vtkSlicerTransformEditorWidget::RotationISChangingCallback(double value)
{
 this->RotationChangingCallback(2, value);
}

///----
void vtkSlicerTransformEditorWidget::RotationChangedCallback(int axis, double value)
{
 if (this->ProcessingCallback)
    {
    return;
    }
  this->ProcessingCallback = true;

  this->ResetRotationSliders(axis);

  vtkTransform* transform = this->GetCurrentTransformInCoordinateSystem();


  if (axis != this->RotationAxis) 
    {
    this->RotationAxis = axis;
    transform->GetMatrix(this->RotationMatrix);
    }

  transform->SetMatrix(this->RotationMatrix);
  if (axis == 0) 
    {
    transform->RotateX(value);
    }
  else if (axis == 1) 
    {
    transform->RotateY(value);
    }
  else if (axis == 2) 
    {
    transform->RotateZ(value);
    }

  vtkMatrix4x4 *matrix = transform->GetMatrix();

  if (this->MatrixWidget->GetMatrix4x4() == NULL)
    {
    this->UpdateMatrix();
    }

  this->MatrixWidget->EnabledOn();
  if ( this->MatrixWidget->GetMatrix4x4() != NULL )
    {
    this->MatrixWidget->GetMatrix4x4()->DeepCopy(matrix);

    this->MatrixWidget->UpdateWidget();
    }

  this->MatrixWidget->UpdateWidget();

  transform->Delete();

  this->ProcessingCallback = false;
}


void vtkSlicerTransformEditorWidget::RotationChangingCallback(int axis, double val)
{
  if (this->ProcessingCallback)
    {
    return;
    }

  vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
  if (node != NULL)
    {
    this->MRMLScene->SaveStateForUndo(node);
    this->RotationChangedCallback(axis, val);
    }
}

///----------------------------------------------
void vtkSlicerTransformEditorWidget::UpdateTranslationSliders()
{
   if (this->MatrixWidget->GetMatrix4x4() != NULL)
      {
      this->TranslationScaleLR->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(0,3));
      this->TranslationScalePA->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(1,3));
      this->TranslationScaleIS->SetValue(this->MatrixWidget->GetMatrix4x4()->GetElement(2,3));
      }
}

void vtkSlicerTransformEditorWidget::ResetRotationSliders(int axis)
{
  if (axis == -1)
    {
    this->RotationScaleLR->SetValue(0);
    this->RotationScalePA->SetValue(0);
    this->RotationScaleIS->SetValue(0);
    }
  if (axis == 0)
    {
    this->RotationScalePA->SetValue(0);
    this->RotationScaleIS->SetValue(0);
    }
  if (axis == 1)
    {
    this->RotationScaleLR->SetValue(0);
    this->RotationScaleIS->SetValue(0);
    }
  if (axis == 2)
    {
    this->RotationScaleLR->SetValue(0);
    this->RotationScalePA->SetValue(0);
    }

}

vtkTransform* 
vtkSlicerTransformEditorWidget::GetCurrentTransformInCoordinateSystem()
{
  vtkTransform* transform = vtkTransform::New();
  transform->Identity();

  vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformEditSelectorWidget->GetSelected());
  if (node != NULL)
    {
    vtkMatrix4x4 *matrix = node->GetMatrixTransformToParent();

    if (matrix)
      {
      transform->SetMatrix(matrix);
      }

    vtkKWMenuButton *mb = this->RotationCoordinateSystemMenu->GetWidget();
    if ( !strcmp (mb->GetValue(), "Global") )   
      {
        transform->PostMultiply();
      }
    else if ( !strcmp (mb->GetValue(), "Local") )   
      {
        transform->PreMultiply();       
      }
    }
  return transform;
}
