#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::vtkSlicerVolumeDisplayWidget ( )
{

    this->VolumeNodeID = NULL;
    this->VolumeDisplayNodeID = NULL;

    this->VolumeSelectorWidget = NULL;
    this->ColorSelectorWidget = NULL;
    this->WindowLevelThresholdEditor = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerVolumeDisplayWidget::~vtkSlicerVolumeDisplayWidget ( )
{
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->SetParent(NULL);
    this->VolumeSelectorWidget->Delete();
    this->VolumeSelectorWidget = NULL;
    }
  if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  if (this->WindowLevelThresholdEditor)
    {
    this->WindowLevelThresholdEditor->SetParent(NULL);
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

void vtkSlicerVolumeDisplayWidget::SetVolumeNode ( vtkMRMLVolumeNode *volumeNode )
{ 
  // Select this volume node
  this->VolumeSelectorWidget->SetSelected(volumeNode); 

  // TODO: display node modified events are not being observed

  // 
  // Set the member variables and do a first process
  //
  if ( volumeNode != NULL)
    {  
    this->SetVolumeNodeID( volumeNode->GetID() );
    this->SetVolumeDisplayNodeID( volumeNode->GetDisplayNodeID() );
    this->ProcessMRMLEvents(volumeNode, vtkCommand::ModifiedEvent, NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  //
  // process volume selector events
  //
  vtkSlicerNodeSelectorWidget *volSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (volSelector == this->VolumeSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLVolumeNode *volume = 
        vtkMRMLVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());

    if (volume != NULL)
      {
      this->SetVolumeNode(volume);
      this->ProcessMRMLEvents(volume, vtkCommand::ModifiedEvent, NULL);
      }

    return;
    }

  //
  // process color selector events
  //
  vtkSlicerNodeSelectorWidget *colSelector = 
      vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (colSelector == this->ColorSelectorWidget && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLColorNode *color =
      vtkMRMLColorNode::SafeDownCast(this->ColorSelectorWidget->GetSelected());
    if (color != NULL)
      {
      // get the volume display node
      vtkMRMLVolumeDisplayNode *displayNode = NULL;
      if (this->VolumeDisplayNodeID != NULL)
        {
        displayNode = 
          vtkMRMLVolumeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID) );
        if (displayNode != NULL)
          {
          // set and observe it's colour node id
          if (strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
            {
            // there's a change, set it
            displayNode->SetAndObserveColorNodeID(color->GetID());
            }
          }        
        }
      }
    return;
    }
  //
  // process window/level/threshold events
  //
  vtkKWWindowLevelThresholdEditor *editor = 
      vtkKWWindowLevelThresholdEditor::SafeDownCast(caller);

  if (editor == this->WindowLevelThresholdEditor && 
        event == vtkKWWindowLevelThresholdEditor::ValueChangedEvent)
    {
    vtkMRMLVolumeDisplayNode *displayNode = NULL;
    if (this->VolumeDisplayNodeID != NULL)
      {
        displayNode = 
          vtkMRMLVolumeDisplayNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID) );
      }
    // 
    // check the volume -- if it doesn't yet have a display node,
    // we need to create one
    //

    if (VolumeNodeID != NULL && displayNode==NULL)
      {
        vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast (this->MRMLScene->GetNodeByID(this->VolumeNodeID) );
        if (volumeNode == NULL)
          {
          return;
          }
        else 
          {
          displayNode = vtkMRMLVolumeDisplayNode::New ();
          displayNode->SetScene(this->MRMLScene);
          this->MRMLScene->AddNode (displayNode);
          //displayNode->SetDefaultColorMap();
          if (this->GetApplication() &&
              vtkSlicerApplication::SafeDownCast(this->GetApplication()) &&
              vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Color") &&
              vtkSlicerColorGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Color")))
            {
            vtkSlicerColorLogic *colorLogic = vtkSlicerColorGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Color"))->GetLogic();
            
            if (colorLogic)
              {
              int isLabelMap = 0;
              if (vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode))
                {
                isLabelMap = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->GetLabelMap();
                }
              if (isLabelMap)
                {
                displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
                }
              else
                {
                displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
                }
              }
            else
              {
              vtkDebugMacro("Unable to get color logic\n");
              }
            }
          else
            {
            vtkDebugMacro("Unable to get application or color gui");
            }
          displayNode->Delete();
          }
        
      this->VolumeDisplayNodeID = displayNode->GetID();
      volumeNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
      }

    if ( displayNode )
      {
      displayNode->SetWindow(this->WindowLevelThresholdEditor->GetWindow());
      displayNode->SetLevel(this->WindowLevelThresholdEditor->GetLevel());
      displayNode->SetUpperThreshold(this->WindowLevelThresholdEditor->GetUpperThreshold());
      displayNode->SetLowerThreshold(this->WindowLevelThresholdEditor->GetLowerThreshold());
      displayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());
      displayNode->SetAutoThreshold(this->WindowLevelThresholdEditor->GetAutoThreshold());
      displayNode->SetApplyThreshold(this->WindowLevelThresholdEditor->GetApplyThreshold());
      return;
      }
    }       

  if (editor == this->WindowLevelThresholdEditor && 
        event == vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent)
    {
    if (this->VolumeDisplayNodeID != NULL)
      {
      vtkMRMLNode *displayNode = this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID);
      if (displayNode != NULL)
        {
        this->MRMLScene->SaveStateForUndo(this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID));
        }
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

  vtkMRMLVolumeNode *curVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->VolumeNodeID));

  if (curVolumeNode == NULL)
    {
    this->SetVolumeNode(NULL);
    this->SetVolumeDisplayNodeID(NULL);
    return;
    }

  if (volumeNode == curVolumeNode && 
      volumeNode != NULL && event == vtkCommand::ModifiedEvent)
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
    vtkMRMLVolumeDisplayNode *displayNode = 
        vtkMRMLVolumeDisplayNode::SafeDownCast(
            this->MRMLScene->GetNodeByID(this->VolumeDisplayNodeID));
    if (displayNode != NULL) 
      {
      this->WindowLevelThresholdEditor->SetWindowLevel(
            displayNode->GetWindow(), displayNode->GetLevel() );
      this->WindowLevelThresholdEditor->SetThreshold(
            displayNode->GetLowerThreshold(), displayNode->GetUpperThreshold() );
      this->WindowLevelThresholdEditor->SetAutoWindowLevel( displayNode->GetAutoWindowLevel() );
      this->WindowLevelThresholdEditor->SetAutoThreshold( displayNode->GetAutoThreshold() );
      this->WindowLevelThresholdEditor->SetApplyThreshold( displayNode->GetApplyThreshold() );
      // set the color node selector to reflect the volume's color node
      this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
      }
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeDisplayWidget::RemoveWidgetObservers ( ) {
    this->VolumeSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );

    this->WindowLevelThresholdEditor->SetImageData(NULL);
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
    vtkKWFrame *volDisplayFrame = vtkKWFrame::New ( );
    volDisplayFrame->SetParent ( this->GetParent() );
    volDisplayFrame->Create ( );
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  volDisplayFrame->GetWidgetName() );

    this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->VolumeSelectorWidget->SetParent ( volDisplayFrame );
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

    // a selector to change the color node associated with this display
    this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->ColorSelectorWidget->SetParent ( volDisplayFrame );
    this->ColorSelectorWidget->Create ( );
    this->ColorSelectorWidget->SetNodeClass("vtkMRMLColorNode", NULL, NULL, NULL);
    this->ColorSelectorWidget->ShowHiddenOn();
    this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->ColorSelectorWidget->SetBorderWidth(2);
    // this->ColorSelectorWidget->SetReliefToGroove();
    this->ColorSelectorWidget->SetPadX(2);
    this->ColorSelectorWidget->SetPadY(2);
    this->ColorSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ColorSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->ColorSelectorWidget->SetLabelText( "Color Select: ");
    this->ColorSelectorWidget->SetBalloonHelpString("select a volume from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->ColorSelectorWidget->GetWidgetName());

    
    this->WindowLevelThresholdEditor = vtkKWWindowLevelThresholdEditor::New();
    this->WindowLevelThresholdEditor->SetParent ( volDisplayFrame );
    this->WindowLevelThresholdEditor->Create ( );
    if (this->VolumeNodeID != NULL)
      {
      vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->VolumeNodeID));
      this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
      }
    this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                  this->WindowLevelThresholdEditor->GetWidgetName() );

    this->VolumeSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
    if (this->MRMLScene != NULL)
      {
      this->SetAndObserveMRMLScene(this->MRMLScene);
      }

    volDisplayFrame->Delete();
    
}
