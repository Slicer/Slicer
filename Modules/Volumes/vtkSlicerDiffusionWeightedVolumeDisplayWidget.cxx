#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerDiffusionWeightedVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"


#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"

#include "vtkImageExtractComponents.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerVolumesLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDiffusionWeightedVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerDiffusionWeightedVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDiffusionWeightedVolumeDisplayWidget::vtkSlicerDiffusionWeightedVolumeDisplayWidget ( )
{
    this->ExtractComponent = vtkImageExtractComponents::New();

    this->DiffusionSelectorWidget = NULL;
    this->ColorSelectorWidget = NULL;
    this->InterpolateButton = NULL;
    this->WindowLevelThresholdEditor = NULL;
    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;
}


//---------------------------------------------------------------------------
vtkSlicerDiffusionWeightedVolumeDisplayWidget::~vtkSlicerDiffusionWeightedVolumeDisplayWidget ( )
{
  this->ExtractComponent->Delete();

  if (this->IsCreated())
    {
    this->RemoveWidgetObservers();
    }
 
 if (this->DiffusionSelectorWidget)
    {
    this->DiffusionSelectorWidget->SetParent(NULL);
    this->DiffusionSelectorWidget->Delete();
    this->DiffusionSelectorWidget = NULL;
    }

  if (this->ColorSelectorWidget)
    {
    this->ColorSelectorWidget->SetParent(NULL);
    this->ColorSelectorWidget->Delete();
    this->ColorSelectorWidget = NULL;
    }
  if (this->InterpolateButton)
    {
    this->InterpolateButton->SetParent(NULL);
    this->InterpolateButton->Delete();
    this->InterpolateButton = NULL;
    }
  if (this->WindowLevelThresholdEditor)
    {
    this->WindowLevelThresholdEditor->SetParent(NULL);
    this->WindowLevelThresholdEditor->Delete();
    this->WindowLevelThresholdEditor = NULL;
    }
  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerDiffusionWeightedVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingWidget = 1;
  
  this->Superclass::ProcessWidgetEvents(caller, event, callData);
  //
  // process volume selector events
  //
  vtkKWScaleWithEntry *dwiSelector = 
      vtkKWScaleWithEntry::SafeDownCast(caller);

  if (dwiSelector == this->DiffusionSelectorWidget && 
        event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    // get the volume display node
    vtkMRMLDiffusionWeightedVolumeDisplayNode *displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    if (displayNode != NULL)
      {
      displayNode->SetDiffusionComponent((int) dwiSelector->GetScale()->GetValue());
      vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();

      if (volumeNode != NULL && volumeNode->GetImageData() && this->WindowLevelThresholdEditor)
        {
        this->ExtractComponent->SetInput(volumeNode->GetImageData());
        this->ExtractComponent->SetComponents(displayNode->GetDiffusionComponent());
        this->ExtractComponent->Update();
        if (vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode) != NULL)
          {
          vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->CalculateScalarAutoLevels(displayNode, this->ExtractComponent->GetOutput());
          }
        else { vtkWarningMacro("Failed to calculate scalar levels, volume node is not a scalar"); }

//        volumeNode->CalculateAutoLevels();
        this->WindowLevelThresholdEditor->SetImageData(this->ExtractComponent->GetOutput());
        }
      }
    this->UpdatingWidget = 0;
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
      vtkMRMLVolumeDisplayNode *displayNode = this->GetVolumeDisplayNode();
      if (displayNode != NULL)
        {
        // set and observe it's colour node id
        if (displayNode->GetColorNodeID() == NULL)
          {
          // there's nothing yet, set it
          displayNode->SetAndObserveColorNodeID(color->GetID());
          }
        if (displayNode->GetColorNodeID() && strcmp(displayNode->GetColorNodeID(), color->GetID()) != 0)
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
    vtkMRMLDiffusionWeightedVolumeDisplayNode *displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());

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
          displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New ();
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
              displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
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
      if (displayNode->GetAutoWindowLevel() != this->WindowLevelThresholdEditor->GetAutoWindowLevel() ||
        this->WindowLevelThresholdEditor->GetAutoWindowLevel())
        {
        // Auto is turned on
        // this will cause window/level recompute in the display node
        displayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());

        //update sliders with recomputed values
        this->WindowLevelThresholdEditor->SetWindowLevel(displayNode->GetWindow(), displayNode->GetLevel());
        }

      int thresholdType = this->WindowLevelThresholdEditor->GetThresholdType();
      if (thresholdType == vtkKWWindowLevelThresholdEditor::ThresholdAuto && !displayNode->GetAutoThreshold())
        {
        // Auto is turned on
        // this will cause window/level recompute in the display node
        displayNode->SetAutoThreshold(1);

        //update sliders with recomputed values
        this->WindowLevelThresholdEditor->SetThreshold(displayNode->GetLowerThreshold(),
                                                       displayNode->GetUpperThreshold());
        }
      displayNode->DisableModifiedEventOn();
      displayNode->SetWindow(this->WindowLevelThresholdEditor->GetWindow());
      displayNode->SetLevel(this->WindowLevelThresholdEditor->GetLevel());
      displayNode->SetUpperThreshold(this->WindowLevelThresholdEditor->GetUpperThreshold());
      displayNode->SetLowerThreshold(this->WindowLevelThresholdEditor->GetLowerThreshold());
      displayNode->SetAutoWindowLevel(this->WindowLevelThresholdEditor->GetAutoWindowLevel());
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

    if (this->InterpolateButton == vtkKWCheckButton::SafeDownCast(caller) && 
        event == vtkKWCheckButton::SelectedStateChangedEvent)
      {
      vtkMRMLDiffusionWeightedVolumeDisplayNode *displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
      if (displayNode != NULL)
        {
        displayNode->SetInterpolate( this->InterpolateButton->GetSelectedState() );
        }
      this->UpdatingWidget = 0;      
      return;
      }

  if ( (editor == this->WindowLevelThresholdEditor && 
        event == vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent) )
    {
    vtkMRMLDiffusionWeightedVolumeDisplayNode *displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    if (displayNode != NULL)
      {
      this->MRMLScene->SaveStateForUndo(displayNode);
      }
    this->UpdatingWidget = 0;    
    return;
    }

}



//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
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
    this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
    this->DiffusionSelectorWidget->GetScale()->SetRange(0,volumeNode->GetImageData()->GetNumberOfScalarComponents()-1);
    vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    //--- check the interpolation which may have been modified from the SliceGUI
    if ( displayNode && this->InterpolateButton )
      {
      if (displayNode->GetInterpolate() != this->InterpolateButton->GetSelectedState() )
        {
        this->InterpolateButton->SetSelectedState( displayNode->GetInterpolate()  );
        }
      }
    //--- end check interpolation.

    }

  if (event == vtkCommand::ModifiedEvent || 
      (event == vtkMRMLScene::NodeAddedEvent && 
       (reinterpret_cast<vtkMRMLVolumeNode *>(callData) != NULL ||
        reinterpret_cast<vtkMRMLVolumeDisplayNode *>(callData) != NULL) ) )
    {
    this->UpdateWidgetFromMRML();
    }
  this->UpdatingMRML = 0;
  
}
//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::UpdateWidgetFromMRML ()
{ 
  vtkDebugMacro("UpdateWidgetFromMRML");
  
  vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
  vtkMRMLDiffusionWeightedVolumeDisplayNode *displayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
  
  if (volumeNode != NULL && displayNode != NULL && this->WindowLevelThresholdEditor)
    {
    
    this->ExtractComponent->SetInput(volumeNode->GetImageData());
    this->ExtractComponent->SetComponents(displayNode->GetDiffusionComponent());
    this->ExtractComponent->Update();
    this->WindowLevelThresholdEditor->SetImageData(this->ExtractComponent->GetOutput());
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

  // Adjust the range of the DWI slider selector
  if (this->GetVolumeNode())
    {
    this->DiffusionSelectorWidget->GetScale()->SetRange(0,this->GetVolumeNode()->GetImageData()->GetNumberOfScalarComponents()-1);
    }
  // Adjust widget from display Node
  if (displayNode != NULL  && this->WindowLevelThresholdEditor != NULL) 
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
    this->DiffusionSelectorWidget->GetScale()->SetValue(displayNode->GetDiffusionComponent());
    this->InterpolateButton->SetSelectedState( displayNode->GetInterpolate()  );
    }
  return;
}

void vtkSlicerDiffusionWeightedVolumeDisplayWidget::AddWidgetObservers ( )
{  
  this->Superclass::AddWidgetObservers();

  this->DiffusionSelectorWidget->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InterpolateButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{
  this->Superclass::RemoveWidgetObservers();

  this->DiffusionSelectorWidget->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InterpolateButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  //this->WindowLevelThresholdEditor->SetImageData(NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionWeightedVolumeDisplayWidget::CreateWidget ( )
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

    vtkKWScaleWithEntry *dwiSlider = vtkKWScaleWithEntry::New();
    this->DiffusionSelectorWidget = dwiSlider;
    dwiSlider->SetParent( volDisplayFrame );
    dwiSlider->Create();
    vtkMRMLDiffusionWeightedVolumeNode *volumeNode = vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(this->GetVolumeNode());
    if (volumeNode != NULL)
      {
      dwiSlider->GetScale()->SetRange(0,volumeNode->GetImageData()->GetNumberOfScalarComponents()-1);
      dwiSlider->GetScale()->SetResolution(1);
      }
    dwiSlider->SetLabelText("DWI Component");
    this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dwiSlider->GetWidgetName());


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
