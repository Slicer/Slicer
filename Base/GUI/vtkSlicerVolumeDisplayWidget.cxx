#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerVolumeDisplayWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::vtkSlicerVolumeDisplayWidget ( )
{

    this->VolumeNode = NULL;
    this->VolumeDisplayNode = NULL;

    this->VolumeSelectorWidget = NULL;
    this->WindowLevelThresholdEditor = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::~vtkSlicerVolumeDisplayWidget ( )
{
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->Delete();
    }
  if (this->WindowLevelThresholdEditor)
    {
    this->WindowLevelThresholdEditor->Delete();
    }
  
  this->SetMRMLScene ( NULL );
  this->SetVolumeNode (NULL);
  this->SetVolumeDisplayNode (NULL);
  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "VolumeNode ID: " << this->GetVolumeNode( )->GetID() << "\n";
    os << indent << "VolumeDisplayNode ID: " << this->GetVolumeDisplayNode( )->GetID() << "\n";
    // print widgets?
}

void vtkSlicerVolumeDisplayWidget::SetVolumeNode ( vtkMRMLVolumeNode *node )
{ 
  this->SetMRML ( vtkObjectPointer( &this->VolumeNode), node );
  this->VolumeSelectorWidget->SetSelected(node); 
  if ( node )
    {
    this->ProcessMRMLEvents(node, vtkCommand::ModifiedEvent, NULL);
    this->ProcessMRMLEvents(node->GetDisplayNode(), vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  vtkKWMenu *volSelectorMenu = vtkKWMenu::SafeDownCast(caller);
  //vtkSlicerNodeSelectorWidget *volSelector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (volSelectorMenu == this->VolumeSelectorWidget->GetWidget()->GetWidget()->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ) 
    {
    vtkMRMLVolumeNode *volume = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (volume != NULL)
      {
      this->SetVolumeNode(volume);
      this->ProcessMRMLEvents(volume, vtkCommand::ModifiedEvent, NULL);
      this->ProcessMRMLEvents(volume->GetDisplayNode(), vtkCommand::ModifiedEvent, NULL);
      }
    return;
    }
  
  vtkKWWindowLevelThresholdEditor *editor = vtkKWWindowLevelThresholdEditor::SafeDownCast(caller);
  if (editor == this->WindowLevelThresholdEditor && event == vtkKWWindowLevelThresholdEditor::ValueChangedEvent)
    {
    if (this->VolumeDisplayNode == NULL)
      {
      this->VolumeDisplayNode = vtkMRMLVolumeDisplayNode::New();
      this->VolumeDisplayNode->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(this->VolumeDisplayNode);
      this->VolumeNode->SetDisplayNodeID(this->VolumeDisplayNode->GetID());
      }
    this->VolumeDisplayNode->SetWindow(this->WindowLevelThresholdEditor->GetWindow());
    this->VolumeDisplayNode->SetLevel(this->WindowLevelThresholdEditor->GetLevel());
    this->VolumeDisplayNode->SetUpperThreshold(this->WindowLevelThresholdEditor->GetUpperThreshold());
    this->VolumeDisplayNode->SetLowerThreshold(this->WindowLevelThresholdEditor->GetLowerThreshold());
    this->VolumeDisplayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());
    this->VolumeDisplayNode->SetAutoThreshold(this->WindowLevelThresholdEditor->GetAutoThreshold());
    this->VolumeDisplayNode->SetApplyThreshold(this->WindowLevelThresholdEditor->GetApplyThreshold());
    return;
    }       
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if ( !this->VolumeNode )
    {
    return;
    }
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(caller);
  if (volumeNode == this->VolumeNode && event == vtkCommand::ModifiedEvent)
    {
    this->SetVolumeDisplayNode(volumeNode->GetDisplayNode());
    this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
    return;
    }

  if ( !this->VolumeDisplayNode )
    {
    return;
    }

  vtkMRMLVolumeDisplayNode *volumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(caller);
  if (volumeDisplayNode == this->VolumeDisplayNode && event == vtkCommand::ModifiedEvent)
    {
    this->WindowLevelThresholdEditor->SetWindowLevel(volumeDisplayNode->GetWindow(), volumeDisplayNode->GetLevel() );
    this->WindowLevelThresholdEditor->SetThreshold(volumeDisplayNode->GetLowerThreshold(), volumeDisplayNode->GetUpperThreshold() );
    this->WindowLevelThresholdEditor->SetAutoWindowLevel( this->VolumeDisplayNode->GetAutoWindowLevel() );
    this->WindowLevelThresholdEditor->SetAutoThreshold( this->VolumeDisplayNode->GetAutoThreshold() );
    this->WindowLevelThresholdEditor->SetApplyThreshold( this->VolumeDisplayNode->GetApplyThreshold() );
    
    return;
    }
  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::CreateWidget ( )
{
    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *volDisplayFrame = vtkKWFrameWithLabel::New ( );
    volDisplayFrame->SetParent ( this->GetParent() );
    volDisplayFrame->Create ( );
    volDisplayFrame->SetLabelText ("Display");
    volDisplayFrame->SetDefaultLabelFontWeightToNormal( );
    volDisplayFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  volDisplayFrame->GetWidgetName() );

    this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->VolumeSelectorWidget->SetParent ( volDisplayFrame->GetFrame() );
    this->VolumeSelectorWidget->Create ( );
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode");
    this->VolumeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    //this->VolumeSelectorWidget->UpdateMenu();
    this->VolumeSelectorWidget->SetBorderWidth(2);
    this->VolumeSelectorWidget->SetReliefToGroove();
    this->VolumeSelectorWidget->SetPadX(2);
    this->VolumeSelectorWidget->SetPadY(2);
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->VolumeSelectorWidget->SetLabelText( "Volume Select: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("select a volume from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->VolumeSelectorWidget->GetWidgetName());

    this->WindowLevelThresholdEditor = vtkKWWindowLevelThresholdEditor::New();
    this->WindowLevelThresholdEditor->SetParent ( volDisplayFrame->GetFrame() );
    this->WindowLevelThresholdEditor->Create ( );
    if (this->VolumeNode != NULL)
      {
      this->WindowLevelThresholdEditor->SetImageData(this->VolumeNode->GetImageData());
      }
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->WindowLevelThresholdEditor->GetWidgetName() );


    this->VolumeSelectorWidget->GetWidget()->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

    volDisplayFrame->Delete();
    
}





