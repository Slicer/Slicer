#include "vtkChangeTrackerSegmentationStep.h"

#include "vtkChangeTrackerGUI.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWRange.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWScale.h"
#include "vtkImageAccumulate.h"
#include "vtkImageThreshold.h"
#include "vtkSlicerVolumesLogic.h" 
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkImageIslandFilter.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerColorLogic.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerSegmentationStep);
vtkCxxRevisionMacro(vtkChangeTrackerSegmentationStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkChangeTrackerSegmentationStep::vtkChangeTrackerSegmentationStep()
{
  this->SetName("3/4. Identify Tumor in First Scan"); 
  this->SetDescription("Move slider to outline boundary of tumor"); 
  
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerSegmentationStep::WizardGUICallback);

  this->ThresholdFrame = NULL;
  this->ThresholdRange = NULL;
  this->ThresholdLabel = NULL;

  this->PreSegment = NULL;
  this->PreSegmentNode = NULL;
  this->SegmentNode = NULL;

  this->Scan1_SegmSelector = NULL;
//  this->Scan2_SegmSelector = NULL;
}

//----------------------------------------------------------------------------
vtkChangeTrackerSegmentationStep::~vtkChangeTrackerSegmentationStep()
{
 
  if (this->ThresholdFrame)
    {
    this->ThresholdFrame->Delete();
    this->ThresholdFrame = NULL;
    }

  if (this->ThresholdRange)
    {
    this->ThresholdRange->Delete();
    this->ThresholdRange = NULL;
    }

  if (this->ThresholdLabel)
    {
    this->ThresholdLabel->Delete();
    this->ThresholdLabel = NULL;
    }

  if (this->Scan1_SegmSelector)
    {
    this->Scan1_SegmSelector->Delete();
    this->Scan1_SegmSelector = NULL;
    }

//  if (this->Scan2_SegmSelector)
//    {
//    this->Scan2_SegmSelector->Delete();
//    this->Scan2_SegmSelector = NULL;
//    }

  this->PreSegmentScan1Remove();
  this->SegmentScan1Remove();
  this->GetGUI()->SliceLogicRemove();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSegmentationStep::ShowUserInterface()
{
  // ----------------------------------------
  // Display Super Sampled Volume 
  // ----------------------------------------

  // cout << "vtkChangeTrackerSegmentationStep::ShowUserInterface()" << endl;

  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  int intMin, intMax;

  if (node) { 
    vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SuperSampleRef()));
    if (volumeNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeNode->GetID());
      applicationLogic->PropagateVolumeSelection();

      double *imgRange = volumeNode->GetImageData()->GetPointData()->GetScalars()->GetRange();

      intMin = int(imgRange[0]);
      intMax = int(imgRange[1]);
    } else {
      intMin = 0;
      intMax = 0;      
    } 
  } else {
      intMin = 0;
      intMax = 0;      
  }

  //  cout << "What h" << endl;
  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------
  this->vtkChangeTrackerStep::ShowUserInterface();
  this->Frame->SetLabelText("Identify Tumor");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());

  if (!this->ThresholdFrame)
    {
    this->ThresholdFrame = vtkKWFrame::New();
    }
  if (!this->ThresholdFrame->IsCreated())
    {
      this->ThresholdFrame->SetParent(this->Frame->GetFrame());
     this->ThresholdFrame->Create();
  }


  if (!this->ThresholdLabel)
    {
    this->ThresholdLabel = vtkKWLabel::New();
    }
  if (!this->ThresholdLabel->IsCreated())
  {
    this->ThresholdLabel->SetParent(this->ThresholdFrame);
    this->ThresholdLabel->Create();
    this->ThresholdLabel->SetText("Threshold:");
  }

  if (!this->ThresholdRange)
    {
    this->ThresholdRange = vtkKWRange::New();
    }
  if (!this->ThresholdRange->IsCreated())
  {
    this->ThresholdRange->SetParent(this->ThresholdFrame);
    this->ThresholdRange->Create();
    this->ThresholdRange->SymmetricalInteractionOff();
    this->ThresholdRange->SetCommand(this, "ThresholdRangeChangedCallback"); 
    this->ThresholdRange->SetWholeRange(intMin, intMax); 
    this->ThresholdRange->SetResolution(1);

  }

  this->Script("pack %s -side top -anchor nw -padx 0 -pady 3",this->ThresholdFrame->GetWidgetName()); 
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->ThresholdLabel->GetWidgetName(),this->ThresholdRange->GetWidgetName());

  this->AdvancedFrame->SetLabelText("Advanced settings");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
    this->AdvancedFrame->GetWidgetName());

  if(!this->Scan1_SegmSelector)
    {
    this->Scan1_SegmSelector = vtkSlicerNodeSelectorWidget::New();
    }
  if(!this->Scan1_SegmSelector->IsCreated())
    {
    this->Scan1_SegmSelector->SetParent(this->AdvancedFrame->GetFrame());
    this->Scan1_SegmSelector->Create();
    this->Scan1_SegmSelector->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH*2);
    this->Scan1_SegmSelector->SetNodeClass("vtkMRMLScalarVolumeNode", "LabelMap", "1", "");
    this->Scan1_SegmSelector->NoneEnabledOn();
    this->Scan1_SegmSelector->NewNodeEnabledOff();
    this->Scan1_SegmSelector->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
    this->Scan1_SegmSelector->SetLabelText("Segmentation of the 1st scan");
    this->Scan1_SegmSelector->SetBalloonHelpString("Specify segmentation of the first time-point. If not available, leave as \"None\"");
    this->Scan1_SegmSelector->SetEnabled(1);
  }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->Scan1_SegmSelector->GetWidgetName());

#if 0 // no scan2 segmentation in 3.6
  if(!this->Scan2_SegmSelector)
    {
    this->Scan2_SegmSelector = vtkSlicerNodeSelectorWidget::New();
    }
  if(!this->Scan2_SegmSelector->IsCreated())
    {
    this->Scan2_SegmSelector->SetParent(this->AdvancedFrame->GetFrame());
    this->Scan2_SegmSelector->Create();
    this->Scan2_SegmSelector->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH*2);
    this->Scan2_SegmSelector->SetNodeClass("vtkMRMLScalarVolumeNode", "LabelMap", "1", "");
    this->Scan2_SegmSelector->NoneEnabledOn();
    this->Scan2_SegmSelector->NewNodeEnabledOff();
    this->Scan2_SegmSelector->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
    this->Scan2_SegmSelector->SetLabelText("Segmentation of the 2nd scan");
    this->Scan2_SegmSelector->SetBalloonHelpString("Specify segmentation of the second time-point. If not available, leave as \"None\"");
    this->Scan2_SegmSelector->SetEnabled(0);
  }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->Scan2_SegmSelector->GetWidgetName());
#endif 

  this->AdvancedFrame->CollapseFrame();

  this->CreateGridButton(); 
  this->CreateSliceButton();

  // this->CreateResetButton(); 
  // ----------------------------------------
  // Show segmentation 
  // ----------------------------------------
  this->PreSegmentScan1Define();

  {
    vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
    double min, max;
    if (mrmlNode && (mrmlNode->GetSegmentThresholdMin() > -1) && (mrmlNode->GetSegmentThresholdMax() > -1)) {
      min =  mrmlNode->GetSegmentThresholdMin();
      max =  mrmlNode->GetSegmentThresholdMax();
    } else {
      min = (intMax - intMin)/2.0;
      max = intMax;
    }
    this->ThresholdRange->SetRange(min,max);

    // Necesary in order to transfere results from above lines  
    this->ThresholdRangeChangedCallback(min, max);
  }
 
  this->AddGUIObservers();

  // Show Reference Image 1 in the 3D Slicer Viewer
  // this->GetGUI()->SliceLogicDefine(); 

 
  // Kilian
  // this->TransitionCallback();   
  //  ~/Slicer/Slicer3/Base/Logic/vtkSlicersModelsLogic Clone 
  // -> copy slice modules
  //  -> look in ~/Slicer/Slicer3/Libs/MRMLLogic/vtkMRMLSliceLogic 
  //    -> CreateSlicerModel 
  //       -> do a copy 
  // 
  // -> Look up pid 
  // like vtkSlicersVolumeLogic 
  // 
  // 
  // use ddd 
  // or 
  // use gdb
  // -> attach pid 
  // after it loads
  // -> c 
  // after it gives a seg fault just say 
  // -> where
  
  // cout << "End Show user interface" << endl;
}

void vtkChangeTrackerSegmentationStep::PreSegmentScan1Remove() {
  // cout << "vtkChangeTrackerSegmentationStep::PreSegmentScan1Remove() Start " << this->PreSegmentNode << endl;  
  if (this->PreSegmentNode && this->GetGUI()) {
    this->GetGUI()->GetMRMLScene()->RemoveNode(this->PreSegmentNode);  
  } 
  this->PreSegmentNode = NULL;

  if (this->PreSegment) {
    this->PreSegment->Delete();
    this->PreSegment = NULL;
  }

  this->RenderRemove();

  // cout << "vtkChangeTrackerSegmentationStep::PreSegmentScan1Remove() End " << endl;
}

void vtkChangeTrackerSegmentationStep::PreSegmentScan1Define() {

  // ---------------------------------
  // Initialize Function
  // ---------------------------------
  vtkMRMLChangeTrackerNode* Node      =  this->GetGUI()->GetNode();
  if (!Node) return;
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_SuperSampleRef()));
  if (!volumeNode) return;
  //vtkMRMLVolumeNode *spgrNode = vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
  // if (!spgrNode) return;
  if (!this->ThresholdRange) return;

  vtkSlicerApplication      *application      =  vtkSlicerApplication::SafeDownCast(this->GetApplication()); 
  // availability of Volumes module has been verified in the first step of the
  // wizard
  vtkSlicerVolumesLogic     *volumesLogic      = (vtkSlicerVolumesGUI::SafeDownCast(application->GetModuleGUIByName("Volumes")))->GetLogic();

  if (this->PreSegment || this->PreSegmentNode) this->PreSegmentScan1Remove();

  // ---------------------------------
  // Define LabelMap 
  // ---------------------------------

  this->PreSegment = vtkImageThreshold::New(); 
  int range[2] = {int(this->ThresholdRange->GetRange()[0]),int(this->ThresholdRange->GetRange()[1])}; 
  vtkChangeTrackerLogic::DefinePreSegment(volumeNode->GetImageData(),range,this->PreSegment);

  char segmNodeName[255];
  sprintf(segmNodeName, "%s_VOI_PreSegmented", this->GetGUI()->GetLogic()->GetInputScanName(0));
  this->PreSegmentNode = volumesLogic->CreateLabelVolume(Node->GetScene(),volumeNode, segmNodeName);
  
  vtkSmartPointer<vtkSlicerColorLogic> colorLogic =
    vtkSmartPointer<vtkSlicerColorLogic>::New();
  this->PreSegmentNode->GetDisplayNode()->SetAndObserveColorNodeID
      (colorLogic->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Labels));

  this->PreSegmentNode->SetAndObserveImageData(this->PreSegment->GetOutput());
  
  this->CreateRender(volumeNode, 0);
  float color[3] = {0.8, 0.8, 0.0};
  this->SetRender_BandPassFilter(range[0],range[1],color,color);
  
  vtkSlicerApplicationGUI   *applicationGUI   = this->GetGUI()->GetApplicationGUI();
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);

  applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);

//  this->ShowSegmentedVolume(this->PreSegmentNode);
//  this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
  
  return;
}

void vtkChangeTrackerSegmentationStep::SegmentScan1Remove() {
  if (this->GetGUI()) { 
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
    if (Node) {
      vtkMRMLVolumeNode* currentNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_SegmentRef()));
      if (currentNode) this->GetGUI()->GetMRMLScene()->RemoveNode(currentNode); 
      Node->SetScan1_SegmentRef(NULL);
    }
  }
  if (this->SegmentNode) {
    //this->SegmentNode->Delete();
    this->SegmentNode = NULL;
  }
}

int vtkChangeTrackerSegmentationStep::SegmentScan1Define() {
  // Initialize
  if (!this->PreSegment || !this->PreSegmentNode) return 0;
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return 0 ;

  this->SegmentScan1Remove();

  vtkImageIslandFilter *RemoveIslands = vtkImageIslandFilter::New();
  vtkChangeTrackerLogic::DefineSegment(this->PreSegment->GetOutput(),RemoveIslands);

  // Set It up 
  vtkSlicerVolumesLogic *volumesLogic = 
    (vtkSlicerVolumesGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes")))->GetLogic();
  char segmNodeName[255];
  sprintf(segmNodeName, "%s_VOI_Segmented", this->GetGUI()->GetLogic()->GetInputScanName(0));

  this->SegmentNode = volumesLogic->CreateLabelVolume(Node->GetScene(), this->PreSegmentNode, segmNodeName);

  vtkSmartPointer<vtkSlicerColorLogic> colorLogic =
    vtkSmartPointer<vtkSlicerColorLogic>::New();
  this->SegmentNode->GetDisplayNode()->SetAndObserveColorNodeID
      (colorLogic->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Labels));

  //return 1;

  this->SegmentNode->SetAndObserveImageData(RemoveIslands->GetOutput());

  RemoveIslands->Delete(); 
  this->PreSegmentScan1Remove();

  // Added it to MRML Script
  Node->SetScan1_SegmentRef(this->SegmentNode->GetID());


  return 1;
}


//----------------------------------------------------------------------------
void vtkChangeTrackerSegmentationStep::ThresholdRangeChangedCallback(double min , double max)
{
  if(!this->PreSegment)
    {
    std::cerr << "Cannot update render -- presegment node empty" << std::endl;
    return;
    }

  if (!this->ThresholdRange || !this->PreSegment) return;
  PreSegment->ThresholdBetween(min,max); 
  PreSegment->Update();
  this->PreSegmentNode->Modified();

  vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
  if (!mrmlNode)
    {
    std::cerr << "Cannot update render -- no node!" << std::endl;
    return;
    }

  mrmlNode->SetSegmentThresholdMin(min);
  mrmlNode->SetSegmentThresholdMax(max);

  // 3D Render 
  this->ChangeRender_BandPassFilter(min,max);
  this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();

  // set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
  // set STEP [$GUI GetSegmentationStep]
  // set FILT [$STEP GetPreSegment]

  // You can also watch MRML by doing 
  // MRMLWatcher m
  // parray MRML
  // $MRML(TG_scan1_SuperSampled) Print

}

//----------------------------------------------------------------------------
void vtkChangeTrackerSegmentationStep::TransitionCallback() 
{   
  vtkMRMLScalarVolumeNode *segmNode = NULL;
  vtkMRMLChangeTrackerNode *ctNode;

  ctNode = this->GetGUI()->GetNode();

  if(this->Scan1_SegmSelector->GetSelected())
    segmNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->Scan1_SegmSelector->GetSelected());
 
  if(!segmNode || segmNode==this->PreSegmentNode)
    {
    this->SegmentScan1Remove();
    /*
    if(this->Scan1_SegmSelector() && this->Scan1_SegmSelector()->GetSelected())
    {
    this->SetScan1_SegmentRef(this->Scan1_SegmSelector()->GetSelected()->GetID());
    }
    if(this->Scan2_SegmSelector() && this->Scan2_SegmSelector()->GetSelected())
    {
    this->SetScan2_SegmentRef(this->Scan2_SegmSelector()->GetSelected()->GetID());
    }
    */
    if (!this->SegmentScan1Define()) 
      return; 
    }
  else
    {
    // Do nothing -- done in GUI event handler for node selector
    /*
    // segmentation is supplied -- check whether resampling is needed or not,
    // and assign to this->SegmentNode
    // For now, allow the segmentation to be either in the pixel space of the
    // original scan, or in the supersampled space of ROI, assume user does
    // not give garbage...
    vtkMRMLScalarVolumeNode *scan1Node = 
      vtkMRMLScalarVolumeNode::SafeDownCast(ctNode->GetScene()->GetNodeByID(ctNode->GetScan1_Ref()));
    vtkImageData *scan1Image = scan1Node->GetImageData(),
                 *segmImage = segmNode->GetImageData();
    int *scan1ImageDim = scan1Image->GetDimensions(),
        *segmImageDim = segmImage->GetDimensions();
    if(scan1ImageDim[0]==segmImageDim[0] && 
       scan1ImageDim[1]==segmImageDim[1] &&
       scan1ImageDim[2]==segmImageDim[2])
      {
      std::cerr << "Input segmented image is the segmentation of scan1: \
        need to extract ROI, and resample" << std::endl;
      vtkMRMLScalarVolumeNode *resampledScan1Segm =
        this->GetGUI()->GetLogic()->CreateSuperSample(0);
      this->GetGUI()->GetLogic()->DeleteSuperSample(0);
      ctNode->SetScan1_InputSegmentSuperSampleRef(resampledScan1Segm->GetID());
      ctNode->SetScan1_SegmentRef(resampledScan1Segm->GetID());
      }
    else
      {
      std::cerr << "Input segmented image is the segmentation of resampled ROI: \
        nothing to do -- just assigned segmentation node to the input \
        segmentation" << std::endl;
      ctNode->SetScan1_SegmentRef(ctNode->GetScan1_InputSegmentRef());
      }
      */
    }
  /*
  vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  this->GetGUI()->GetLogic()->SaveVolume(application,this->SegmentNode); 
  */

  // Proceed to next step 
  this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
}


//----------------------------------------------------------------------------
void vtkChangeTrackerSegmentationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkChangeTrackerSegmentationStep::RemoveResults()  { 
    this->PreSegmentScan1Remove();
    this->GetGUI()->SliceLogicRemove();
}

void vtkChangeTrackerSegmentationStep::AddGUIObservers()
{
  if(this->Scan1_SegmSelector && (!this->Scan1_SegmSelector->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand)))
    {
    this->Scan1_SegmSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand);
    }
#if 0
  if(this->Scan2_SegmSelector && (!this->Scan2_SegmSelector->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand)))
    {
    this->Scan2_SegmSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand);
    }
#endif
}

void vtkChangeTrackerSegmentationStep::RemoveGUIObservers()
{
  if (this->Scan1_SegmSelector && this->WizardGUICallbackCommand)
    {
    this->Scan1_SegmSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand);
//    this->Scan2_SegmSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand);
    }
}

void vtkChangeTrackerSegmentationStep::ProcessGUIEvents(vtkObject *caller,
                                                        unsigned long event,
                                                        void *vtkNotUsed(callData))
{
  vtkSlicerNodeSelectorWidget *sel = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if(this->Scan1_SegmSelector && sel == this->Scan1_SegmSelector &&
     event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
//     this->Scan1_SegmSelector->GetSelected() != NULL)
    {
    // for the segmentation of the first scan, update the node, and set label
    // in the node selector
    if(this->Scan1_SegmSelector->GetSelected())
      {
      vtkMRMLChangeTrackerNode *ctNode = this->GetGUI()->GetNode();
      
      ctNode->SetScan1_InputSegmentRef(this->Scan1_SegmSelector->GetSelected()->GetID());

      vtkMRMLScalarVolumeNode *segmNode = 
        vtkMRMLScalarVolumeNode::SafeDownCast(this->Scan1_SegmSelector->GetSelected());
      
      vtkMRMLScalarVolumeNode *scan1Node = 
        vtkMRMLScalarVolumeNode::SafeDownCast(ctNode->GetScene()->GetNodeByID(ctNode->GetScan1_Ref()));
      if(!segmNode || !scan1Node)
        return;
      vtkMRMLScalarVolumeNode *resampledScan1Segm;
      vtkImageData *scan1Image = scan1Node->GetImageData(),
                   *segmImage = segmNode->GetImageData();
      if(!scan1Image || !segmImage)
        return;
      int *scan1ImageDim = scan1Image->GetDimensions(),
          *segmImageDim = segmImage->GetDimensions();
      // resample right away. problem -- gui unresponsive, long processing in
      // event handler
      if(scan1ImageDim[0]==segmImageDim[0] && 
        scan1ImageDim[1]==segmImageDim[1] &&
        scan1ImageDim[2]==segmImageDim[2])
        {
        std::cerr << "Input segmented image is the segmentation of scan1: \
          need to extract ROI, and resample" << std::endl;
        resampledScan1Segm = this->GetGUI()->GetLogic()->CreateSuperSample(0);
        this->GetGUI()->GetLogic()->DeleteSuperSample(0);
        ctNode->SetScan1_InputSegmentSuperSampleRef(resampledScan1Segm->GetID());
        ctNode->SetScan1_SegmentRef(resampledScan1Segm->GetID());
        // assume label is within [1,255]
        ctNode->SetSegmentThresholdMin(1);
        ctNode->SetSegmentThresholdMax(255);
        }
      else
        {
        std::cerr << "Input segmented image is the segmentation of resampled ROI: \
          nothing to do -- just assigned segmentation node to the input \
          segmentation" << std::endl;
        ctNode->SetScan1_SegmentRef(ctNode->GetScan1_InputSegmentRef());
        resampledScan1Segm = 
          vtkMRMLScalarVolumeNode::SafeDownCast(ctNode->GetScene()->GetNodeByID(ctNode->GetScan1_InputSegmentRef()));
        }
      // remove islands
      vtkImageIslandFilter *removeIslands = vtkImageIslandFilter::New();
      removeIslands->SetInput(resampledScan1Segm->GetImageData());
      removeIslands->SetIslandMinSize(1000);
      removeIslands->SetNeighborhoodDim3D();
      removeIslands->Update();
      resampledScan1Segm->SetAndObserveImageData(removeIslands->GetOutput());
      removeIslands->Delete();
      this->RenderRemove();
      // keep the pre-segment scan, so that the user can go back
      //      this->PreSegmentScan1Remove();
      this->ShowSegmentedVolume(resampledScan1Segm);
      this->ThresholdRange->SetEnabled(0);
      }
    else
      {
      this->ThresholdRange->SetEnabled(1);
      this->RenderRemove();
      // if "None" is selected, go back to pre-segment to show
      this->GetGUI()->GetNode()->SetScan1_InputSegmentRef(NULL);
      this->ShowSegmentedVolume(this->PreSegmentNode);
      }
    }

//  if(this->Scan2_SegmSelector && sel == this->Scan2_SegmSelector &&
//     event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
//     this->Scan2_SegmSelector->GetSelected() != NULL)
//    {
//    this->GetGUI()->GetNode()->SetScan2_InputSegmentRef(this->Scan2_SegmSelector->GetSelected()->GetID());
//    }
}

void vtkChangeTrackerSegmentationStep::ShowSegmentedVolume(vtkMRMLScalarVolumeNode *segVol)
{
  // ---------------------------------
  // show segmentation in Slice view 
  // ------------------------------
  
  vtkSlicerApplicationGUI   *applicationGUI   = this->GetGUI()->GetApplicationGUI();
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(segVol->GetID());
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(segVol->GetID());
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(segVol->GetID());
  
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);

  applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);
  // ------------------------------------
  // Show Segmentation through 3D Volume Rendering
  // ------------------------------------
  //float color[3] = { 0.8, 0.8, 0.0 };

  this->CreateRender(segVol, 0);
  // assume the label value is between 1 and 255
//  this->SetRender_BandPassFilter(1, 255, color, color);
  this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
}

void vtkChangeTrackerSegmentationStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkChangeTrackerSegmentationStep *self = reinterpret_cast<vtkChangeTrackerSegmentationStep *>(clientData);
    if (self) 
      { 
      self->ProcessGUIEvents(caller, event, callData); 
      }
}

