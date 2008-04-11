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
  this->ViewGlyphs = NULL;
  this->ViewTracts = NULL;
  this->RunFrame = NULL;
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
  if (this->RunFrame)
    {
    this->RunFrame->SetParent (NULL);
    this->RunFrame->Delete();
    this->RunFrame = NULL;
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
  if (this->ViewGlyphs)
    {
    this->ViewGlyphs->Delete();
    this->ViewGlyphs = NULL;
    }
  if (this->ViewTracts)
    {
    this->ViewTracts->Delete();
    this->ViewTracts = NULL;
    }
  this->ModifiedForNewTensor = 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::AddWidgetObservers ( )
  {    
  this->RunButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FiducialSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewGlyphs->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  }

//---------------------------------------------------------------------------
void vtkSlicerDWITestingWidget::RemoveWidgetObservers( )
  {
  this->RunButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DTISelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ViewGlyphs->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ViewTracts->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
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
    if(this->ViewGlyphs->GetSelectedState())
      {
      //TODO 
      }
    if(this->ViewTracts->GetSelectedState())
      {
      this->CreateTracts(); //start tractography seeding
      }

    }

  if (this->DTISelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->DTISelector->GetSelected() != NULL) 
    {
    //set internal tensorNode
    this->TensorNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->DTISelector->GetSelected());
    //create tracts
    if(this->ViewTracts->GetSelectedState()) this->CreateTracts();
    //create glyphs
    if(this->ViewGlyphs->GetSelectedState()) this->CreateTracts();
    }

  if (this->FiducialSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->FiducialSelector->GetSelected() != NULL) 
    {
    if(this->ViewTracts->GetSelectedState() && this->DTISelector->GetSelected())
      {
      this->CreateTracts();
      }
    }

  //view of glyphs
  if(this->ViewGlyphs == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //get the existing GUI of the "Volumes Module"
    vtkSlicerVolumesGUI *volumesGUI = vtkSlicerVolumesGUI::SafeDownCast(
      this->Application->GetModuleGUIByName("Volumes"));
    volumesGUI->Enter(); 
    if(this->ViewGlyphs->GetSelectedState())
      {
      volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(0);
      volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetDiffusionTensorVolumeNode(this->TensorNode);
      volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(1);
      this->Application->GetApplicationGUI()->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID(this->TensorNode->GetID());
      this->Application->GetApplicationGUI()->GetApplicationLogic()->PropagateVolumeSelection();
      }
    else
      {
      volumesGUI->GetdtiVDW()->GetGlyphDisplayWidget()->SetGlyphVisibility(0);
      }
    }

  //view of tracts
  if(this->ViewTracts == vtkKWCheckButton::SafeDownCast(caller) && event == vtkKWCheckButton::SelectedStateChangedEvent
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
  this->TestFrame->SetLabelText("Test (Tensor Estimation & Tractography Fiducial Seeding)");
  this->Script("pack %s -side top -anchor n -fill x -padx 2 -pady 4", 
    this->TestFrame->GetWidgetName());

  ////create frame for run button and fiducial list
  //this->RunFrame = vtkKWFrame::New();
  //this->RunFrame->SetParent(this->TestFrame->GetFrame());
  //this->RunFrame->Create();
  //this->Script("pack %s -side top -anchor ne -padx 2 -pady 2", 
  //  this->RunFrame->GetWidgetName());

  //create run button
  this->RunButton = vtkKWPushButtonWithLabel::New();
  this->RunButton->SetParent(this->TestFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetLabelText("Estimate new Tensor: ");
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
  this->DTISelector->SetLabelText("Display a DTI Volume: ");
  this->DTISelector->SetBalloonHelpString("Select a DTI volume from the current mrml scene and see its tracts.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 4 ", 
    this->DTISelector->GetWidgetName()); 

  this->ViewGlyphs = vtkKWCheckButton::New();
  this->ViewGlyphs->SetParent(this->TestFrame->GetFrame());
  this->ViewGlyphs->Create();
  this->ViewGlyphs->SetText("View Glyphs ");
  this->Script("pack %s -side top -anchor nw -padx 2 ", 
    this->ViewGlyphs->GetWidgetName());

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


 void vtkSlicerDWITestingWidget::SetModifiedForNewTensor(int modified)
   {
   this->ModifiedForNewTensor = modified;
   }
