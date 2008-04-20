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
#include "vtkSlicerGradientEditorLogic.h"
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
  this->ViewGlyphsRed = NULL;
  this->ViewGlyphsGreen = NULL;
  this->ViewGlyphsYellow = NULL;
  this->ViewTracts = NULL;
  this->VolumesGUI = NULL;
  }

//---------------------------------------------------------------------------
vtkSlicerDWITestingWidget::~vtkSlicerDWITestingWidget(void)
  {
  this->RemoveWidgetObservers();
  if (this->ActiveVolumeNode)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
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
  if (this->ViewGlyphsRed)
    {
    this->ViewGlyphsRed->SetParent (NULL);
    this->ViewGlyphsRed->Delete();
    this->ViewGlyphsRed = NULL;
    }
  if (this->ViewGlyphsGreen)
    {
    this->ViewGlyphsGreen->SetParent (NULL);
    this->ViewGlyphsGreen->Delete();
    this->ViewGlyphsGreen = NULL;
    }
  if (this->ViewGlyphsYellow)
    {
    this->ViewGlyphsYellow->SetParent (NULL);
    this->ViewGlyphsYellow->Delete();
    this->ViewGlyphsYellow = NULL;
    }
  if (this->ViewTracts)
    {
    this->ViewTracts->SetParent (NULL);
    this->ViewTracts->Delete();
    this->ViewTracts = NULL;
    }
  if (this->GlyphResolutionScale)
    {
    this->GlyphResolutionScale->SetParent (NULL);
    this->GlyphResolutionScale->Delete();
    this->GlyphResolutionScale = NULL;
    }
  this->ModifiedForNewTensor = 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::AddWidgetObservers ( )
  {    
  this->RunButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FiducialSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewGlyphsRed->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewGlyphsGreen->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewGlyphsYellow->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphResolutionScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::RemoveWidgetObservers( )
  {
  this->RunButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewGlyphsRed->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewGlyphsGreen->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewGlyphsYellow->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GlyphResolutionScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
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
    if(this->ModifiedForNewTensor || this->MRMLScene->GetNodesByName("GradientenEditor_Tensor_Node")->GetNumberOfItems() == 0 )
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
      this->BaselineNode->SetName("GradientenEditor_Baseline_Node");
      this->MRMLScene->AddNode(this->BaselineNode);

      this->MaskNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLScalarVolumeNode"));
      this->MaskNode->SetScene(this->GetMRMLScene());
      this->MaskNode->SetName("GradientenEditor_Threshold_Mask");
      this->MRMLScene->AddNode(this->MaskNode);

      if(this->TensorNode)
        {
        this->TensorNode->Delete(); //delete previous data to avoid leaks
        }
      this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
        this->MRMLScene->CreateNodeByClass("vtkMRMLDiffusionTensorVolumeNode"));
      this->TensorNode->SetScene(this->GetMRMLScene());
      this->TensorNode->SetName("GradientenEditor_Tensor_Node");
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
    this->DTISelector->SetSelected(this->TensorNode);
    this->RunButton->SetEnabled(1);
    //create tracts and glyphs
    if(this->ViewTracts->GetSelectedState()) this->CreateTracts();
    if(this->ViewGlyphsRed->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsRed);
    if(this->ViewGlyphsYellow->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsYellow);
    if(this->ViewGlyphsGreen->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsGreen);
    }

  //dti selected (update tracts)
  else if (this->DTISelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->DTISelector->GetSelected() != NULL) 
    {
    //set internal tensorNode
    this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->DTISelector->GetSelected());
    //create tracts and glyphs
    if(this->ViewTracts->GetSelectedState()) this->CreateTracts();
    if(this->ViewGlyphsRed->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsRed);
    if(this->ViewGlyphsYellow->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsYellow);
    if(this->ViewGlyphsGreen->GetSelectedState()) this->CreateGlyphs(this->ViewGlyphsGreen);
    }

  //create tracts when fiducials list is selected 
  else if (this->FiducialSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->FiducialSelector->GetSelected() != NULL) 
    {
    if(this->ViewTracts->GetSelectedState() && this->DTISelector->GetSelected())
      {
      this->CreateTracts();
      }
    }

  //view of glyphs
  else if(event == vtkKWCheckButton::SelectedStateChangedEvent && this->TensorNode != NULL)
    {
    vtkKWCheckButton *calledGlyph = vtkKWCheckButton::SafeDownCast(caller);
    this->CreateGlyphs(calledGlyph);
    }

  //glyph resolution
  else if (vtkKWScale::SafeDownCast(caller) == this->GlyphResolutionScale->GetWidget() && 
    event == vtkKWScale::ScaleValueChangedEvent && this->TensorNode != NULL && 
    (this->ViewGlyphsYellow->GetSelectedState() || this->ViewGlyphsRed->GetSelectedState() 
    || this->ViewGlyphsGreen->GetSelectedState()))
    {
    //get the existing GUI of the "Volumes Module"
    if(!this->VolumesGUI) this->CreateVolumesGUI();
    //set new resolution
    this->VolumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphRosolution(
      this->GlyphResolutionScale->GetWidget()->GetValue());
    }

  //view of tracts
  else if(this->ViewTracts == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent
    && this->FiducialSelector->GetSelected() != NULL)
    {
    //get the existing GUI of the "Tractography Display Module"
    vtkSlicerTractographyDisplayGUI *tractGUI = vtkSlicerTractographyDisplayGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("DisplayLoadSave"));
    tractGUI->Enter();
    if(this->ViewTracts->GetSelectedState() && this->DTISelector->GetSelected() != NULL)
      {
      //visibility of tracts on
      tractGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(1);
      this->CreateTracts(); //start tractography seeding
      }
    else
      {
      //visibility of tracts off
      tractGUI->GetFiberBundleDisplayWidget()->SetTractVisibility(0);  
      }
    }
  }

void vtkSlicerDWITestingWidget::CreateGlyphs(vtkKWCheckButton *calledGlyph)
  {
  //get the existing GUI of the "Volumes Module"
  if(!this->VolumesGUI) this->CreateVolumesGUI();  
  // which plane should change its visibility
  int plane = -1;
  if (this->ViewGlyphsRed == calledGlyph) plane = 0;
  else if (this->ViewGlyphsYellow == calledGlyph) plane = 1;
  else if (this->ViewGlyphsGreen == calledGlyph) plane = 2;
  if (plane == -1) return;

  if(calledGlyph->GetSelectedState())
    {
    //necessary when tensor node changed to switch off previous glyphs
    this->VolumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(plane,0);
    //view glyphs on
    this->VolumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetDiffusionTensorVolumeNode(this->TensorNode);
    this->VolumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(plane,1);
    //change current node to TensorNode in the main GUI
    this->Application->GetApplicationGUI()->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID(this->TensorNode->GetID());
    this->Application->GetApplicationGUI()->GetApplicationLogic()->PropagateVolumeSelection();
    }
  else
    {
    //view glyphs off
    this->VolumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(plane,0);
    }
  }

void vtkSlicerDWITestingWidget::CreateTracts ( )
  {
  if(this->TensorNode == NULL) return;
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
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, dwiNode); //set ActiveVolumeNode
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
  this->TestFrame->SetLabelText("Testing (Tensor Estimation & TractographySeeding & Glyphs)");
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 4", 
    this->TestFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButtonWithLabel::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetLabelText("Estimate new tensor: ");
  this->RunButton->GetWidget()->SetText("Run");
  this->RunButton->GetWidget()->SetWidth(10);
  this->RunButton->SetBalloonHelpString("Run test by computing tensors and tractography seeding.");
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 4 ", 
    this->RunButton->GetWidgetName()); 

  //create dti selector
  this->DTISelector = vtkSlicerNodeSelectorWidget::New();
  this->DTISelector->SetNodeClass("vtkMRMLDiffusionTensorVolumeNode", NULL, NULL, NULL);
  this->DTISelector->SetParent(this->TestFrame->GetFrame());
  this->DTISelector->Create();
  this->DTISelector->SetMRMLScene(this->GetMRMLScene());
  this->DTISelector->UpdateMenu();
  this->DTISelector->SetLabelText("Display a DTI volume: ");
  this->DTISelector->SetBalloonHelpString("Select a DTI volume from the current mrml scene and see its tracts or glyphs.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 4 ", 
    this->DTISelector->GetWidgetName());

  //create frame for glyphs 
  this->GlyphFrame = vtkKWFrame::New();
  this->GlyphFrame->SetParent(this->TestFrame->GetFrame());
  this->GlyphFrame->Create();
  this->Script("pack %s -side top -anchor nw -pady 2", 
    this->GlyphFrame->GetWidgetName());

  //create view red glyphs
  this->ViewGlyphsRed = vtkKWCheckButton::New();
  this->ViewGlyphsRed->SetParent(this->GlyphFrame);
  this->ViewGlyphsRed->Create();
  this->ViewGlyphsRed->SetText("View red glyphs.");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->ViewGlyphsRed->GetWidgetName());

  //create view yellow glyphs
  this->ViewGlyphsYellow = vtkKWCheckButton::New();
  this->ViewGlyphsYellow->SetParent(this->GlyphFrame);
  this->ViewGlyphsYellow->Create();
  this->ViewGlyphsYellow->SetText("View yellow glyphs.");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->ViewGlyphsYellow->GetWidgetName());

  //create view green glyphs
  this->ViewGlyphsGreen = vtkKWCheckButton::New();
  this->ViewGlyphsGreen->SetParent(this->GlyphFrame);
  this->ViewGlyphsGreen->Create();
  this->ViewGlyphsGreen->SetText("View green glyphs.");
  this->Script("pack %s -side left -anchor nw -padx 2 ", 
    this->ViewGlyphsGreen->GetWidgetName());

  //create resolution scale
  this->GlyphResolutionScale = vtkKWScaleWithLabel::New();
  this->GlyphResolutionScale->SetParent(this->TestFrame->GetFrame());
  this->GlyphResolutionScale->Create();
  this->GlyphResolutionScale->SetLabelText("Resolution of glyphs: ");
  this->GlyphResolutionScale->SetLabelPositionToLeft();
  this->GlyphResolutionScale->GetWidget()->SetRange(1,50);
  this->GlyphResolutionScale->GetWidget()->SetResolution(1);
  this->GlyphResolutionScale->GetWidget()->SetValue(20);
  this->GlyphResolutionScale->SetBalloonHelpString("Skip step for glyphs.");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 6",
    this->GlyphResolutionScale->GetWidgetName());

  //create view tracts
  this->ViewTracts = vtkKWCheckButton::New();
  this->ViewTracts->SetParent(this->TestFrame->GetFrame());
  this->ViewTracts->Create();
  this->ViewTracts->SetText("View Tracts based on Fiducial List");
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
  this->FiducialSelector->SetWidth(20);
  this->FiducialSelector->SetLabelPositionToRight();
  this->FiducialSelector->SetBalloonHelpString("Set Fiducial List for tractography seeding.");

  this->Script("pack %s %s -side left -anchor ne -padx 2 -pady 2", 
    this->ViewTracts->GetWidgetName(),
    this->FiducialSelector->GetWidgetName());
  } 

void vtkSlicerDWITestingWidget::CreateVolumesGUI()
  {
  if(!this->VolumesGUI)
    {
    //get the existing GUI of the "Volumes Module"
    this->VolumesGUI = vtkSlicerVolumesGUI::SafeDownCast(this->Application->GetModuleGUIByName("Volumes"));
    this->VolumesGUI->Enter();
    }
  }

void vtkSlicerDWITestingWidget::SetModifiedForNewTensor(int modified)
  {
  this->ModifiedForNewTensor = modified;
  }
