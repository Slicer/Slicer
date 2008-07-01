#include "vtkTumorGrowthAnalysisStep.h"

#include "vtkTumorGrowthGUI.h"
#include "vtkMRMLTumorGrowthNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWThumbWheel.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWScale.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkTumorGrowthLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkSlicerApplication.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkImageAppend.h"
#include "vtkImageConstantPad.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTumorGrowthAnalysisStep);
vtkCxxRevisionMacro(vtkTumorGrowthAnalysisStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkTumorGrowthAnalysisStep::vtkTumorGrowthAnalysisStep()
{
  this->SetName("Analysis"); 
  this->SetDescription("Analysis of Tumor Growth"); 
  this->WizardGUICallbackCommand->SetCallback(vtkTumorGrowthAnalysisStep::WizardGUICallback);

  this->GrowthLabel = NULL;

  this->ButtonsWorkingDir = NULL;
  this->ButtonsAnalysis = NULL;
  this->ButtonsSave = NULL;
  this->FrameButtons = NULL;
  this->FrameButtonsFunctions = NULL;
  this->ButtonsSnapshot = NULL;
  this->ButtonsInfo = NULL;
  this->SnapshotCount = 0;

  this->FrameDeformable     = NULL;
  this->FrameDeformableCol1 = NULL;
  this->FrameDeformableCol2 = NULL;
  this->DeformableTextLabel = NULL;
  this->DeformableMeassureLabel = NULL;

  this->SensitivityRadio = NULL;
  this->SensitivityLow = NULL;
  this->SensitivityMedium = NULL;
  this->SensitivityHigh = NULL;


}

//----------------------------------------------------------------------------
vtkTumorGrowthAnalysisStep::~vtkTumorGrowthAnalysisStep()
{

  if (this->ButtonsWorkingDir) 
    {
      this->ButtonsWorkingDir->Delete();
      this->ButtonsWorkingDir = NULL;
    }

  if (this->ButtonsAnalysis) 
    {
      this->ButtonsAnalysis->Delete();
      this->ButtonsAnalysis = NULL;

    }

  if (this->ButtonsSave)
    {
    this->ButtonsSave->Delete();
    this->ButtonsSave  = NULL;
    }

  if (this->ButtonsSnapshot)
    {
    this->ButtonsSnapshot->Delete();
    this->ButtonsSnapshot  = NULL;
    }

  if (this->FrameButtons)
    {
    this->FrameButtons->Delete();
    this->FrameButtons  = NULL;
    }

  if (this->FrameButtonsFunctions)
    {
    this->FrameButtonsFunctions->Delete();
    this->FrameButtonsFunctions  = NULL;
    }

  if (this->ButtonsInfo)
    {
    this->ButtonsInfo->Delete();
    this->ButtonsInfo  = NULL;
    }
  
  if (this->SensitivityRadio)
    {
      this->SensitivityRadio->Delete();
      this->SensitivityRadio= NULL;
    }

  if (this->SensitivityLow)
    {
      this->SensitivityLow->Delete();
      this->SensitivityLow= NULL;
    }

  if (this->SensitivityMedium)
    {
      this->SensitivityMedium->Delete();
      this->SensitivityMedium= NULL;
    }

  if (this->SensitivityHigh)
    {
      this->SensitivityHigh->Delete();
      this->SensitivityHigh= NULL;
    }

  if (this->GrowthLabel) 
    {
      this->GrowthLabel->Delete();
      this->GrowthLabel = NULL;
    }

 if (this->FrameDeformable) 
    {
      this->FrameDeformable->Delete();
      this->FrameDeformable = NULL;
    }

 if (this->FrameDeformableCol1) 
    {
      this->FrameDeformableCol1->Delete();
      this->FrameDeformableCol1 = NULL;
    }

 if (this->FrameDeformableCol2) 
    {
      this->FrameDeformableCol2->Delete();
      this->FrameDeformableCol2 = NULL;
    }

 if (this->DeformableTextLabel) 
    {
      this->DeformableTextLabel->Delete();
      this->DeformableTextLabel = NULL;
    }

 if (this->DeformableMeassureLabel) 
    {
      this->DeformableMeassureLabel->Delete();
      this->DeformableMeassureLabel = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::AddGUIObservers() 
{
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  //if (this->ButtonsSnapshot && (!this->ButtonsSnapshot->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
  //  {
  //    this->ButtonsSnapshot->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
  //  } 

  if (this->ButtonsSave && (!this->ButtonsSave->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsSave->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    }

  //if (this->ButtonsWorkingDir && (!this->ButtonsWorkingDir->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
  //  {
  //    this->ButtonsWorkingDir->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
  //  } 


  if (this->ButtonsAnalysis && (!this->ButtonsAnalysis->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsAnalysis->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 


  if (this->SensitivityMedium && !this->SensitivityMedium->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))
    {
      this->SensitivityMedium->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }
  if (this->SensitivityHigh && !this->SensitivityHigh->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))
    {
      this->SensitivityHigh->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }
  if (this->SensitivityLow && !this->SensitivityLow->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))
    {
      this->SensitivityLow->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }
}

void vtkTumorGrowthAnalysisStep::RemoveGUIObservers() 
{
  // cout << "vtkTumorGrowthAnalysisStep::RemoveGUIObservers" << endl;
  //if (this->ButtonsSnapshot) 
  //  {
  //    this->ButtonsSnapshot->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  //  }

  if (this->ButtonsSave) 
  {
      this->ButtonsSave->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  }

  //if (this->ButtonsWorkingDir) 
  //{
  //    this->ButtonsWorkingDir->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  //}


  if (this->ButtonsAnalysis) 
  {
      this->ButtonsAnalysis->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  }

  if (this->SensitivityLow)
    {
      //  this->SensitivityLow->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
      this->SensitivityLow->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }

  if (this->SensitivityMedium)
    {
      this->SensitivityMedium->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }
  if (this->SensitivityHigh)
    {
      this->SensitivityHigh->RemoveObservers( vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);
    }
}

void vtkTumorGrowthAnalysisStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
  // cout << "void vtkTumorGrowthAnalysisStep::WizardGUICallback" << endl;
    vtkTumorGrowthAnalysisStep *self = reinterpret_cast<vtkTumorGrowthAnalysisStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}


void vtkTumorGrowthAnalysisStep::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {

  // cout << "vtkTumorGrowthAnalysisStep::ProcessGUIEvents" << endl;

  if (event != vtkKWPushButton::InvokedEvent) return;

  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  //if (this->ButtonsSnapshot && (button == this->ButtonsSnapshot)) 
  //{ 
  //    this->TakeScreenshot(); 
  //    return;
  //}

  if ((this->ButtonsAnalysis && (button == this->ButtonsAnalysis)) || 
      (this->ButtonsSave && (button == this->ButtonsSave)))
  { 
     vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
     if (node) {
        // Define MRML file 
        node->GetScene()->SetRootDirectory(node->GetWorkingDir());
        std::string fileName(node->GetWorkingDir());
        fileName.append("/Data.mrml");
        node->GetScene()->SetURL(fileName.c_str());
   
        if  (button == this->ButtonsAnalysis) {
          // Depends on analysis
          // Save Data 
          vtkMRMLVolumeNode *volumeAnalysisNode = NULL;
          if (node->GetAnalysis_Intensity_Flag()) { 
            volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Intensity_Ref()));
        if (volumeAnalysisNode) {
          vtkTumorGrowthLogic *Logic = this->GetGUI()->GetLogic();
          Logic->SaveVolumeForce(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()),volumeAnalysisNode);
        }   
          }

      if (node->GetAnalysis_Deformable_Flag()) { 
            volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Deformable_Ref()));
        if (volumeAnalysisNode) {
          vtkTumorGrowthLogic *Logic = this->GetGUI()->GetLogic();
          Logic->SaveVolumeForce(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()),volumeAnalysisNode);
        } 
          }
          // Saves file  
          node->GetScene()->Commit();

          //std::string infoMsg("Saved Data to ");
          //infoMsg.append(node->GetWorkingDir());

          //vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),
          //                               "Tumor Growth",infoMsg.c_str(), vtkKWMessageDialog::OkDefault);
        } else {


           this->GetGUI()->GetApplicationGUI()->ProcessSaveSceneAsCommand();
        }
      } else {
        this->GetGUI()->GetApplicationGUI()->ProcessSaveSceneAsCommand();
        node = this->GetGUI()->GetNode();
      }
   
      // Save Results to file 
      if (node) { 
        std::string fileName(node->GetWorkingDir());
        fileName.append("/AnalysisOutcome.log");
        std::ofstream outfile(fileName.c_str());
        if (outfile.fail()) {
           cout << "Error: vtkTumorGrowthAnalysisStep::ProcessGUIEvents: Cannot write to file " << fileName.c_str() << endl;
    } else {     
          this->GetGUI()->GetLogic()->PrintResult(outfile, vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()));
          cout << "Wrote outcome of analysis to " << fileName.c_str() << endl;
        }
      }
      return;
  }

  //  if (event == vtkKWPushButton::InvokedEvent ) {
    vtkKWCheckButton *checkButton = vtkKWCheckButton::SafeDownCast(caller);
    if (checkButton ==  this->SensitivityLow) {
      this->SensitivityChangedCallback(1);
      return;
    } 
    if (checkButton ==  this->SensitivityMedium) {
      this->SensitivityChangedCallback(2);
      return;
    }
    if (checkButton ==  this->SensitivityHigh) {
      this->SensitivityChangedCallback(3);
      return;
    }
    // }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::ShowUserInterface()
{
  // ----------------------------------------
  // Display Analysis Volume 
  // ----------------------------------------  
  vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
  if (node) { 
    vtkMRMLVolumeNode *volumeSampleNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SuperSampleRef()));
    vtkMRMLVolumeNode *volumeAnalysisNode = NULL;
    float colorMin[3] = {0, 0.0, 0.8 };
    float colorMax[3] = {0.0 , 0.0, 0.8 };

    if (node->GetAnalysis_Intensity_Flag()) {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Intensity_Ref()));
      this->CreateRender(volumeAnalysisNode, 1);
      this->SetRender_BandPassFilter(12, 14, colorMax, colorMax);

    } else if (node->GetAnalysis_Deformable_Flag()) {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Deformable_Ref()));
      // this->CreateRender(volumeAnalysisNode);
      // this->SetRender_HighPassFilter(10);
      this->CreateRender(volumeAnalysisNode, 1);
      this->SetRender_HighPassFilter(12,colorMax, colorMax);
    } else {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan2_LocalRef()));
    }

    if (volumeSampleNode && volumeAnalysisNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeSampleNode->GetID());

      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();

      double oldSliceSetting[3];
      oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetValue());
      oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->GetValue());
      oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->GetValue());

      applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeAnalysisNode);
      applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeAnalysisNode);
      applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetForegroundSelector()->SetSelected(volumeAnalysisNode);
      applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.6);

      applicationLogic->PropagateVolumeSelection();

      // Return to original slice position 
      applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[0]);
      applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[1]);
      applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->SetValue(oldSliceSetting[2]);
    } 
  }

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------

  this->vtkTumorGrowthStep::ShowUserInterface();

  this->Frame->SetLabelText("Intensity Pattern Analysis");
  if (node->GetAnalysis_Intensity_Flag()) {
    this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());
  }

  if (!this->SensitivityRadio)
    {
      this->SensitivityRadio = vtkKWFrame::New();
    }

  if (!this->SensitivityRadio->IsCreated())
  {
      this->SensitivityRadio->SetParent(this->Frame->GetFrame());
      this->SensitivityRadio->Create();
  }

  this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->SensitivityRadio->GetWidgetName());

  if (!this->SensitivityLow) {
    this->SensitivityLow = vtkKWCheckButton::New();
  } 

  if (!this->SensitivityLow->IsCreated()) {
    this->SensitivityLow->SetParent(this->SensitivityRadio);
    this->SensitivityLow->Create();
    this->SensitivityLow->SelectedStateOff();
    this->SensitivityLow->SetText("Sensitive");
    this->SensitivityLow->SetBalloonHelpString("Perform a very sensitive intensity analysis ");
  }

  if (!this->SensitivityMedium) {
    this->SensitivityMedium = vtkKWCheckButton::New();
  } 

  if (!this->SensitivityMedium->IsCreated()) {
    this->SensitivityMedium->SetParent(this->SensitivityRadio);
    this->SensitivityMedium->Create();
    this->SensitivityMedium->SelectedStateOff();
    this->SensitivityMedium->SetText("Moderate");
    this->SensitivityMedium->SetBalloonHelpString("Perform a moderate intensity analysis ");
  }

  if (!this->SensitivityHigh) {
    this->SensitivityHigh = vtkKWCheckButton::New();
  } 

  if (!this->SensitivityHigh->IsCreated()) {
    this->SensitivityHigh->SetParent(this->SensitivityRadio);
    this->SensitivityHigh->Create();
    this->SensitivityHigh->SelectedStateOff();
    this->SensitivityHigh->SetText("Robust");
    this->SensitivityHigh->SetBalloonHelpString("Perform a very robust intensity analysis ");
  }

  this->Script("pack %s %s %s -side left -anchor nw -fill x -padx 0 -pady 0", this->SensitivityLow->GetWidgetName(), this->SensitivityMedium->GetWidgetName(), this->SensitivityHigh->GetWidgetName());


  // if (!this->SensitivityRadio)
  //   {
  //   this->SensitivityRadio = vtkKWThumbWheel::New();
  //   }
  // 
  // if (!this->SensitivityScale->IsCreated())
  // {
  //   this->SensitivityScale->SetParent(this->Frame->GetFrame());
  //   this->SensitivityScale->Create();
  //   this->SensitivityScale->SetRange(0.0,1.0);
  //   this->SensitivityScale->SetMinimumValue(0.0);
  //   this->SensitivityScale->ClampMinimumValueOn(); 
  //   this->SensitivityScale->SetMaximumValue(1.0);
  //   this->SensitivityScale->ClampMaximumValueOn(); 
  //   this->SensitivityScale->SetResolution(0.75);
  //   this->SensitivityScale->SetLinearThreshold(1);
  //   this->SensitivityScale->SetThumbWheelSize (TUMORGROWTH_WIDGETS_SLIDER_WIDTH,TUMORGROWTH_WIDGETS_SLIDER_HEIGHT);
  //   this->SensitivityScale->DisplayEntryOn();
  //   this->SensitivityScale->DisplayLabelOn();
  //   this->SensitivityScale->GetLabel()->SetText("Sensitivity");
  //   this->SensitivityScale->SetCommand(this,"SensitivityChangedCallback");
  //   this->SensitivitySale->DisplayEntryAndLabelOnTopOff(); 
  //   this->SensitivityScale->SetBalloonHelpString("The further the wheel is turned to the right the more robust the result");
  // 
  //   // this->SensitivityScale->GetEntry()->SetCommandTriggerToAnyChange();
  // }

  // Initial value 
  // vtkMRMLTumorGrowthNode *mrmlNode = this->GetGUI()->GetNode();
  // if (mrmlNode) this->SensitivityScale->SetValue(mrmlNode->GetAnalysis_Intensity_Sensitivity());
  // this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->SensitivityScale->GetWidgetName());

  if (!this->GrowthLabel)
    {
    this->GrowthLabel = vtkKWLabel::New();
    }
  if (!this->GrowthLabel->IsCreated())
  {
    this->GrowthLabel->SetParent(this->Frame->GetFrame());
    this->GrowthLabel->Create();
  }
  this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->GrowthLabel->GetWidgetName());


  if (!this->FrameDeformable)
  {
    this->FrameDeformable = vtkKWFrameWithLabel::New();
  }
  if (!this->FrameDeformable->IsCreated())
  {
      vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
      this->FrameDeformable->SetParent(wizard_widget->GetClientArea());
      this->FrameDeformable->Create();
      this->FrameDeformable->SetLabelText("Deformable Map");
      this->FrameDeformable->AllowFrameToCollapseOff();
  }
  if (node->GetAnalysis_Deformable_Flag()) {
     this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->FrameDeformable->GetWidgetName());
  }

  if (!this->FrameDeformableCol1)
  {
    this->FrameDeformableCol1 = vtkKWFrame::New();
  }
  if (!this->FrameDeformableCol1->IsCreated())
  {
      this->FrameDeformableCol1->SetParent(this->FrameDeformable->GetFrame());
      this->FrameDeformableCol1->Create();
  }

  if (!this->FrameDeformableCol2)
  {
    this->FrameDeformableCol2 = vtkKWFrame::New();
  }
  if (!this->FrameDeformableCol2->IsCreated())
  {
      this->FrameDeformableCol2->SetParent(this->FrameDeformable->GetFrame());
      this->FrameDeformableCol2->Create();
  }

  this->Script("pack %s %s -side left -anchor nw -fill x -padx 0 -pady 0", this->FrameDeformableCol1->GetWidgetName(),this->FrameDeformableCol2->GetWidgetName());


  if (!this->DeformableTextLabel)
    {
    this->DeformableTextLabel = vtkKWLabel::New();
    }
  if (!this->DeformableTextLabel->IsCreated())
  {
    this->DeformableTextLabel->SetParent(this->FrameDeformableCol1);
    this->DeformableTextLabel->Create();
  }

  this->DeformableTextLabel->SetText("Segmentation Metric: \nJacobian Metric:");
  this->Script( "pack %s -side left -anchor nw -padx 2 -pady 2", this->DeformableTextLabel->GetWidgetName());


  if (!this->DeformableMeassureLabel)
    {
    this->DeformableMeassureLabel = vtkKWLabel::New();
    }
  if (!this->DeformableMeassureLabel->IsCreated())
  {
    this->DeformableMeassureLabel->SetParent(this->FrameDeformableCol2);
    this->DeformableMeassureLabel->Create();
  }

  {
    char TEXT[1024];
    sprintf(TEXT,"%.3f mm%c (%d Voxels)\n%.3f mm%c (%d Voxels)", 
    node->GetAnalysis_Deformable_SegmentationGrowth(),179,int(node->GetAnalysis_Deformable_SegmentationGrowth()/node->GetScan1_VoxelVolume()), 
    node->GetAnalysis_Deformable_JacobianGrowth(),179,int(node->GetAnalysis_Deformable_JacobianGrowth()/node->GetScan1_VoxelVolume()));
    this->DeformableMeassureLabel->SetText(TEXT);

  }
  this->Script( "pack %s -side left -anchor nw -padx 2 -pady 0", this->DeformableMeassureLabel->GetWidgetName());


  // Define buttons 
  if (!this->FrameButtons)
  {
    this->FrameButtons = vtkKWFrameWithLabel::New();
  }
  if (!this->FrameButtons->IsCreated())
  {
      vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
      this->FrameButtons->SetParent(wizard_widget->GetClientArea());
      this->FrameButtons->Create();
      this->FrameButtons->SetLabelText("Save");
      this->FrameButtons->AllowFrameToCollapseOff();
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->FrameButtons->GetWidgetName());

  if (!this->FrameButtons)
  {
    this->FrameButtons = vtkKWFrameWithLabel::New();
  }
  if (!this->FrameButtons->IsCreated())
  {
      vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
      this->FrameButtons->SetParent(wizard_widget->GetClientArea());
      this->FrameButtons->Create();
      this->FrameButtons->SetLabelText("Save");
      this->FrameButtons->AllowFrameToCollapseOff();
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->FrameButtons->GetWidgetName());

  if (!this->FrameButtonsFunctions)
  {
    this->FrameButtonsFunctions = vtkKWFrame::New();
  }
  if (!this->FrameButtonsFunctions->IsCreated())
  {
      this->FrameButtonsFunctions->SetParent(this->FrameButtons->GetFrame());
      this->FrameButtonsFunctions->Create();
  }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameButtonsFunctions->GetWidgetName());

  if (!this->ButtonsSnapshot) {
    this->ButtonsSnapshot = vtkKWPushButton::New();
   }

  if (!this->ButtonsSnapshot->IsCreated()) {
    this->ButtonsSnapshot->SetParent(this->FrameButtonsFunctions);
    this->ButtonsSnapshot->Create();
    this->ButtonsSnapshot->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH_SMALL);
    this->ButtonsSnapshot->SetText("Screenshot");
    this->ButtonsSnapshot->SetBalloonHelpString("Save a screenshot of the current view in 3D Slicer.");
    this->ButtonsSnapshot->SetCommand(this, "TakeScreenshot");
 
  }

  if (!this->ButtonsAnalysis) {
    this->ButtonsAnalysis = vtkKWPushButton::New();
  }
  if (!this->ButtonsAnalysis->IsCreated()) {
    this->ButtonsAnalysis->SetParent(this->FrameButtonsFunctions);
    this->ButtonsAnalysis->Create();
    this->ButtonsAnalysis->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH_SMALL);
    this->ButtonsAnalysis->SetText("Analysis");
    this->ButtonsAnalysis->SetBalloonHelpString("Save the results of the analysis to a disk."); 
  }

  if (!this->ButtonsSave) {
    this->ButtonsSave = vtkKWPushButton::New();
  }
  if (!this->ButtonsSave->IsCreated()) {
    this->ButtonsSave->SetParent(this->FrameButtonsFunctions);
    this->ButtonsSave->Create();
    this->ButtonsSave->SetWidth(TUMORGROWTH_MENU_BUTTON_WIDTH_SMALL);
    this->ButtonsSave->SetText("Data");
    this->ButtonsSave->SetBalloonHelpString("Save all intermediate results of the analysis to disk."); 
  }

  this->Script("pack %s %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
         this->ButtonsSnapshot->GetWidgetName(),this->ButtonsAnalysis->GetWidgetName(), this->ButtonsSave->GetWidgetName());


  // Look at Qdec Module 
  if (!this->ButtonsInfo)
    {
    this->ButtonsInfo = vtkKWLabel::New();
    }
  if (!this->ButtonsInfo->IsCreated())
  {
    this->ButtonsInfo->SetParent(this->FrameButtons->GetFrame());
    this->ButtonsInfo->Create();
    this->ButtonsInfo->SetText("Results will be saved to directory:");
  }

  if (!this->ButtonsWorkingDir)
    {
    this->ButtonsWorkingDir = vtkKWLoadSaveButtonWithLabel::New();
    }
  if (!this->ButtonsWorkingDir->IsCreated())
  {
    this->ButtonsWorkingDir->SetParent(this->FrameButtons->GetFrame());
    this->ButtonsWorkingDir->Create();
    this->ButtonsWorkingDir->SetLabelText("");
    if (node) {
      // set parth correctly 
      vtksys_stl::string path = node->GetWorkingDir();
      this->ButtonsWorkingDir->GetWidget()->GetLoadSaveDialog()->GenerateLastPath(path.c_str());
      this->ButtonsWorkingDir->GetWidget()->GetLoadSaveDialog()->SetInitialFileName(path.c_str());
      this->ButtonsWorkingDir->GetWidget()->SetText(path.c_str());
    } else {
      this->ButtonsWorkingDir->GetWidget()->SetText("None");
    }
    this->ButtonsWorkingDir->SetBalloonHelpString("Define the working directory in which the results should be saved.");
    this->ButtonsWorkingDir->GetWidget()->SetCommand(this, "SelectDirectoryCallback");

    // this->ButtonsWorkingDir->SetImageToPredefinedIcon(vtkKWIcon::IconFolderOpen);
    this->ButtonsWorkingDir->GetWidget()->TrimPathFromFileNameOn();
    this->ButtonsWorkingDir->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  }

  this->Script( "pack %s %s -side top -anchor nw -padx 2 -pady 2", this->ButtonsInfo->GetWidgetName(), this->ButtonsWorkingDir->GetWidgetName());

  {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    // wizard_widget->GetOKButton()->SetText("Run");
    wizard_widget->GetCancelButton()->SetText("OK"); 
    wizard_widget->GetCancelButton()->SetCommand(this, "ResetPipelineCallback");
    wizard_widget->GetCancelButton()->EnabledOn();
    wizard_widget->OKButtonVisibilityOff();

  }

  this->CreateGridButton();
  this->CreateSliceButton();
  this->AddGUIObservers();
  this->SensitivityChangedCallback(-1);
  this->GetGUI()->PropagateVolumeSelection();
}


//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::SelectDirectoryCallback()
{
  // The template file has changed because of user interaction

  if (this->ButtonsWorkingDir && this->ButtonsWorkingDir->IsCreated())
    {
      if (this->ButtonsWorkingDir->GetWidget()->GetLoadSaveDialog()-> GetStatus() == vtkKWDialog::StatusOK)
      {
        this->ButtonsWorkingDir->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
        vtksys_stl::string filename = this->ButtonsWorkingDir->GetWidget()->GetFileName();
      // this->ButtonsWorkingDir->SetText("Working Directory");

      if(!vtksys::SystemTools::FileExists(filename.c_str()) ||
        !vtksys::SystemTools::FileIsDirectory(filename.c_str()))
        {
        if(!vtksys::SystemTools::MakeDirectory(filename.c_str()))
          {
          vtkErrorMacro("Can not create directory: " << filename.c_str());
          return;
          }
        }

        vtkMRMLTumorGrowthNode* node = this->GetGUI()->GetNode();
        if (node) {
          node->SetWorkingDir(filename.c_str());
          if (this->ButtonsWorkingDir)
            this->ButtonsWorkingDir->GetWidget()->SetText(filename.c_str());
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::SensitivityChangedCallback(int flag)
{
  // cout << "vtkTumorGrowthAnalysisStep::SensitivityChangedCallback" << endl;
  // Sensitivity has changed because of user interaction
  vtkMRMLTumorGrowthNode *mrmlNode = this->GetGUI()->GetNode();
  if (!this->SensitivityMedium || !this->SensitivityLow ||  !this->SensitivityHigh || !mrmlNode || !this->GrowthLabel ) return;

  double senValue = mrmlNode->GetAnalysis_Intensity_Sensitivity();
  if (flag == -1) {
    if (senValue == 0.1) flag = 1;
    else if (senValue == 1.0) flag = 3;
    else {
      flag = 2;
      senValue = 0.6;
    }
  }

  if (flag == 1) {
    if (!this->SensitivityLow->GetSelectedState()) {
      // make sure that it is always on even if you click on it twice
      if (senValue == 0.1) this->SensitivityLow->SelectedStateOn();
      // don't do anything 
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(0.1);
    this->SensitivityMedium->SelectedStateOff();
    this->SensitivityHigh->SelectedStateOff();
  } else if (flag == 2) {
    if (!this->SensitivityMedium->GetSelectedState()) {
      if (senValue == 0.6) this->SensitivityMedium->SelectedStateOn();
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(0.6);
    this->SensitivityLow->SelectedStateOff();
    this->SensitivityHigh->SelectedStateOff();
  } else if (flag == 3) {
    if (!this->SensitivityHigh->GetSelectedState()) {
      if (senValue == 1) this->SensitivityHigh->SelectedStateOn();
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(1);
    this->SensitivityLow->SelectedStateOff();
    this->SensitivityMedium->SelectedStateOff();
  }
  
  vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication())->Script("::TumorGrowthTcl::Analysis_Intensity_UpdateThreshold_GUI"); 
  double Growth = this->GetGUI()->GetLogic()->MeassureGrowth();
  // show here 
  // cout << "Growth " << Growth << " " << this->SensitivityScale->GetValue() << endl;
  char TEXT[1024];
  // cout << "---------- " << Growth << " " << mrmlNode->GetSuperSampled_VoxelVolume() << " " << mrmlNode->GetSuperSampled_RatioNewOldSpacing() << endl;;
  sprintf(TEXT,"Intensity Metric: %.3f mm%c (%d Voxels)", Growth*mrmlNode->GetSuperSampled_VoxelVolume(),179,int(Growth*mrmlNode->GetSuperSampled_RatioNewOldSpacing()));

  this->GrowthLabel->SetText(TEXT);
  // Show updated results 
  vtkMRMLVolumeNode *analysisNode = vtkMRMLVolumeNode::SafeDownCast(mrmlNode->GetScene()->GetNodeByID(mrmlNode->GetAnalysis_Intensity_Ref()));
  if (analysisNode) analysisNode->Modified();
}

//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::TakeScreenshot() {
  vtkImageAppend *screen = vtkImageAppend::New();
    screen->SetAppendAxis(0);

  vtkWindowToImageFilter **window = new vtkWindowToImageFilter*[3];
  vtkImageConstantPad ** windowPad = new vtkImageConstantPad*[3];          
  for (int i = 0 ; i < 3; i++) {
    window[i] = vtkWindowToImageFilter::New();
    windowPad[i] = vtkImageConstantPad::New();    
  }

  window[0]->SetInput(this->GetGUI()->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow());
  window[1]->SetInput(this->GetGUI()->GetApplicationGUI()->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow());
  window[2]->SetInput(this->GetGUI()->GetApplicationGUI()->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetRenderWindow());

  int maxExtent[6];
  for (int i = 0 ; i < 3; i++) {
    window[i]->Update();
    if (i) {
      int newExtent[6];
      window[i]->GetOutput()->GetExtent(newExtent);
      for (int j = 0 ; j < 6; j += 2) {
    if (newExtent[j] < maxExtent[j] ) maxExtent[j] = newExtent[j]; 
    if (newExtent[j+1] > maxExtent[j+1] ) maxExtent[j+1] = newExtent[j+1]; 
      }
    } else {
      window[i]->GetOutput()->GetExtent(maxExtent);
    } 
  }

  for (int i = 0 ; i < 3; i++) {
    windowPad[i]->SetInput(window[i]->GetOutput()); 
    windowPad[i]->SetOutputWholeExtent(maxExtent);
    windowPad[i]->SetConstant(0);
    windowPad[i]->Update();
    screen->AddInput(windowPad[i]->GetOutput());
  }

  screen->Update();

  vtkPNGWriter *saveWriter = vtkPNGWriter::New();
     saveWriter->SetInput(screen->GetOutput());
     this->SnapshotCount ++;
     std::stringstream ss;
     
     char fileName[1024];
     sprintf(fileName,"%s/TG_Screenshot_%03d.png",this->GetGUI()->GetNode()->GetWorkingDir(),this->SnapshotCount);
     saveWriter->SetFileName(fileName);
  saveWriter->Write();

  saveWriter->Delete();

  //std::string infoMsg("View is saved as ");
  //infoMsg.append(fileName);
  //vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),
  //                                        "Tumor Growth",infoMsg.c_str(), vtkKWMessageDialog::OkDefault);

  for (int i = 0 ; i < 3; i++) {
    window[i]->Delete();
    windowPad[i]->Delete();
  }
  delete[] window; 
  delete[] windowPad;



  screen->Delete();
}

//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::ResetPipelineCallback() {
  if (this->GetGUI()) this->GetGUI()->ResetPipeline();
}

//----------------------------------------------------------------------------
void vtkTumorGrowthAnalysisStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void  vtkTumorGrowthAnalysisStep::RemoveResults()  { 
    vtkMRMLTumorGrowthNode* Node = this->GetGUI()->GetNode();
    if (!Node) return;
    {
       vtkMRMLVolumeNode* currentNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetAnalysis_Intensity_Ref()));
       if (currentNode) { 
         this->GetGUI()->GetMRMLScene()->RemoveNode(currentNode); 
         Node->SetAnalysis_Intensity_Ref(NULL);
       }
       
    }
    this->RenderRemove();
}
