#include "vtkTumorGrowthROIStep.h"

#include "vtkTumorGrowthGUI.h"
#include "vtkTumorGrowthLogic.h"
#include "vtkMRMLTumorGrowthNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWMessageDialog.h"
#include "vtkImageRectangularSource.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerVolumesGUI.h" 
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWScale.h"

#include "vtkKWMatrixWidget.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTumorGrowthROIStep);
vtkCxxRevisionMacro(vtkTumorGrowthROIStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkTumorGrowthROIStep::vtkTumorGrowthROIStep()
{
  this->SetName("2/4. Define Volume of Interest"); 
  this->SetDescription("Define VOI by clicking left mouse button\n around the tumor or moving sliders"); 
  this->WizardGUICallbackCommand->SetCallback(vtkTumorGrowthROIStep::WizardGUICallback);

  this->FrameButtons    = NULL;
  this->FrameBlank      = NULL;
  this->FrameROI        = NULL;
  this->FrameROIX       = NULL;
  this->FrameROIY       = NULL;
  this->FrameROIZ       = NULL;
  this->ButtonsShow     = NULL;
  this->ButtonsReset    = NULL;
  this->ROIX            = NULL;
  this->ROIY            = NULL;
  this->ROIZ            = NULL;
  this->LabelROIX       = NULL;
  this->LabelROIY       = NULL;
  this->LabelROIZ       = NULL;
  this->ROILabelMapNode = NULL;
  this->ROILabelMap     = NULL;
}

//----------------------------------------------------------------------------
vtkTumorGrowthROIStep::~vtkTumorGrowthROIStep()
{
  if (this->FrameButtons)
  {
    this->FrameButtons->Delete();
    this->FrameButtons = NULL;
  }

  if (this->FrameBlank)
  {
    this->FrameBlank->Delete();
    this->FrameBlank = NULL;
  }

  if (this->FrameROI)
  {
    this->FrameROI->Delete();
    this->FrameROI = NULL;
  }

  if (this->FrameROIX)
  {
    this->FrameROIX->Delete();
    this->FrameROIX = NULL;
  }

  if (this->FrameROIY)
  {
    this->FrameROIY->Delete();
    this->FrameROIY = NULL;
  }

  if (this->FrameROIZ)
  {
    this->FrameROIZ->Delete();
    this->FrameROIZ = NULL;
  }
  if (this->ROIX)
  {
    this->ROIX->Delete();
    this->ROIX = NULL;
  }

  if (this->ROIY)
  {
    this->ROIY->Delete();
    this->ROIY = NULL;
  }

  if (this->ROIZ)
  {
    this->ROIZ->Delete();
    this->ROIZ = NULL;
  }

  if (this->LabelROIX)
  {
    this->LabelROIX->Delete();
    this->LabelROIX = NULL;
  }

  if (this->LabelROIY)
  {
    this->LabelROIY->Delete();
    this->LabelROIY = NULL;
  }

  if (this->LabelROIZ)
  {
    this->LabelROIZ->Delete();
    this->LabelROIZ = NULL;
  }

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove();
}

void vtkTumorGrowthROIStep::DeleteSuperSampleNode() 
{
  this->GetGUI()->GetLogic()->DeleteSuperSample(1);
} 

//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::ShowUserInterface()
{
  // ----------------------------------------
  // Display Scan1, Delete Super Sampled and Grid  
  // ----------------------------------------
  this->DeleteSuperSampleNode();

  vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
  int dimensions[3]={1,1,1};
  if (node) {
    vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_Ref()));
    if (volumeNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeNode->GetID());
      applicationLogic->PropagateVolumeSelection();
      memcpy(dimensions,volumeNode->GetImageData()->GetDimensions(),sizeof(int)*3);
      // Load File 
      char fileName[1024];
      sprintf(fileName,"%s/TG_Analysis_Intensity.nhdr",node->GetWorkingDir());
      
      // vtkMRMLVolumeNode* tmp =  this->GetGUI()->GetLogic()->LoadVolume(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()),fileName,1,"TG_analysis");
    } 
  } else {
    cout << "no node "  << endl;
  }
  this->GridRemove();

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------

  this->vtkTumorGrowthStep::ShowUserInterface();

  // Create the frame
  // Needs to be check bc otherwise with wizrd can be created over again

  this->Frame->SetLabelText("Define VOI");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());

  if (!this->FrameButtons)
    {
    this->FrameButtons = vtkKWFrame::New();
    }
  if (!this->FrameButtons->IsCreated())
    {
      this->FrameButtons->SetParent(this->Frame->GetFrame());
    this->FrameButtons->Create();
    // this->FrameButtons->SetLabelText("");
    // define buttons 
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameButtons->GetWidgetName());

  if (!this->FrameBlank)
    {
    this->FrameBlank = vtkKWFrame::New();
    }
  if (!this->FrameBlank->IsCreated())
    {
      this->FrameBlank->SetParent(this->Frame->GetFrame());
    this->FrameBlank->Create();
    // this->FrameButtons->SetLabelText("");
    // define buttons 
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 4", this->FrameBlank->GetWidgetName());

  if (!this->FrameROI)
    {
    this->FrameROI = vtkSlicerModuleCollapsibleFrame::New();
    }
  if (!this->FrameROI->IsCreated())
    {
      this->FrameROI->SetParent(this->Frame->GetFrame());
    this->FrameROI->Create();
    this->FrameROI->SetLabelText("Manual");
    // this->FrameROI->CollapseFrame();
  }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameROI->GetWidgetName());

  if (!this->ButtonsShow) {
    this->ButtonsShow = vtkKWPushButton::New();
  }

  if (!this->ButtonsShow->IsCreated()) {
    this->ButtonsShow->SetParent(this->FrameButtons);
    this->ButtonsShow->Create();
    this->ButtonsShow->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH);
    this->ButtonsShow->SetText("Show VOI");
    this->ButtonsShow->SetBalloonHelpString("Show/hide ROI in image viewer"); 
  }

  if (!this->ButtonsReset) {
    this->ButtonsReset = vtkKWPushButton::New();
  }
  if (!this->ButtonsReset->IsCreated()) {
    this->ButtonsReset->SetParent(this->FrameButtons);
    this->ButtonsReset->Create();
    this->ButtonsReset->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH);
    this->ButtonsReset->SetText("Reset");
    this->ButtonsReset->SetBalloonHelpString("Reset Values"); 
  }

  this->Script("pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
                this->ButtonsShow->GetWidgetName(),this->ButtonsReset->GetWidgetName());

  if (!this->FrameROIX)
    {
    this->FrameROIX = vtkKWFrame::New();
    }
  if (!this->FrameROIX->IsCreated())
    {
      this->FrameROIX->SetParent(this->FrameROI->GetFrame());
    this->FrameROIX->Create();
  }

  if (!this->LabelROIX)
    {
    this->LabelROIX = vtkKWLabel::New();
    }
  if (!this->LabelROIX->IsCreated())
    {

    this->LabelROIX->SetParent(this->FrameROIX);
    this->LabelROIX->Create();
    this->LabelROIX->SetText("X:");

    }
  if (!this->ROIX)
    {
    this->ROIX = vtkKWRange::New();
    }
  if (!this->ROIX->IsCreated())
    {

    this->ROIX->SetParent(this->FrameROIX);
    this->ROIX->Create();
    this->ROIX->SymmetricalInteractionOff();
    this->ROIX->SetCommand(this, "ROIXChangedCallback");    
    this->ROIX->SetWholeRange(-1, double(dimensions[0]-1));
    this->ROIX->SetResolution(1);
    }

  if (!this->FrameROIY)
    {
    this->FrameROIY = vtkKWFrame::New();
    }
  if (!this->FrameROIY->IsCreated())
    {
      this->FrameROIY->SetParent(this->FrameROI->GetFrame());
    this->FrameROIY->Create();
  }

  if (!this->LabelROIY)
    {
    this->LabelROIY = vtkKWLabel::New();
    }
  if (!this->LabelROIY->IsCreated())
    {

    this->LabelROIY->SetParent(this->FrameROIY);
    this->LabelROIY->Create();
    this->LabelROIY->SetText("Y:");

    }

 if (!this->ROIY)
    {
    this->ROIY = vtkKWRange::New();
    }
  if (!this->ROIY->IsCreated())
    {

    this->ROIY->SetParent(this->FrameROIY);

    this->ROIY->Create();
    this->ROIY->SymmetricalInteractionOff();
    this->ROIY->SetCommand(this, "ROIYChangedCallback");    
    this->ROIY->SetWholeRange(-1, double(dimensions[1]-1));
    this->ROIY->SetResolution(1);
    }

  if (!this->FrameROIZ)
    {
    this->FrameROIZ = vtkKWFrame::New();
    }
  if (!this->FrameROIZ->IsCreated())
    {
      this->FrameROIZ->SetParent(this->FrameROI->GetFrame());
    this->FrameROIZ->Create();
  }

  if (!this->LabelROIZ)
    {
    this->LabelROIZ = vtkKWLabel::New();
    }
  if (!this->LabelROIZ->IsCreated())
    {

    this->LabelROIZ->SetParent(this->FrameROIZ);
    this->LabelROIZ->Create();
    this->LabelROIZ->SetText("Z:");

    }

 if (!this->ROIZ)
    {
    this->ROIZ = vtkKWRange::New();
    }
  if (!this->ROIZ->IsCreated())
    {

    this->ROIZ->SetParent(this->FrameROIZ);

    this->ROIZ->Create();
    this->ROIZ->SymmetricalInteractionOff();
    this->ROIZ->SetCommand(this, "ROIZChangedCallback");    
    this->ROIZ->SetWholeRange(-1, double(dimensions[2]-1));
    this->ROIZ->SetResolution(1);
    }


  this->Script("pack %s %s %s -side top -anchor nw -padx 0 -pady 3",this->FrameROIX->GetWidgetName(),this->FrameROIY->GetWidgetName(),this->FrameROIZ->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIX->GetWidgetName(),this->ROIX->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIY->GetWidgetName(),this->ROIY->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIZ->GetWidgetName(),this->ROIZ->GetWidgetName());
 
  // Set it up so it has default value from MRML file 
  this->ROIUpdateWithNode();
  {
   vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget(); 
   wizard_widget->BackButtonVisibilityOn();
   wizard_widget->GetCancelButton()->EnabledOn();
  }
  // Very Important 
  this->AddGUIObservers();
  // Keep seperate bc GUIObserver is also called from vtkTumorGrowthGUI ! 
  // You only want to add the observers below when the step is active 
  this->AddROISamplingGUIObservers();

  // this->TransitionCallback();
}

//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::ROIXChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(0,min, max);
}


//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::ROIYChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(1,min, max);
}

//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::ROIZChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(2,min, max);
}  

//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::ROIChangedCallback(int axis, double min, double max)  
{
  vtkMRMLTumorGrowthNode *mrmlNode = this->GetGUI()->GetNode();
  if (!mrmlNode) return;

  mrmlNode->SetROIMin(axis,int(min));  
  mrmlNode->SetROIMax(axis,int(max));  
  this->ROIMapUpdate();

}



//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::AddGUIObservers() 
{
  // cout << "vtkTumorGrowthROIStep::AddGUIObservers()" << endl; 
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->ButtonsShow && (!this->ButtonsShow->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsShow->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 

  if (this->ButtonsReset && (!this->ButtonsReset->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsReset->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkTumorGrowthROIStep::AddROISamplingGUIObservers() {
  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI0()->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand);

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI1()->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent,this->WizardGUICallbackCommand);

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI2()->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand);
} 


void vtkTumorGrowthROIStep::RemoveGUIObservers() 
{
  if (this->ButtonsShow) 
    {
      this->ButtonsShow->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
    }

  if (this->ButtonsReset) 
  {
      this->ButtonsReset->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  }
  this->RemoveROISamplingGUIObservers();
}


void vtkTumorGrowthROIStep::RemoveROISamplingGUIObservers() {
  if (!this->GetGUI()) return;
  vtkSlicerApplicationGUI *ApplicationGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI());
  if (!ApplicationGUI) return; 
  for (int i = 0 ; i < 3 ; i ++ ) {
    vtkSlicerSliceGUI *MainGUI = NULL;
    if (i == 0) MainGUI = ApplicationGUI->GetMainSliceGUI0();
    if (i == 1) MainGUI = ApplicationGUI->GetMainSliceGUI1();
    if (i == 2) MainGUI = ApplicationGUI->GetMainSliceGUI2();
    if (!MainGUI) return;
    vtkRenderWindowInteractor *rwi = MainGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    rwi->GetInteractorStyle()->RemoveObservers(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand);
  }
}

void vtkTumorGrowthROIStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkTumorGrowthROIStep *self = reinterpret_cast<vtkTumorGrowthROIStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}

void vtkTumorGrowthROIStep::ROIReset() {
  // cout << "ROIReset Start" << endl;
  if (this->ROIX) this->ROIX->SetRange(-1,-1);
  if (this->ROIY) this->ROIY->SetRange(-1,-1);
  if (this->ROIZ) this->ROIZ->SetRange(-1,-1);
}


void vtkTumorGrowthROIStep::ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample) {
  if (!ROIAxis) return;
  double *oldRange = ROIAxis->GetRange();
  double newRange[2];

  if ((Sample < oldRange[0]) || (oldRange[0] < 0)) newRange[0] = Sample;
  else  newRange[0] = oldRange[0]; 
  if ((Sample > oldRange[1]) || (oldRange[1] < 0)) newRange[1] = Sample;
  else newRange[1] = oldRange[1]; 
  ROIAxis->SetRange(newRange);
}

void vtkTumorGrowthROIStep::ROIUpdateWithNewSample(int ijkSample[3]) {
  // cout << "ROIUpdateWithNewSample start " << ijkSample[0] << " " << ijkSample[1] << " " << ijkSample[2] << " " << endl;
  this->ROIUpdateAxisWithNewSample(this->ROIX,ijkSample[0]);
  this->ROIUpdateAxisWithNewSample(this->ROIY,ijkSample[1]);
  this->ROIUpdateAxisWithNewSample(this->ROIZ,ijkSample[2]);
}

void vtkTumorGrowthROIStep::ROIUpdateAxisWithNode(vtkMRMLTumorGrowthNode* Node, vtkKWRange *ROIAxis, int Axis) {
  if (!Node || !ROIAxis) return;
  ROIAxis->SetRange(Node->GetROIMin(Axis),Node->GetROIMax(Axis));
}

void vtkTumorGrowthROIStep::ROIUpdateWithNode() {
  // cout << "ROIUpdateWithNode Start" << endl;
  vtkMRMLTumorGrowthNode* Node = this->GetGUI()->GetNode();
  this->ROIUpdateAxisWithNode(Node, this->ROIX,0); 
  this->ROIUpdateAxisWithNode(Node, this->ROIY,1); 
  this->ROIUpdateAxisWithNode(Node, this->ROIZ,2); 
  this->ROIMapUpdate();
}



// Return 1 if it is a valid ROI and zero otherwise
int vtkTumorGrowthROIStep::ROICheck() {
  // Define Variables
  vtkMRMLTumorGrowthNode* Node = this->GetGUI()->GetNode();
  if (!Node) return 0;

  vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
  if (!volumeNode) return 0;
  return this->GetGUI()->GetLogic()->CheckROI(volumeNode);
}

void vtkTumorGrowthROIStep::ROIMapUpdate() {

  vtkMRMLTumorGrowthNode* Node      =  this->GetGUI()->GetNode();
  if (!this->ROILabelMapNode || !this->ROILabelMap || !Node || !this->ROICheck()) return;

  int size[3]   = {Node->GetROIMax(0) - Node->GetROIMin(0) + 1, Node->GetROIMax(1) - Node->GetROIMin(1) + 1, Node->GetROIMax(2) - Node->GetROIMin(2) + 1};
  int center[3] = {(Node->GetROIMax(0) + Node->GetROIMin(0))/2 ,(Node->GetROIMax(1) + Node->GetROIMin(1))/2, (Node->GetROIMax(2) + Node->GetROIMin(2))/2};
  this->ROILabelMap->SetCenter(center);
  this->ROILabelMap->SetSize(size);
  this->ROILabelMap->Update();
  this->ROILabelMapNode->Modified();
}


int vtkTumorGrowthROIStep::ROIMapShow() {
  // -----
  // Initialize
  if (!this->ROICheck()) {
    vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),"Tumor Growth", "Please define VOI correctly before pressing button", vtkKWMessageDialog::ErrorIcon);
    return 0;
  }

  vtkMRMLTumorGrowthNode* Node      =  this->GetGUI()->GetNode();
  if (!Node) return 0;
  vtkMRMLScene* mrmlScene           =  Node->GetScene();
  vtkMRMLNode* mrmlFristScanRefNode =  mrmlScene->GetNodeByID(Node->GetScan1_Ref());
  vtkMRMLVolumeNode* volumeNode     =  vtkMRMLVolumeNode::SafeDownCast(mrmlFristScanRefNode);
  if (!volumeNode) return 0;
  int* dimensions = volumeNode->GetImageData()->GetDimensions();

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove(); 

  // -----
  // Define LabelMap 
  this->ROILabelMap =  vtkImageRectangularSource::New();
  this->ROILabelMap->SetWholeExtent(0,dimensions[0] -1,0,dimensions[1] -1, 0,dimensions[2] -1); 
  this->ROILabelMap->SetOutputScalarTypeToShort();
  this->ROILabelMap->SetInsideGraySlopeFlag(0); 
  this->ROILabelMap->SetInValue(17);
  this->ROILabelMap->SetOutValue(0);
  this->ROILabelMap->Update();

  // Show map in Slicer 3 
  //  set scene [[$this GetLogic] GetMRMLScene]
  //  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
  vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();

  vtkSlicerVolumesGUI  *volumesGUI    = vtkSlicerVolumesGUI::SafeDownCast(application->GetModuleGUIByName("Volumes")); 
  vtkSlicerVolumesLogic *volumesLogic = volumesGUI->GetLogic();
  // set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]
  this->ROILabelMapNode = volumesLogic->CreateLabelVolume(mrmlScene,volumeNode, "TG_ROI");
  this->ROILabelMapNode->SetAndObserveImageData(this->ROILabelMap->GetOutput());

  // Now show in foreground 
  //  make the source node the active background, and the label node the active label
  // set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  //$selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  //$selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID] 
  //  applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(volumeNode->GetID());
 
  // Reset to original slice location 
  double oldSliceSetting[3];
  oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetOffsetScale()->GetValue());
  oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetOffsetScale()->GetValue());
  oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetOffsetScale()->GetValue());

  //applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetBackgroundSelector()->SetSelected(volumeNode);
  applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetForegroundSelector()->SetSelected(this->ROILabelMapNode);

  //applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetBackgroundSelector()->SetSelected(volumeNode);
  applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetForegroundSelector()->SetSelected(this->ROILabelMapNode);

  //applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetBackgroundSelector()->SetSelected(volumeNode);
  applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetForegroundSelector()->SetSelected(this->ROILabelMapNode);

  applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);
  applicationLogic->PropagateVolumeSelection();

  // Reset to original slice location 
  applicationGUI->GetMainSliceGUI0()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[0]);
  applicationGUI->GetMainSliceGUI1()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[1]);
  applicationGUI->GetMainSliceGUI2()->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[2]);

  this->ROIMapUpdate();

  return 1;
}

void vtkTumorGrowthROIStep::ROIMapRemove() {
  
  if (this->ROILabelMapNode && this->GetGUI()) { 
    this->GetGUI()->GetMRMLScene()->RemoveNode(this->ROILabelMapNode);
  }
  this->ROILabelMapNode = NULL;

  if (this->ROILabelMap) { 
    this->ROILabelMap->Delete();
    this->ROILabelMap = NULL;
  }

  // Needs to be done otherwise when going backwards field is not correctly defined   
  if (this->ButtonsShow && this->ButtonsShow->IsCreated())  {
    this->ButtonsShow->SetText("Show VOI");
  }
}




void vtkTumorGrowthROIStep::RetrieveInteractorIJKCoordinates(vtkSlicerSliceGUI *sliceGUI, vtkRenderWindowInteractor *rwi,int coords[3]) {

  coords[0] = coords[1] = coords[2] = -1;
  vtkMRMLTumorGrowthNode* Node = this->GetGUI()->GetNode();
  if (!Node) {
    cout << "ERROR: vtkTumorGrowthROIStep::RetrieveInteractorIJKCoordinates: No Node" << endl;
    return;
  } 

  if (!Node->GetScan1_Ref()) {
    cout << "ERROR: vtkTumorGrowthROIStep::RetrieveInteractorIJKCoordinates: No First Volume Defined" << endl;
    return;
  }
  vtkMRMLNode* mrmlNode =   Node->GetScene()->GetNodeByID(Node->GetScan1_Ref());
  vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(mrmlNode);

  if (!volumeNode)
    {
      cout << "ERROR: vtkTumorGrowthROIStep::RetrieveInteractorIJKCoordinates: No Scan1_Ref" << endl;
      return;
    }

  // --------------------------------------------------------------
  // Compute RAS coordinates
   int point[2];
   rwi->GetLastEventPosition(point);
   double inPt[4] = {point[0], point[1], 0, 1};
   double rasPt[4];
   vtkMatrix4x4 *matrix = sliceGUI->GetLogic()->GetSliceNode()->GetXYToRAS();
   matrix->MultiplyPoint(inPt, rasPt); 

  // --------------------------------------------------------------
  // Compute IJK coordinates
  double ijkPt[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPt, ijkPt);
  rasToijk->Delete();

  // --------------------------------------------------------------
  // Check validity of coordinates
  int* dimensions = volumeNode->GetImageData()->GetDimensions();
  for (int i = 0 ; i < 3 ; i++) {
    if (ijkPt[i] < 0 ) ijkPt[i] = 0;
    else if (ijkPt[i] >=  dimensions[i] ) ijkPt[i] = dimensions[i] -1;    
  }
  coords[0] = int(round(ijkPt[0]));  coords[1] = int(round(ijkPt[1])); coords[2] = int(round(ijkPt[2])); 

  //cout << "Sample:  " << rasPt[0] << " " <<  rasPt[1] << " " << rasPt[2] << " " << rasPt[3] << endl;
  //cout << "Coord: " << coords[0] << " " << coords[1] << " " << coords[2] << " " << coords[3] << endl;
  //cout << "Dimen: " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " " <<  endl;

}
void vtkTumorGrowthROIStep::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {

  if (event == vtkKWPushButton::InvokedEvent) {
    vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
    if (this->ButtonsShow && (button == this->ButtonsShow)) 
    { 
      if (this->ROILabelMapNode) {
        this->ButtonsShow->SetText("Show VOI");
        this->ROIMapRemove();
      } else { 
        if (this->ROIMapShow()) { 
          this->ButtonsShow->SetText("Hide ROI");
        }
      }
    }
    if (this->ButtonsReset && (button == this->ButtonsReset)) 
    { 
      this->ROIReset();
    }
    return;
  }

  vtkSlicerInteractorStyle *s = vtkSlicerInteractorStyle::SafeDownCast(caller);
  if (s && event == vtkCommand::LeftButtonPressEvent)
  {
    // Retrieve Coordinates and update ROI
    int index = 0; 
    vtkSlicerSliceGUI *sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI0();

    vtkRenderWindowInteractor *rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    while (index < 2 && (s != rwi->GetInteractorStyle())) {
        index ++;
        if (index == 1) {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI1();
        } else {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI2();
        }
        rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    }
    int ijkCoords[3];
    this->RetrieveInteractorIJKCoordinates(sliceGUI, rwi, ijkCoords);
    this->ROIUpdateWithNewSample(ijkCoords);

  }    
  // Define SHOW Button 
}



//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::TransitionCallback() 
{
  // cout << "vtkTumorGrowthROIStep::TransitionCallback() Start" << endl; 
  if (this->ROICheck()) { 
     // ----------------------------
     // Create SuperSampledVolume 
    vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
    vtkMRMLScalarVolumeNode *outputNode = this->GetGUI()->GetLogic()->CreateSuperSample(1);
    this->GetGUI()->GetLogic()->SaveVolume(application,outputNode); 

    if (outputNode) {
       // Prepare to update mrml node with results 
       vtkMRMLTumorGrowthNode* Node = this->GetGUI()->GetNode();
       if (!Node) return;
              
       // Delete old attached node first 
       this->GetGUI()->GetLogic()->DeleteSuperSample(1);

       // Update node 
       
       Node->SetScan1_SuperSampleRef(outputNode->GetID());
       //cout << "==============================" << endl;
       //cout << "vtkTumorGrowthROIStep::TransitionCallback " << Node->GetScan1_SuperSampleRef() << " " <<  Node->GetScan1_Ref() << endl;
       //cout << "==============================" << endl;

       // Remove blue ROI screen 
       this->ROIMapRemove();
       
       this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
     } else {
       vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),"Tumor Growth", "Could not proceed to next step - scan1 might have disappeared", vtkKWMessageDialog::ErrorIcon); 
     }
     // ---------------------------------
   } else {     
     vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),"Tumor Growth", "Please define ROI correctly before proceeding", vtkKWMessageDialog::ErrorIcon);
   }
}


//----------------------------------------------------------------------------
void  vtkTumorGrowthROIStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveROISamplingGUIObservers();
}

//----------------------------------------------------------------------------
void vtkTumorGrowthROIStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
