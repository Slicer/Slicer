#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerDiffusionTensorVolumeDisplayWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"

#include "vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

#include "vtkDiffusionTensorMathematics.h"

#include "vtkAssignAttribute.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerVolumesLogic.h"

#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDiffusionTensorVolumeDisplayWidget );
vtkCxxRevisionMacro ( vtkSlicerDiffusionTensorVolumeDisplayWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorVolumeDisplayWidget::vtkSlicerDiffusionTensorVolumeDisplayWidget ( )
{
    this->DTIMathematics = vtkDiffusionTensorMathematics::New();
    this->ExtractComponent = vtkImageExtractComponents::New();
    
    this->AssignAttributeTensorsFromScalars= vtkAssignAttribute::New();
    this->AssignAttributeScalarsFromTensors= vtkAssignAttribute::New();
    this->AssignAttributeTensorsFromScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
    this->AssignAttributeScalarsFromTensors->Assign(vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::TENSORS, vtkAssignAttribute::POINT_DATA);

    this->ScalarModeMenu = NULL;
    this->ScalarOptionsFrame=NULL;
    this->InterpolateButton = NULL;
    this->ColorSelectorWidget = NULL;
    this->WindowLevelThresholdEditor = NULL;
    
    this->GlyphDisplayWidget = NULL;
    
    this->UpdatingMRML = 0;
    this->UpdatingWidget = 0;

}


//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorVolumeDisplayWidget::~vtkSlicerDiffusionTensorVolumeDisplayWidget ( )
{
  this->DTIMathematics->Delete();
  this->ExtractComponent->Delete();
  this->AssignAttributeTensorsFromScalars->Delete();
  this->AssignAttributeScalarsFromTensors->Delete();

  if (this->IsCreated())
    {
    this->RemoveWidgetObservers();
    }
 
  if (this->ScalarModeMenu)
    {
    this->ScalarModeMenu->SetParent(NULL);
    this->ScalarModeMenu->Delete();
    this->ScalarModeMenu = NULL;
    }
  if (this->ScalarOptionsFrame)
    {
    this->ScalarOptionsFrame->SetParent(NULL);
    this->ScalarOptionsFrame->Delete();
    this->ScalarOptionsFrame = NULL;
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
  if (this->GlyphDisplayWidget)
    {
    this->GlyphDisplayWidget->SetParent(NULL);
    this->GlyphDisplayWidget->Delete();
    this->GlyphDisplayWidget = NULL;
    }
  this->SetMRMLScene ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerDiffusionTensorVolumeDisplayWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  if (this->UpdatingMRML || this->UpdatingWidget)
    {
    return;
    }

  this->UpdatingWidget = 1;

  this->Superclass::ProcessWidgetEvents(caller, event, callData);

  //
  // process scalar mode menu events
  //
  vtkKWMenu *scalarMenu = 
      vtkKWMenu::SafeDownCast(caller);

  if (scalarMenu == this->ScalarModeMenu->GetWidget()->GetWidget()->GetMenu() && 
        event == vtkKWMenu::MenuItemInvokedEvent)
    {
    vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
    vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    if (volumeNode != NULL && displayNode != NULL)
      {
      const char *scalarSelection = this->ScalarModeMenu->GetWidget()->GetWidget()->GetValue();
      if (displayNode->GetDiffusionTensorDisplayPropertiesNode())
        {
        displayNode->GetDiffusionTensorDisplayPropertiesNode()->SetScalarInvariant(this->ScalarModeMap[std::string(scalarSelection)]);
        this->DTIMathematics->SetInput(volumeNode->GetImageData());
        this->DTIMathematics->SetOperation(displayNode->GetDiffusionTensorDisplayPropertiesNode()->
                                            GetScalarInvariant());
        this->DTIMathematics->Update();
        
        vtkImageData* image = this->DTIMathematics->GetOutput();
        vtkSlicerVolumesLogic::CalculateScalarAutoLevels(image, displayNode);
        this->WindowLevelThresholdEditor->SetImageData(image);
        }
      }
    this->UpdatingWidget = 0;
    return;
    }

 // Widgets that apply to Scalar Invariant Display properties

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
    vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());

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
          displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New ();
          displayNode->SetScene(this->MRMLScene);
          this->MRMLScene->AddNode (displayNode);
          displayNode->Delete();
          vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
          propNode->SetScene(this->MRMLScene);
          this->MRMLScene->AddNode (propNode);
          displayNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(propNode->GetID());
          propNode->Delete();
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
     this->UpdatingWidget = 0;
     return;
      }
    }

    if (this->InterpolateButton == vtkKWCheckButton::SafeDownCast(caller) && 
        event == vtkKWCheckButton::SelectedStateChangedEvent)
      {
      vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
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
    vtkMRMLNode *displayNode = this->GetVolumeDisplayNode();
    if (displayNode != NULL)
      {
      this->MRMLScene->SaveStateForUndo(displayNode);
      }
    this->UpdatingWidget = 0;
    return;
    }

}



//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::ProcessMRMLEvents ( vtkObject *caller,
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

  vtkMRMLDiffusionTensorVolumeNode *volumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(caller);
  if (volumeNode == curVolumeNode && 
      volumeNode != NULL && event == vtkCommand::ModifiedEvent)
    {
    this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
    this->GlyphDisplayWidget->SetDiffusionTensorVolumeNode(volumeNode);
    //--- check the interpolation which may have been modified from the SliceGUI
    vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
    if (displayNode && this->InterpolateButton )
      {
      if (displayNode->GetInterpolate() != this->InterpolateButton->GetSelectedState() )
        {
        this->InterpolateButton->SetSelectedState( displayNode->GetInterpolate()  );
        }
      }
    //--- end check interpolation

    }

  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateWidgetFromMRML();
    this->UpdatingMRML = 0;
    return;
    }
}
//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::UpdateWidgetFromMRML ()
{ 
  vtkDebugMacro("UpdateWidgetFromMRML");
  vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());

  if(displayNode == NULL) return;

  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(displayNode->GetDiffusionTensorDisplayPropertiesNode());
  
  if ( this->ScalarModeMenu )
    {
    if (propNode != NULL)
      {
      this->ScalarModeMenu->GetWidget()->GetWidget()->SetValue(propNode->GetScalarInvariantAsString());
      }
    }
    

  vtkMRMLDiffusionTensorVolumeNode *volumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->GetVolumeNode());
  
  if (volumeNode != NULL && this->GlyphDisplayWidget)
    {
    this->GlyphDisplayWidget->SetDiffusionTensorVolumeNode(volumeNode);
    }
  if (volumeNode != NULL && displayNode != NULL && this->WindowLevelThresholdEditor)
    {
    if (displayNode->GetDiffusionTensorDisplayPropertiesNode())
      {
      this->DTIMathematics->SetInput(volumeNode->GetImageData());
      this->DTIMathematics->SetOperation(displayNode->GetDiffusionTensorDisplayPropertiesNode()->
                                   GetScalarInvariant());
      this->DTIMathematics->Update();
      
      vtkImageData* image = this->DTIMathematics->GetOutput();
      this->WindowLevelThresholdEditor->SetImageData(image);
      }
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
  
  if (displayNode != NULL) 
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

void vtkSlicerDiffusionTensorVolumeDisplayWidget::AddWidgetObservers ( )
{  
  this->Superclass::AddWidgetObservers();

  this->ScalarModeMenu->GetWidget()->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->AddObserver(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InterpolateButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::RemoveWidgetObservers ( ) 
{
  this->Superclass::RemoveWidgetObservers();

  this->ScalarModeMenu->GetWidget()->GetWidget()->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->WindowLevelThresholdEditor->RemoveObservers(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->InterpolateButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->WindowLevelThresholdEditor->SetImageData(NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorVolumeDisplayWidget::CreateWidget ( )
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

  vtkKWMenuButtonWithSpinButtonsWithLabel *scalarMenuButton = vtkKWMenuButtonWithSpinButtonsWithLabel::New();
  this->ScalarModeMenu = scalarMenuButton;
  scalarMenuButton->SetParent( volDisplayFrame );
  scalarMenuButton->Create();
  scalarMenuButton->SetLabelText("Scalar Mode");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 scalarMenuButton->GetWidgetName());
  // --
  // SCALAR OPTIONS FRAME  
  vtkSlicerModuleCollapsibleFrame *scalarFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->ScalarOptionsFrame = scalarFrame;
  scalarFrame->SetParent ( volDisplayFrame );
  scalarFrame->Create ( );
  scalarFrame->SetLabelText ("Scalar Vis Options");
  scalarFrame->ExpandFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    scalarFrame->GetWidgetName(), volDisplayFrame->GetWidgetName());

   // a selector to change the color node associated with this display
  this->ColorSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ColorSelectorWidget->SetParent ( scalarFrame->GetFrame() );
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
  this->InterpolateButton->SetParent(scalarFrame->GetFrame());
  this->InterpolateButton->Create();
  this->InterpolateButton->SelectedStateOn();
  this->InterpolateButton->SetText("Interpolate");
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
  this->InterpolateButton->GetWidgetName());

  vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode());
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propNode = NULL;

  if (displayNode == NULL)
    {
    //Create dummy display Node to set init variables
    displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
    }

  if (displayNode != NULL)
    {
    propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(displayNode->GetDiffusionTensorDisplayPropertiesNode());
    if (propNode == NULL)
      {
      //Create dummy display Node to set
      propNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
      }

    if (propNode != NULL)
      {
      //Fill up menu buttons and map variables
      int initIdx = propNode->GetFirstScalarInvariant();
      int endIdx = propNode->GetLastScalarInvariant();
      int currentVal = propNode->GetScalarInvariant();
      this->ScalarModeMap.clear();

      for (int k=initIdx ; k<=endIdx ; k++)
        {
        propNode->SetScalarInvariant(k);
        const char* tag = propNode->GetScalarInvariantAsString();
        this->ScalarModeMap[std::string(tag)]=k;
        scalarMenuButton->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(tag);
        }
      // Restore inital scalar Invariant value
      propNode->SetScalarInvariant(currentVal);
      scalarMenuButton->GetWidget()->GetWidget()->SetValue(propNode->GetScalarInvariantAsString());

      initIdx = propNode->GetFirstGlyphGeometry();
      endIdx = propNode->GetLastGlyphGeometry();
      currentVal = propNode->GetGlyphGeometry();
      this->GlyphModeMap.clear();
      for (int k=initIdx ; k<=endIdx ; k++)
        {
        propNode->SetGlyphGeometry(k);
        const char *tag = propNode->GetGlyphGeometryAsString();
        this->GlyphModeMap[std::string(tag)]=k;
        }
      //Restore inital scalar Invariant value
      propNode->SetScalarInvariant(currentVal);
      }
    //Set glyph button
    }

    this->GlyphDisplayWidget = vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget::New();
    this->GlyphDisplayWidget->SetParent( volDisplayFrame );
    this->GlyphDisplayWidget->SetMRMLScene(this->MRMLScene);
    this->GlyphDisplayWidget->Create();
    vtkMRMLDiffusionTensorVolumeNode *dtvolumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->GetVolumeNode());
    if (dtvolumeNode != NULL)
      {
      this->GlyphDisplayWidget->SetDiffusionTensorVolumeNode(dtvolumeNode);
      }
    this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->GlyphDisplayWidget->GetWidgetName());

    this->WindowLevelThresholdEditor = vtkKWWindowLevelThresholdEditor::New();
    this->WindowLevelThresholdEditor->SetParent ( scalarFrame->GetFrame());
    this->WindowLevelThresholdEditor->Create ( );
    vtkMRMLVolumeNode *volumeNode = this->GetVolumeNode();
    if (volumeNode != NULL)
      {
      this->WindowLevelThresholdEditor->SetImageData(volumeNode->GetImageData());
      }
    this->Script ( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->WindowLevelThresholdEditor->GetWidgetName());


    this->AddWidgetObservers();
    if (this->MRMLScene != NULL)
      {
      this->SetAndObserveMRMLScene(this->MRMLScene);
      }

    //volDisplayFrame->Delete();



    //Delete dummy display nodes
    if (displayNode != NULL)
      {
       if (propNode != displayNode->GetDiffusionTensorDisplayPropertiesNode() && propNode != NULL)
        {
        propNode->Delete();
        }
       if (displayNode != vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetVolumeDisplayNode()) )
        {
        displayNode->Delete();
        }
      }
}
