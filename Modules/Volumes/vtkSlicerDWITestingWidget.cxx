#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerGUICollection.h"
#include "vtkCommandLineModuleGUI.h"
#include "vtkSlicerTractographyFiducialSeedingGUI.h"
#include "vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget.h"
#include "vtkSlicerTractographyDisplayGUI.h"
#include "vtkSlicerVisibilityIcons.h"
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
#include "vtkKWScale.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"

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
  this->TensorNode = NULL;
  this->FiberNode = NULL;
  this->TractVisibilityButton = NULL;
  this->GlyphFrame = NULL;
  this->GlyphVisibilityLabel = NULL;
  this->TractVisibilityLabel = NULL;
  for (int i=0; i<3; i++)
    {
    this->GlyphVisibility[i] = 0;
    this->GlyphVisibilityButton[i] = NULL;
    }
  this->ModifiedForNewTensor = 1;
  this->TractVisibility = 0;
  this->NumberOfTensorEstimations = 0;
  this->VisibilityIcons = NULL;
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
  if (this->FiberNode)
    {
    this->FiberNode->Delete();
    this->FiberNode = NULL;
    }
  for (int i=0; i<3; i++)
    {
    this->GlyphVisibilityButton[i]->SetParent (NULL);
    this->GlyphVisibilityButton[i]->Delete();
    this->GlyphVisibilityButton[i] = NULL;
    this->GlyphVisibility[i] = 0;
    }
  if (this->TractVisibilityButton)
    {
    this->TractVisibilityButton->SetParent (NULL);
    this->TractVisibilityButton->Delete();
    this->TractVisibilityButton = NULL;
    }
  if (this->GlyphSpacingScale)
    {
    this->GlyphSpacingScale->SetParent (NULL);
    this->GlyphSpacingScale->Delete();
    this->GlyphSpacingScale = NULL;
    }
  if (this->GlyphVisibilityLabel)
    {
    this->GlyphVisibilityLabel->SetParent (NULL);
    this->GlyphVisibilityLabel->Delete();
    this->GlyphVisibilityLabel = NULL;
    }
  if (this->TractVisibilityLabel)
    {
    this->TractVisibilityLabel->SetParent (NULL);
    this->TractVisibilityLabel->Delete();
    this->TractVisibilityLabel = NULL;
    }
  if (this->VisibilityIcons ) 
    {
    this->VisibilityIcons->Delete();
    this->VisibilityIcons = NULL;
    }
  this->ModifiedForNewTensor = 0;
  this->NumberOfTensorEstimations = 0;
  this->TractVisibility = 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::AddWidgetObservers ( )
  {
  this->RunButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FiducialSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TractVisibilityButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphSpacingScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for (int i=0; i<3; i++)
    {
    this->GlyphVisibilityButton[i]->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::RemoveWidgetObservers( )
  {
  this->RunButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TractVisibilityButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphSpacingScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  for (int i=0; i<3; i++)
    {
    this->GlyphVisibilityButton[i]->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
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
  // possible caller (widgets)
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);

  //---------
  //run tensor test
  if (this->RunButton->GetWidget() == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent)
    {
    this->RunButton->SetEnabled(0);
    if(this->ModifiedForNewTensor)
      {
      // create a command line module node
      vtkMRMLCommandLineModuleNode *tensorCLM = vtkMRMLCommandLineModuleNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLCommandLineModuleNode"));

      // set its name  
      tensorCLM->SetModuleDescription("Diffusion Tensor Estimation");
      tensorCLM->SetName("GradientEditor: Tensor Estimation");

      // set the parameters
      tensorCLM->SetParameterAsString("estimationMethod", "Least Squares");
      tensorCLM->SetParameterAsDouble("otsuOmegaThreshold",0.5);
      tensorCLM->SetParameterAsBool("removeIslands", 0);
      tensorCLM->SetParameterAsBool("applyMask", 0);
      tensorCLM->SetParameterAsString("inputVolume", this->ActiveVolumeNode->GetID());

      this->NumberOfTensorEstimations++;
      std::stringstream nodeName1;
      nodeName1 << "DiffusionEditor_" << this->NumberOfTensorEstimations <<"._" << "BaselineNode";

      // create the output nodes
      vtkMRMLScalarVolumeNode *baselineNodeCLM = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      baselineNodeCLM->SetScene(this->GetMRMLScene());
      baselineNodeCLM->SetName(nodeName1.str().c_str());
      this->MRMLScene->AddNode(baselineNodeCLM);

      std::stringstream nodeName2;
      nodeName2 << "DiffusionEditor_" << this->NumberOfTensorEstimations <<"._" << "ThresholdMask";

      vtkMRMLScalarVolumeNode *maskNodeCLM = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      maskNodeCLM->SetScene(this->GetMRMLScene());
      maskNodeCLM->SetName(nodeName2.str().c_str());
      this->MRMLScene->AddNode(maskNodeCLM);

      std::stringstream nodeName3;
      nodeName3 << "DiffusionEditor_" << this->NumberOfTensorEstimations <<"._" << "TensorNode";

      vtkMRMLDiffusionTensorVolumeNode *tensorNodeCLM = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLDiffusionTensorVolumeNode"));
      tensorNodeCLM->SetScene(this->GetMRMLScene());
      tensorNodeCLM->SetName(nodeName3.str().c_str());
      this->MRMLScene->AddNode(tensorNodeCLM);

      tensorNodeCLM->SetBaselineNodeID(baselineNodeCLM->GetID());
      tensorNodeCLM->SetMaskNodeID(maskNodeCLM->GetID());

      // set output parameters
      tensorCLM->SetParameterAsString("outputTensor", tensorNodeCLM->GetID());
      tensorCLM->SetParameterAsString("outputBaseline", baselineNodeCLM->GetID());
      tensorCLM->SetParameterAsString("thresholdMask", maskNodeCLM->GetID());

      //get the existing GUI of the "Diffusion Tensor Estimation Command Line Module" 
      vtkCommandLineModuleGUI *moduleGUI = vtkCommandLineModuleGUI::SafeDownCast(
        this->Application->GetModuleGUIByName("Diffusion Tensor Estimation"));
      moduleGUI->Enter();

      //set command line node to GUI an logic
      moduleGUI->SetCommandLineModuleNode(tensorCLM);
      moduleGUI->GetLogic()->SetCommandLineModuleNode(tensorCLM); //use the GUI's Logic to invoke the task

      //estimate tensors
      moduleGUI->GetLogic()->Apply(tensorCLM);

      //clean up
      tensorCLM->Delete();
      tensorNodeCLM->Delete();
      baselineNodeCLM->Delete();
      maskNodeCLM->Delete();
      this->ModifiedForNewTensor = 0;
      }
    this->RunButton->SetEnabled(1);
    }

  //---------
  //dti selected (update tracts)
  else if (selector == this->DTISelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent) 
    {
    vtkMRMLDiffusionTensorVolumeNode *selected = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->DTISelector->GetSelected());

    if(selected == NULL)
      {
      this->SetAllVisibilityButtons(0);
      return;
      }

    std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodes = selected->GetSliceGlyphDisplayNodes();
    if(glypDisplayNodes.size() != 3) 
      {
      vtkKWMessageDialog *MessageDialog = vtkKWMessageDialog::New();
      MessageDialog->SetParent(this->Application->GetApplicationGUI()->GetMainSlicerWindow());
      MessageDialog->SetDisplayPositionToMasterWindowCenter();
      MessageDialog->Create();
      MessageDialog->SetText("Tensor Estimation not completed!");
      MessageDialog->Invoke();
      MessageDialog->Delete();
      this->SetWidgetToDefault();
      return;
      }

    //set old tensorNode
    if(this->TensorNode != NULL)
      {
      std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodesOld = this->TensorNode->GetSliceGlyphDisplayNodes();
      for (unsigned int i=0; i<glypDisplayNodesOld.size(); i++)
        {
        glypDisplayNodesOld[i]->SetVisibility(0);
        }
      }
    //set new tensorNode
    this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(selected);
    //create tracts and glyphs
    this->CreateTracts();
    this->CreateGlyphs();
    this->SetAllVisibilityButtons(1);
    }

  //---------
  //all other events need a tensorNode
  if(this->TensorNode == NULL) return;

  //---------
  //fiducial list (update tracts)
  else if (selector == this->FiducialSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->FiducialSelector->GetSelected() != NULL) 
    {
    this->CreateTracts();
    }

  //---------
  //create glyphs
  else if((button == this->GlyphVisibilityButton[0]->GetWidget() || 
    button == this->GlyphVisibilityButton[1]->GetWidget() || 
    button == this->GlyphVisibilityButton[2]->GetWidget()) 
    && event == vtkKWPushButton::InvokedEvent )
    {
    //who is the caller?
    int i = -1;
    for (i=0; i<3; i++)
      {
      if (button == this->GlyphVisibilityButton[i]->GetWidget()) break;
      }

    if (i == -1) return;

    if(!this->GlyphVisibility[i])
      {
      this->SetGlyphVisibility(i,1); //glyphs on in plane i
      }
    else
      {
      this->SetGlyphVisibility(i,0); //no glyphs
      }
    this->CreateGlyphs();
    }

  //---------
  //glyph spacing
  else if (scale == this->GlyphSpacingScale->GetWidget() && event == vtkKWScale::ScaleValueChangedEvent)
    {
    this->UpdateGlyphSpacing();
    }

  //---------
  //view of tracts
  else if(button == this->TractVisibilityButton->GetWidget() && event == vtkKWPushButton::InvokedEvent)
    {
    if(this->FiducialSelector->GetSelected() != NULL && !this->TractVisibility)
      {
      this->SetTractVisibility(1); //create tracts and show them
      }
    else
      {
      this->SetTractVisibility(0); //no tracts
      }
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::CreateGlyphs()
  {
  std::vector<vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> glypDisplayNodes = this->TensorNode->GetSliceGlyphDisplayNodes();
  if(glypDisplayNodes.size() != 3) return;
  for (unsigned int i=0; i<3; i++)
    {
    if(this->GlyphVisibility[i])
      {
      //change current node to TensorNode in the main GUI
      char* currentID = this->Application->GetApplicationGUI()->GetApplicationLogic()->GetSelectionNode()->GetActiveVolumeID();
      if(currentID != this->TensorNode->GetID())
        {
        this->Application->GetApplicationGUI()->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID(this->TensorNode->GetID());
        this->Application->GetApplicationGUI()->GetApplicationLogic()->PropagateVolumeSelection();
        }
      glypDisplayNodes[i]->SetVisibility(1); //glyphs on for plane i
      this->UpdateGlyphSpacing(); //adjust glyph spacing
      }
    else
      {
      glypDisplayNodes[i]->SetVisibility(0); //glyphs off
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
void vtkSlicerDWITestingWidget::CreateTracts()
  {
  if(this->TensorNode == NULL || this->TensorNode->GetImageData() == NULL 
    || this->FiducialSelector->GetSelected() == NULL || !this->TractVisibility) return;

  //get fiducial list
  vtkMRMLFiducialListNode* fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(
    this->FiducialSelector->GetSelected());

  //get the existing GUI of the "Tractography Fiducial Seeding Module"
  vtkSlicerTractographyFiducialSeedingGUI *moduleGUI = vtkSlicerTractographyFiducialSeedingGUI::SafeDownCast(
    this->Application->GetModuleGUIByName("FiducialSeeding"));    
  moduleGUI->Enter();

  //create new fiber node
  //also check if FiberNode is in the MRML scene (needed, when node is deleated in the data modul)
  if(this->FiberNode == NULL || !this->MRMLScene->GetNodeByID(this->FiberNode->GetID()))
    {
    vtkMRMLFiberBundleNode *fiberNodeTract = vtkMRMLFiberBundleNode::New();
    fiberNodeTract->SetScene(this->GetMRMLScene());
    fiberNodeTract->SetName("GradientenEditor_Fiber_Node");
    this->MRMLScene->AddNode(fiberNodeTract);
    vtkSetMRMLNodeMacro(this->FiberNode, fiberNodeTract);
    fiberNodeTract->Delete();
    }

  //set the selectors to my nodes
  moduleGUI->SetVolumeSelector(this->TensorNode);
  moduleGUI->SetFiducialSelector(fiducialListNode);
  moduleGUI->SetOutFiberSelector(this->FiberNode);

  //create tracts
  moduleGUI->CreateTracts();
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode)
  {
  if (dwiNode == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << ": dwiNode in UpdateWidget() is NULL");
    return;
    }
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode); //set ActiveVolumeNode

  //deactivate tensor estimation button
  if (dwiNode->IsA("vtkMRMLDiffusionTensorVolumeNode")) this->RunButton->EnabledOff(); 
  else this->RunButton->EnabledOn(); //activate tensor estimation button

  //set widegt to default status
  this->SetWidgetToDefault();
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetModifiedForNewTensor(int modified)
  {
  this->ModifiedForNewTensor = modified;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetGlyphVisibility(int plane, int status)
  {
  if(status)
    {
    this->GlyphVisibilityButton[plane]->GetWidget()->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());
    this->GlyphVisibility[plane] = 1;
    }
  else
    {
    this->GlyphVisibilityButton[plane]->GetWidget()->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
    this->GlyphVisibility[plane] = 0;
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetTractVisibility(int status)
  {
  if(status)
    {
    this->TractVisibilityButton->GetWidget()->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());
    this->TractVisibility = 1;
    this->CreateTracts(); //start tractography seeding
    }
  else
    {
    this->TractVisibilityButton->GetWidget()->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
    this->TractVisibility = 0;
    }
  if(this->FiberNode != NULL)
    this->FiberNode->GetTubeDisplayNode()->SetVisibility(status); //visibility of tracts
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetWidgetToDefault()
  {
  //switch off glyphs
  for (int i=0; i<3; i++)
    {
    this->SetGlyphVisibility(i,0);
    }
  //visibility of tracts off
  this->SetTractVisibility(0);
  //dti selector to default
  this->DTISelector->GetWidget()->GetWidget()->SetValue("None");
  this->SetAllVisibilityButtons(0);
  this->ModifiedForNewTensor = 1;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::SetAllVisibilityButtons(int status)
  {
  for (int i=0; i<3; i++)
    {
    this->GlyphVisibilityButton[i]->SetEnabled(status);
    }
  this->TractVisibilityButton->SetEnabled(status);
  this->GlyphSpacingScale->SetEnabled(status);
  this->GlyphSpacingScale->GetWidget()->SetValueVisibility(status);
  this->GlyphSpacingScale->GetWidget()->SetEnabled(status);
  this->FiducialSelector->SetEnabled(status);
  this->TractVisibilityLabel->SetEnabled(status);
  this->GlyphVisibilityLabel->SetEnabled(status);
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

  //create visibility icons
  this->VisibilityIcons = vtkSlicerVisibilityIcons::New();

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
  this->RunButton->SetLabelText("Estimate New Tensor: ");
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
  this->DTISelector->SetLabelText("Display a DTI Node: ");
  this->DTISelector->SetBalloonHelpString("Select a DTI volume from the current mrml scene and see its tracts or glyphs.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 4 ", 
    this->DTISelector->GetWidgetName());

  //create frame for glyphs 
  this->GlyphFrame = vtkKWFrame::New();
  this->GlyphFrame->SetParent(this->TestFrame->GetFrame());
  this->GlyphFrame->Create();
  this->Script("pack %s -side top -anchor nw -pady 2", 
    this->GlyphFrame->GetWidgetName());

  //create glyph visibility label
  this->GlyphVisibilityLabel = vtkKWLabel::New();
  this->GlyphVisibilityLabel->SetParent(this->GlyphFrame);
  this->GlyphVisibilityLabel->Create();
  this->GlyphVisibilityLabel->SetText("Glyph Visibility: ");
  this->Script("pack %s -side left -anchor w -padx 2 ",
    this->GlyphVisibilityLabel->GetWidgetName());

  //create glyph visibility buttons
  char* glyphNames[] = {"Red", "Yellow", "Green"};
  for (int i=0; i<3; i++)
    {   
    this->GlyphVisibilityButton[i] = vtkKWPushButtonWithLabel::New();
    this->GlyphVisibilityButton[i]->SetParent(this->GlyphFrame);
    this->GlyphVisibilityButton[i]->Create();
    this->GlyphVisibilityButton[i]->GetWidget()->SetReliefToFlat();
    this->GlyphVisibilityButton[i]->GetWidget()->SetOverReliefToNone();
    this->GlyphVisibilityButton[i]->GetWidget()->SetBorderWidth(0);
    this->GlyphVisibilityButton[i]->GetWidget()->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
    this->GlyphVisibilityButton[i]->SetBalloonHelpString("Set visibility of glyphs.");
    this->GlyphVisibilityButton[i]->SetLabelText(glyphNames[i]);
    this->GlyphVisibilityButton[i]->SetLabelPositionToRight();
    this->Script("pack %s -side left -anchor w -padx 2 ",
      this->GlyphVisibilityButton[i]->GetWidgetName());
    }

  //create resolution scale
  this->GlyphSpacingScale = vtkKWScaleWithLabel::New();
  this->GlyphSpacingScale->SetParent(this->TestFrame->GetFrame());
  this->GlyphSpacingScale->Create();
  this->GlyphSpacingScale->SetLabelText("Glyph Spacing: ");
  this->GlyphSpacingScale->SetLabelPositionToLeft();
  this->GlyphSpacingScale->GetWidget()->SetRange(1,50);
  this->GlyphSpacingScale->GetWidget()->SetResolution(1);
  this->GlyphSpacingScale->GetWidget()->SetValue(20);
  this->GlyphSpacingScale->SetBalloonHelpString("Skip step for glyphs.");
  this->Script("pack %s -side top -anchor w -fill x -padx 2 -pady 6",
    this->GlyphSpacingScale->GetWidgetName());

  //create tract visibility label
  this->TractVisibilityLabel = vtkKWLabel::New();
  this->TractVisibilityLabel->SetParent(this->TestFrame->GetFrame());
  this->TractVisibilityLabel->Create();
  this->TractVisibilityLabel->SetText("Tract Visibility:  ");
  this->Script("pack %s -side left -anchor w -padx 2 ",
    this->TractVisibilityLabel->GetWidgetName());

  //create view tracts
  this->TractVisibilityButton = vtkKWPushButtonWithLabel::New();
  this->TractVisibilityButton->SetParent(this->TestFrame->GetFrame());
  this->TractVisibilityButton->Create();
  this->TractVisibilityButton->GetWidget()->SetReliefToFlat();
  this->TractVisibilityButton->GetWidget()->SetOverReliefToNone();
  this->TractVisibilityButton->GetWidget()->SetBorderWidth(0);
  this->TractVisibilityButton->SetLabelText("Based on Fiducial List");
  this->TractVisibilityButton->SetLabelPositionToRight();
  this->TractVisibilityButton->SetBalloonHelpString("Set visibility of tracts.");
  this->Script("pack %s -side left -anchor w ", 
    this->TractVisibilityButton->GetWidgetName());

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

  this->Script("pack %s %s -side left -anchor w -pady 2", 
    this->TractVisibilityButton->GetWidgetName(),
    this->FiducialSelector->GetWidgetName());
  } 
