#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUICollection.h"
#include "vtkCommandLineModuleGUI.h"
#include "vtkSlicerTractographyFiducialSeedingGUI.h"
#include "vtkTimerLog.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget.h"
#include "vtkSlicerTractographyDisplayGUI.h"
//MRML nodes
#include "vtkMRMLNode.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiducialListNode.h"
//logics
#include "vtkCommandLineModuleLogic.h"
#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkSlicerDiffusionEditorLogic.h"
//widgets
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithLabel.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerDWITestingWidget.h"
#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDWITestingWidget);
vtkCxxRevisionMacro (vtkSlicerDWITestingWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerDWITestingWidget::vtkSlicerDWITestingWidget(void)
  {
  this->Application = NULL;
  this->ActiveVolumeNode = NULL;
  //Testframe
  this->TestFrame = NULL;
  this->RunButton = NULL;
  this->FiducialSelector = NULL;
  this->DTISelector = NULL;
  this->ModifiedForNewTensor = 1;
  this->TensorNode = NULL;
  this->FiberNode = NULL;
  this->BaselineNode = NULL;
  this->MaskNode = NULL;
  this->GlyphFrame = NULL;
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i] = NULL;
    }
  this->ViewTracts = NULL;
  this->VolumesGUI = NULL;
  this->TractDisplayGUI = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerDWITestingWidget::~vtkSlicerDWITestingWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    vtkSetMRMLNodeMacro(this->ActiveVolumeNode, NULL);
    }
  if (this->Application)
    {
    this->Application->Delete();
    this->Application = NULL;
    }
  if (this->TestFrame)
    {
    this->TestFrame->SetParent (NULL);
    this->TestFrame->Delete();
    this->TestFrame = NULL;
    }
  if (this->FiducialSelector)
    {
    this->FiducialSelector->SetParent (NULL);
    this->FiducialSelector->Delete();
    this->FiducialSelector = NULL;
    }
  if (this->RunButton)
    {
    this->RunButton->SetParent (NULL);
    this->RunButton->Delete();
    this->RunButton = NULL;
    }
  if (this->DTISelector)
    {
    this->DTISelector->SetParent (NULL);
    this->DTISelector->Delete();
    this->DTISelector = NULL;
    }
  if (this->GlyphFrame)
    {
    this->GlyphFrame->SetParent (NULL);
    this->GlyphFrame->Delete();
    this->GlyphFrame = NULL;
    }
  if (this->TensorNode)
    {
    this->TensorNode->Delete();
    this->TensorNode = NULL;
    }
  if (this->FiberNode)
    {
    this->FiberNode->Delete();
    this->FiberNode = NULL;
    }
  if (this->BaselineNode)
    {
    this->BaselineNode->Delete();
    this->BaselineNode = NULL;
    }
  if (this->MaskNode)
    {
    this->MaskNode->Delete();
    this->MaskNode = NULL;
    }
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i]->SetParent (NULL);
    this->VisibilityButton[i]->Delete();
    this->VisibilityButton[i] = NULL;
    }
  if (this->ViewTracts)
    {
    this->ViewTracts->SetParent (NULL);
    this->ViewTracts->Delete();
    this->ViewTracts = NULL;
    }
  if (this->GlyphSpacingScale)
    {
    this->GlyphSpacingScale->SetParent (NULL);
    this->GlyphSpacingScale->Delete();
    this->GlyphSpacingScale = NULL;
    }
  this->ModifiedForNewTensor = 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::AddWidgetObservers ( )
  {    
  this->RunButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FiducialSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewTracts->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphSpacingScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i]->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::RemoveWidgetObservers( )
  {
  this->RunButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewTracts->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphSpacingScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i]->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerDWITestingWidget: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
  //run tensor test
  if (this->RunButton->GetWidget() == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    this->RunButton->SetEnabled(0);
    if(this->ModifiedForNewTensor || this->MRMLScene->GetNodesByName("DiffusionEditor_Tensor_Node")->GetNumberOfItems() == 0 )
      {
      // create a command line module node
      vtkMRMLCommandLineModuleNode *tensorCML = vtkMRMLCommandLineModuleNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLCommandLineModuleNode"));

      // set its name  
      tensorCML->SetModuleDescription("Diffusion Tensor Estimation");
      tensorCML->SetName("GradientEditor: Tensor Estimation");

      // set the parameters
      tensorCML->SetParameterAsString("estimationMethod", "Least Squares");
      tensorCML->SetParameterAsDouble("otsuOmegaThreshold",0.5);
      tensorCML->SetParameterAsBool("removeIslands", 0);
      tensorCML->SetParameterAsBool("applyMask", 0);
      tensorCML->SetParameterAsString("inputVolume", this->ActiveVolumeNode->GetID());

      // create the output nodes
      this->BaselineNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      this->BaselineNode->SetScene(this->GetMRMLScene());
      this->BaselineNode->SetName("DiffusionEditor_Baseline_Node");
      this->MRMLScene->AddNode(this->BaselineNode);

      this->MaskNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      this->MaskNode->SetScene(this->GetMRMLScene());
      this->MaskNode->SetName("DiffusionEditor_Threshold_Mask");
      this->MRMLScene->AddNode(this->MaskNode);

      if(this->TensorNode)
        {
        this->TensorNode->Delete(); //delete previous data to avoid leaks
        }
      this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLDiffusionTensorVolumeNode"));
      this->TensorNode->SetScene(this->GetMRMLScene());
      this->TensorNode->SetName("DiffusionEditor_Tensor_Node");
      this->MRMLScene->AddNode(this->TensorNode);

      this->TensorNode->SetBaselineNodeID(this->BaselineNode->GetID());
      this->TensorNode->SetMaskNodeID(this->MaskNode->GetID());

      // set output parameters
      tensorCML->SetParameterAsString("outputTensor", this->TensorNode->GetID());
      tensorCML->SetParameterAsString("outputBaseline", this->BaselineNode->GetID());
      tensorCML->SetParameterAsString("thresholdMask", this->MaskNode->GetID());

      //get the existing GUI of the "Diffusion Tensor Estimation Command Line Module" 
      vtkCommandLineModuleGUI *moduleGUI = vtkCommandLineModuleGUI::SafeDownCast(
        this->Application->GetModuleGUIByName("Diffusion Tensor Estimation"));
      moduleGUI->Enter();

      //set command line node to GUI an logic
      moduleGUI->SetCommandLineModuleNode( tensorCML);
      moduleGUI->GetLogic()->SetCommandLineModuleNode( tensorCML); //use the GUI's Logic to invoke the task

      //estimate tensors
      moduleGUI->GetLogic()->Apply( tensorCML);

      //clean up
      tensorCML->Delete();
      this->BaselineNode->Delete();
      this->MaskNode->Delete();
      this->ModifiedForNewTensor = 0;
      }
    this->RunButton->SetEnabled(1);
    }

  //dti selected (update tracts)
  else if (this->DTISelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && 
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) 
    {
    if(this->DTISelector->GetSelected() == NULL)
      {
      this->SetAllVisibilityButtons(0);
      return;
      }
    this->SetAllVisibilityButtons(1);
    //set old tensorNode
    if(this->TensorNode != NULL)
      {
      std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodes = this->TensorNode->GetSliceGlyphDisplayNodes();
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node;
      for (unsigned int i=0; i<glypDisplayNodes.size(); i++)
        {
        node = glypDisplayNodes[i];
        node->SetVisibility(0);
        }
      }
    //set new tensorNode
    this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->DTISelector->GetSelected());
    //create tracts and glyphs
    this->CreateTracts();
    this->CreateGlyphs();
    }

  //create tracts when fiducials list is selected 
  else if (this->FiducialSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->FiducialSelector->GetSelected() != NULL && this->DTISelector->GetSelected()) 
    {
    this->CreateTracts();
    }

  //view of glyphs
  else if((this->VisibilityButton[0] == vtkKWCheckButton::SafeDownCast(caller)|| this->VisibilityButton[1] == vtkKWCheckButton::SafeDownCast(caller) 
    || this->VisibilityButton[2] == vtkKWCheckButton::SafeDownCast(caller)) && this->TensorNode != NULL && 
    event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    this->CreateGlyphs();
    }

  //glyph spacing
  else if (vtkKWScale::SafeDownCast(caller) == this->GlyphSpacingScale->GetWidget() && 
    event == vtkKWScale::ScaleValueChangedEvent && this->TensorNode != NULL)
    {
    this->UpdateGlyphSpacing();
    }

  //view of tracts
  else if(this->ViewTracts == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent
     && this->TensorNode != NULL)
    {
    if(this->FiducialSelector->GetSelected() != NULL && this->ViewTracts->GetSelectedState())
      {
      //visibility of tracts on
      this->TractDisplayGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(1);
      this->CreateTracts(); //start tractography seeding
      }
    else
      {
      //visibility of tracts off
      this->TractDisplayGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(0);  
      }
    }
  }

void vtkSlicerDWITestingWidget::CreateGlyphs()
  {
  std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodes = this->TensorNode->GetSliceGlyphDisplayNodes();
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = NULL;

  for (unsigned int i=0; i<glypDisplayNodes.size(); i++)
    {
    node = glypDisplayNodes[i];
    //glyphs on/off
    node->SetVisibility(this->VisibilityButton[i]->GetSelectedState());
    if(this->VisibilityButton[i]->GetSelectedState())
      {
      //change current node to TensorNode in the main GUI
      //vtkErrorMacro("ID: "<<this->TensorNode->GetID());
      this->Application->GetApplicationGUI()->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID(this->TensorNode->GetID());
      this->Application->GetApplicationGUI()->GetApplicationLogic()->PropagateVolumeSelection();
      //adjust glyph spacing
      this->UpdateGlyphSpacing();
      }
    }

  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::UpdateGlyphSpacing()
  {
  if(this->TensorNode == NULL) return;

  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *displayNode = NULL;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *propertiesNode = NULL;

  //find the properties node
  std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodes = this->TensorNode->GetSliceGlyphDisplayNodes();
  displayNode = glypDisplayNodes[0];
  propertiesNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(displayNode->GetDTDisplayPropertiesNode());

  //update the spacing
  if(propertiesNode == NULL) return;
  propertiesNode->SetLineGlyphResolution((int)(this->GlyphSpacingScale->GetWidget()->GetValue()));
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetWidgetToDefault()
  {
  //switch off glyphs
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i]->SelectedStateOff();
    }
  //visibility of tracts off
  this->ViewTracts->SelectedStateOff();
  //dti selector to default
  this->DTISelector->GetWidget()->GetWidget()->SetValue("None");
  this->SetAllVisibilityButtons(0);
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetAllVisibilityButtons(int status)
  {
  for (int i=0; i<3; i++)
    {
    this->VisibilityButton[i]->SetEnabled(status);
    }
  this->ViewTracts->SetEnabled(status);
  this->GlyphSpacingScale->SetEnabled(status);
  this->GlyphSpacingScale->GetWidget()->SetValueVisibility(status);
  this->GlyphSpacingScale->GetWidget()->SetEnabled(status);
  this->FiducialSelector->SetEnabled(status);
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::CreateTracts()
  {
  if(this->TensorNode == NULL || !this->ViewTracts->GetSelectedState()) return;
  //wait untill ImageData is set in other thread
  if(this->TensorNode->GetImageData() == NULL)
    {
    this->Script ( "update idletasks" );
    this->Script ( "after 5 \"%s CreateTracts \"",  this->GetTclName() );
    }
  else
    {
    //get fiducial list
    vtkMRMLFiducialListNode* fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(
      this->FiducialSelector->GetSelected());

    if(fiducialListNode == NULL) return;

    //create new fiber node
    //also check if FiberNode is in the MRML scene (needed, when node is deleated in the data modul)
    if(this->FiberNode == NULL || this->MRMLScene->GetNodesByName("GradientenEditor_Fiber_Node")->GetNumberOfItems() == 0)
      {
      if(this->FiberNode != NULL) this->FiberNode->Delete();
      this->FiberNode = vtkMRMLFiberBundleNode::New();
      this->FiberNode->SetScene(this->GetMRMLScene());
      this->FiberNode->SetName("GradientenEditor_Fiber_Node");
      this->MRMLScene->AddNode(this->FiberNode);
      }

    //get the existing GUI of the "Tractography Fiducial Seeding Module"
    vtkSlicerTractographyFiducialSeedingGUI *moduleGUI = vtkSlicerTractographyFiducialSeedingGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("FiducialSeeding"));    
    moduleGUI->Enter(); 

    //set the selectors to my nodes
    moduleGUI->SetVolumeSelector(this->TensorNode);
    moduleGUI->SetFiducialSelector(fiducialListNode);
    moduleGUI->SetOutFiberSelector(this->FiberNode);

    //create tracts
    moduleGUI->CreateTracts();
    this->Application->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Done");
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  if (dwiNode == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << ": dwiNode in UpdateWidget() is NULL");
    return;
    }
  this->RunButton->EnabledOn(); //activate tensor estimation button
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode); //set ActiveVolumeNode
  if (dwiNode->IsA("vtkMRMLDiffusionTensorVolumeNode"))
    {
    this->RunButton->EnabledOff(); //deactivate tensor estimation button
    }
  if(!this->VolumesGUI)
    {
    //get the existing GUI of the "Volumes Module"
    this->VolumesGUI = vtkSlicerVolumesGUI::SafeDownCast(this->Application->GetModuleGUIByName("Volumes"));
    }
  if(!this->TractDisplayGUI)
    {
    //get the existing GUI of the "Tractography Display Module"
    this->TractDisplayGUI = vtkSlicerTractographyDisplayGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("DisplayLoadSave"));
    this->TractDisplayGUI->Enter();
    }
  this->SetWidgetToDefault();
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  //create test frame 
  this->TestFrame = vtkKWFrameWithLabel::New();
  this->TestFrame->SetParent(this->GetParent());
  this->TestFrame->Create();
  this->TestFrame->CollapseFrame();
  this->TestFrame->SetLabelText("Testing (Tensor Estimation & Glyphs & Tractography Seeding)");
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 4", 
    this->TestFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButtonWithLabel::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetLabelText("Estimate new tensor: ");
  this->RunButton->GetWidget()->SetText("Run");
  this->RunButton->GetWidget()->SetWidth(10);
  this->RunButton->SetBalloonHelpString("Compute new tensors.");
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 4 ", 
    this->RunButton->GetWidgetName()); 

  //create dti selector
  this->DTISelector = vtkSlicerNodeSelectorWidget::New();
  this->DTISelector->SetNodeClass("vtkMRMLDiffusionTensorVolumeNode", NULL, NULL, NULL);
  this->DTISelector->SetParent(this->TestFrame->GetFrame());
  this->DTISelector->Create();
  this->DTISelector->SetMRMLScene(this->GetMRMLScene());
  this->DTISelector->NoneEnabledOn();
  this->DTISelector->SetNewNodeEnabled(0);
  this->DTISelector->UpdateMenu();
  this->DTISelector->SetLabelText("Display a DTI node: ");
  this->DTISelector->SetBalloonHelpString("Select a DTI volume from the current mrml scene and see its tracts or glyphs.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 4 ", 
    this->DTISelector->GetWidgetName());

  //create frame for glyphs 
  this->GlyphFrame = vtkKWFrame::New();
  this->GlyphFrame->SetParent(this->TestFrame->GetFrame());
  this->GlyphFrame->Create();
  this->Script("pack %s -side top -anchor nw -pady 2", 
    this->GlyphFrame->GetWidgetName());


  //create glyph visibility buttons
  char* glyphNames[] = {"View red glyphs", "View yellow glyphs", "View green glyphs"};
  for (int i=0; i<3; i++)
    {   
    this->VisibilityButton[i] = vtkKWCheckButton::New();
    this->VisibilityButton[i]->SetParent(this->GlyphFrame);
    this->VisibilityButton[i]->Create();
    this->VisibilityButton[i]->SetText(glyphNames[i]);
    this->VisibilityButton[i]->SetBalloonHelpString("Set visibility of glyphs.");
    this->Script("pack %s -side left -anchor nw -padx 2 ",
      this->VisibilityButton[i]->GetWidgetName());
    }

  //create resolution scale
  this->GlyphSpacingScale = vtkKWScaleWithLabel::New();
  this->GlyphSpacingScale->SetParent(this->TestFrame->GetFrame());
  this->GlyphSpacingScale->Create();
  this->GlyphSpacingScale->SetLabelText("Spacing of glyphs: ");
  this->GlyphSpacingScale->SetLabelPositionToLeft();
  this->GlyphSpacingScale->GetWidget()->SetRange(1,50);
  this->GlyphSpacingScale->GetWidget()->SetResolution(1);
  this->GlyphSpacingScale->GetWidget()->SetValue(20);
  this->GlyphSpacingScale->SetBalloonHelpString("Skip step for glyphs.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 6",
    this->GlyphSpacingScale->GetWidgetName());

  //create view tracts
  this->ViewTracts = vtkKWCheckButton::New();
  this->ViewTracts->SetParent(this->TestFrame->GetFrame());
  this->ViewTracts->Create();
  this->ViewTracts->SetText("View tracts based on fiducial list");
  this->ViewTracts->SetBalloonHelpString("Set visibility of tracts.");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->ViewTracts->GetWidgetName());

  //create fiducial list
  this->FiducialSelector = vtkSlicerNodeSelectorWidget::New();
  this->FiducialSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
  this->FiducialSelector->SetNewNodeEnabled(0);
  this->FiducialSelector->NoneEnabledOn();
  this->FiducialSelector->SetShowHidden(1);
  this->FiducialSelector->SetParent(this->TestFrame->GetFrame());
  this->FiducialSelector->SetMRMLScene(this->GetMRMLScene());
  this->FiducialSelector->Create();  
  this->FiducialSelector->UpdateMenu();
  this->FiducialSelector->SetWidth(25);
  this->FiducialSelector->SetBalloonHelpString("Set fiducial list for tractography seeding.");

  this->Script("pack %s %s -side left -anchor ne -padx 2 -pady 2", 
    this->ViewTracts->GetWidgetName(),
    this->FiducialSelector->GetWidgetName());
  } 

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetModifiedForNewTensor(int modified)
  {
  this->ModifiedForNewTensor = modified;
  }
