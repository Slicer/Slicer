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

#include "vtkSlicerModelsLogic.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerSegmentationStep);
vtkCxxRevisionMacro(vtkChangeTrackerSegmentationStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkChangeTrackerSegmentationStep::vtkChangeTrackerSegmentationStep()
{
  this->SetName("3/4. Identify Tumor in First Scan"); 
  this->SetDescription("Move slider to outline boundary of tumor"); 

  this->ThresholdFrame = NULL;
  this->ThresholdRange = NULL;
  this->ThresholdLabel = NULL;

  this->PreSegment = NULL;
  this->PreSegmentNode = NULL;
  this->SegmentNode = NULL;
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
   

  // Show Reference Image 1 in the 3D Slicer Viewer
  // this->GetGUI()->SliceLogicDefine(); 

 
  // Kilian
  // this->TransitionCallback();   
  //  ~/Slicer/Slicer3/Base/Logic/vtkSlicersModelsLogic Clone 
  // -> copy slice modules
  //  -> look in ~/Slicer/Slicer3/Base/Logic/vtkSlicerSliceLogic 
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
  vtkSlicerApplicationGUI   *applicationGUI   = this->GetGUI()->GetApplicationGUI();
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

  // ---------------------------------
  // show segmentation in Slice view 
  // ------------------------------
  char segmNodeName[255];
  sprintf(segmNodeName, "%s_VOI_PreSegmented", this->GetGUI()->GetLogic()->GetInputScanName(0));
  this->PreSegmentNode = volumesLogic->CreateLabelVolume(Node->GetScene(),volumeNode, segmNodeName);
  this->PreSegmentNode->SetAndObserveImageData(this->PreSegment->GetOutput());
  
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(this->PreSegmentNode->GetID());
  
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);

  applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);
  // ------------------------------------
  // Show Segmentation through 3D Volume Rendering
  // ------------------------------------
  float color[3] = { 0.8, 0.8, 0.0 };

  this->CreateRender(volumeNode, 0);
  this->SetRender_BandPassFilter(range[0],range[1], color, color);
  
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
  if (!this->ThresholdRange || !this->PreSegment) return;
  PreSegment->ThresholdBetween(min,max); 
  PreSegment->Update();
  this->PreSegmentNode->Modified();

  vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
  if (!mrmlNode) return;
  mrmlNode->SetSegmentThresholdMin(min);
  mrmlNode->SetSegmentThresholdMax(max);

  // 3D Render 
  this->ChangeRender_BandPassFilter(min,max);
  this->GetGUI()->GetApplicationGUI()->GetViewerWidget()->RequestRender();


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
  this->SegmentScan1Remove();
  if (!this->SegmentScan1Define()) return; 
  vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  this->GetGUI()->GetLogic()->SaveVolume(application,this->SegmentNode); 

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
