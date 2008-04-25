#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerScalarVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerScalarVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerScalarVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerScalarVolumeDisplayWidget::vtkSlicerScalarVolumeDisplayWidget ( )
{
    this->ColorSelectorWidget = NULL;
    this->WindowLevelThresholdEditor = NULL;
    this->InterpolateButton = NULL;
    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;
}


//---------------------------------------------------------------------------
vtkSlicerScalarVolumeDisplayWidget::~vtkSlicerScalarVolumeDisplayWidget ( )
{
  if (this->IsCreated())
    {
    this->RemoveWidgetObservers();
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
  if (this->InterpolateButton)
    {
    this->InterpolateButton->SetParent(NULL);
    this->InterpolateButton->Delete();
    this->InterpolateButton = NULL;
    }

  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerScalarVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingWidget = 1;

  this->Superclass::ProcessWidgetEvents(caller, event, callData);
  
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
      vtkMRMLVolumeDisplayNode *displayNode = this->GetVolumeDisplayNode();
      if (displayNode != NULL && displayNode->GetColorNodeID() != NULL)
        {
        // set and observe it's colour node id
        if (strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
          {
          // there's a change, set it
          displayNode->SetAndObserveColorNodeID(color->GetID());
          }
        }
      }
    this->UpdatingWidget = 0;
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
    vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());

    // 
    // check the volume -- if it doesn't yet have a display node,
    // we need to create one
    //

    if (displayNode==NULL)
      {
        vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
        if (volumeNode == NULL)
          {
          this->UpdatingWidget = 0;
          return;
          }
        else 
          {
          displayNode = vtkMRMLScalarVolumeDisplayNode::New ();
          displayNode->SetScene(this->MRMLScene);
          this->MRMLScene->AddNode (displayNode);
          displayNode->Delete();
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
          }

      volumeNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
      }

    if ( displayNode )
      {
      displayNode->DisableModifiedEventOn();
      displayNode->SetWindow(this->WindowLevelThresholdEditor->GetWindow());
      displayNode->SetLevel(this->WindowLevelThresholdEditor->GetLevel());
      displayNode->SetUpperThreshold(this->WindowLevelThresholdEditor->GetUpperThreshold());
      displayNode->SetLowerThreshold(this->WindowLevelThresholdEditor->GetLowerThreshold());
      displayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());
      int thresholdType = this->WindowLevelThresholdEditor->GetThresholdType();
      if (thresholdType == vtkKWWindowLevelThresholdEditor::ThresholdOff) 
        {
        displayNode->SetApplyThreshold(0);
        }
      else if (thresholdType == vtkKWWindowLevelThresholdEditor::ThresholdAuto) 
        {
        displayNode->SetApplyThreshold(1);
        displayNode->SetAutoThreshold(1);
        }
      else if (thresholdType == vtkKWWindowLevelThresholdEditor::ThresholdManual) 
        {
        displayNode->SetApplyThreshold(1);
        displayNode->SetAutoThreshold(0);
        }
      displayNode->DisableModifiedEventOff();
      displayNode->InvokePendingModifiedEvent();
      this->UpdatingWidget = 0;
      return;
      }
    }

  if (editor == this->WindowLevelThresholdEditor && 
        event == vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent)
    {
    vtkMRMLNode *displayNode = this->GetVolumeDisplayNode();
    if (displayNode != NULL)
      {
      this->MRMLScene->SaveStateForUndo(displayNode);
      }
    this->UpdatingWidget = 0;
    return;
    }


    if (this->InterpolateButton == vtkKWCheckButton::SafeDownCast(caller) && 
        event == vtkKWCheckButton::SelectedStateChangedEvent)
      {
      vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
      if (displayNode != NULL)
        {
        displayNode->SetInterpolate( this->InterpolateButton->GetSelectedState() );
        }
      this->UpdatingWidget = 0;
      return;
      }
}



//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingMRML = 1;

  vtkMRMLVolumeNode *curVolumeNode = this->GetVolumeNode();
  if (curVolumeNode  == NULL)
    {
    this->UpdatingMRML = 0;
    return;
    }

  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(caller);
  if (volumeNode == curVolumeNode && 
      volumeNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    if (volumeNode && volumeNode->GetImageData())
      {
      this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
      }
    // check the interpolation which may have been modified from the SliceGUI
    vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    if (displayNode && this->InterpolateButton )
      {
      if (displayNode->GetInterpolate() != this->InterpolateButton->GetSelectedState() )
        {
        this->InterpolateButton->SetSelectedState( displayNode->GetInterpolate()  );
        }
      }
    }

  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidgetFromMRML();
    this->UpdatingMRML = 0;
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::UpdateWidgetFromMRML ()
{
  vtkDebugMacro("UpdateWidgetFromMRML");
  
  vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
  if (volumeNode != NULL && this->WindowLevelThresholdEditor)
    {
    this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
    }

  // check to see if the color selector widget has it's mrml scene set (it
  // could have been set to null)
  if ( this->ColorSelectorWidget )
    {
    if (this->GetMRMLScene() != NULL &&
        this->ColorSelectorWidget->GetMRMLScene() == NULL)
      {
      vtkDebugMacro("UpdateWidgetFromMRML: resetting the color selector's mrml scene");
      this->ColorSelectorWidget->SetMRMLScene(this->GetMRMLScene());
      }
    }  
  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
  
  if (displayNode != NULL && this->WindowLevelThresholdEditor) 
    {
    this->WindowLevelThresholdEditor->SetWindowLevel(
          displayNode->GetWindow(), displayNode->GetLevel() );
    this->WindowLevelThresholdEditor->SetThreshold(
          displayNode->GetLowerThreshold(), displayNode->GetUpperThreshold() );
    this->WindowLevelThresholdEditor->SetAutoWindowLevel( displayNode->GetAutoWindowLevel() );
    if (displayNode->GetApplyThreshold() == 0) 
      {
      this->WindowLevelThresholdEditor->SetThresholdType(vtkKWWindowLevelThresholdEditor::ThresholdOff);
      }
    else if (displayNode->GetAutoThreshold())
      {
      this->WindowLevelThresholdEditor->SetThresholdType(vtkKWWindowLevelThresholdEditor::ThresholdAuto);
      }
    else
      {
      this->WindowLevelThresholdEditor->SetThresholdType(vtkKWWindowLevelThresholdEditor::ThresholdManual);
      }
    // set the color node selector to reflect the volume's color node
    this->ColorSelectorWidget->SetSelected(displayNode->GetColorNode());
    this->InterpolateButton->SetSelectedState( displayNode->GetInterpolate()  );
    }
  
  return;

}

//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::AddWidgetObservers ( )
{
  this->Superclass::AddWidgetObservers();
  
   this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
   this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
   this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
   this->InterpolateButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}

//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{
  this->Superclass::RemoveWidgetObservers();

  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InterpolateButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->WindowLevelThresholdEditor->SetImageData(NULL); 
}


//---------------------------------------------------------------------------
void vtkSlicerScalarVolumeDisplayWidget::CreateWidget ( )
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
    //vtkKWFrame *volDisplayFrame = vtkKWFrame::New ( );
    //volDisplayFrame->SetParent ( this->GetParent() );
    //volDisplayFrame->Create ( );
    //this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    //              volDisplayFrame->GetWidgetName() );

    vtkKWWidget *volDisplayFrame = this->GetParent();
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
    this->ColorSelectorWidget->SetBalloonHelpString("select a color from the current mrml scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->ColorSelectorWidget->GetWidgetName());

    this->InterpolateButton = vtkKWCheckButton::New();
    this->InterpolateButton->SetParent(volDisplayFrame);
    this->InterpolateButton->Create();
    this->InterpolateButton->SelectedStateOn();
    this->InterpolateButton->SetText("Interpolate");
    this->Script(
      "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
      this->InterpolateButton->GetWidgetName());

    this->WindowLevelThresholdEditor = vtkKWWindowLevelThresholdEditor::New();
    this->WindowLevelThresholdEditor->SetParent ( volDisplayFrame );
    this->WindowLevelThresholdEditor->Create ( );
    vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
    if (volumeNode != NULL)
      {
      this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
      }
    this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
                  this->WindowLevelThresholdEditor->GetWidgetName() );

   this->AddWidgetObservers();
    if (this->MRMLScene != NULL)
      {
      this->SetAndObserveMRMLScene(this->MRMLScene);
      }

    //volDisplayFrame->Delete();

}
