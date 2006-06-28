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

    this->VolumeNodeID = NULL;
    this->VolumeDisplayNodeID = NULL;

    this->VolumeSelectorWidget = NULL;
    this->WindowLevelThresholdEditor = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::~vtkSlicerVolumeDisplayWidget ( )
{
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->Delete();
    this->VolumeSelectorWidget = NULL;
    }
  if (this->WindowLevelThresholdEditor)
    {
    this->WindowLevelThresholdEditor->Delete();
    this->WindowLevelThresholdEditor = NULL;
    }
  
  this->SetMRMLScene ( NULL );
  this->SetVolumeNodeID (NULL);
  this->SetVolumeDisplayNodeID (NULL);
  
}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "VolumeNode ID: " << this->GetVolumeNodeID() << "\n";
    os << indent << "VolumeDisplayNode ID: " << this->GetVolumeDisplayNodeID() << "\n";
    // print widgets?
}

void vtkSlicerVolumeDisplayWidget::SetVolumeNode ( vtkMRMLVolumeNode *node )
{ 
  this->VolumeSelectorWidget->SetSelected(node); 
  this->SetVolumeNodeID( node->GetID() );
  this->SetVolumeDisplayNodeID( node->GetDisplayNodeID() );
  if ( node )
    {
    this->ProcessMRMLEvents(node, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  vtkSlicerNodeSelectorWidget *volSelector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (volSelector == this->VolumeSelectorWidget && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLVolumeNode *volume = vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (volume != NULL)
      {
      this->SetVolumeNode(volume);
      this->ProcessMRMLEvents(volume, vtkCommand::ModifiedEvent, NULL);
      }
    return;
    }
  
  vtkKWWindowLevelThresholdEditor *editor = vtkKWWindowLevelThresholdEditor::SafeDownCast(caller);
  if (editor == this->WindowLevelThresholdEditor && event == vtkKWWindowLevelThresholdEditor::ValueChangedEvent)
    {
    vtkMRMLVolumeDisplayNode *displayNode = NULL;
    if (this->VolumeDisplayNodeID != NULL)
      {
        displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID) );
      }
    if (displayNode)
      {
      displayNode->SetWindow(this->WindowLevelThresholdEditor->GetWindow());
      displayNode->SetLevel(this->WindowLevelThresholdEditor->GetLevel());
      displayNode->SetUpperThreshold(this->WindowLevelThresholdEditor->GetUpperThreshold());
      displayNode->SetLowerThreshold(this->WindowLevelThresholdEditor->GetLowerThreshold());
      displayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());
      displayNode->SetAutoThreshold(this->WindowLevelThresholdEditor->GetAutoThreshold());
      displayNode->SetApplyThreshold(this->WindowLevelThresholdEditor->GetApplyThreshold());
      }
    return;
    }       
  if (editor == this->WindowLevelThresholdEditor && event == vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent)
    {
    if (this->VolumeDisplayNodeID != NULL)
      {
      this->MRMLScene->SaveStateForUndo(this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID));
      }
    return;
    }       
} 



//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if ( !this->VolumeNodeID )
    {
    return;
    }
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(caller);
  if (volumeNode == this->MRMLScene->GetNodeByID(this->VolumeNodeID) && volumeNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    if (volumeNode->GetDisplayNode())
    {
      this->SetVolumeDisplayNodeID(volumeNode->GetDisplayNode()->GetID());
    }
    this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
    }

  if ( !this->VolumeDisplayNodeID )
    {
    return;
    }

  if (event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLVolumeDisplayNode *displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID));
    if (displayNode != NULL) 
      {
      this->WindowLevelThresholdEditor->SetWindowLevel(displayNode->GetWindow(), displayNode->GetLevel() );
      this->WindowLevelThresholdEditor->SetThreshold(displayNode->GetLowerThreshold(), displayNode->GetUpperThreshold() );
      this->WindowLevelThresholdEditor->SetAutoWindowLevel( displayNode->GetAutoWindowLevel() );
      this->WindowLevelThresholdEditor->SetAutoThreshold( displayNode->GetAutoThreshold() );
      this->WindowLevelThresholdEditor->SetApplyThreshold( displayNode->GetApplyThreshold() );
      }
    return;
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::RemoveWidgetObservers ( ) {
    this->VolumeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::CreateWidget ( )
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
    vtkKWFrameWithLabel *volDisplayFrame = vtkKWFrameWithLabel::New ( );
    volDisplayFrame->SetParent ( this->GetParent() );
    volDisplayFrame->Create ( );
    volDisplayFrame->SetLabelText ("Display");
    volDisplayFrame->CollapseFrame ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  volDisplayFrame->GetWidgetName() );

    this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->VolumeSelectorWidget->SetParent ( volDisplayFrame->GetFrame() );
    this->VolumeSelectorWidget->Create ( );
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->VolumeSelectorWidget->SetBorderWidth(2);
    // this->VolumeSelectorWidget->SetReliefToGroove();
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
    if (this->VolumeNodeID != NULL)
      {
      vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->VolumeNodeID));
      this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
      }
    this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                  this->WindowLevelThresholdEditor->GetWidgetName() );

    this->VolumeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
    this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    if (this->MRMLScene != NULL)
      {
      this->MRMLScene->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }

    volDisplayFrame->Delete();
    
}





