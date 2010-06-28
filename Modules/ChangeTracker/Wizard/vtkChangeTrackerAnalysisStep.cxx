#include "vtkChangeTrackerAnalysisStep.h"

#include "vtkChangeTrackerGUI.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWThumbWheel.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWScale.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
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
#include "vtkRenderer.h"
#include "vtkMRMLCrosshairNode.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerAnalysisStep);
vtkCxxRevisionMacro(vtkChangeTrackerAnalysisStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkChangeTrackerAnalysisStep::vtkChangeTrackerAnalysisStep()
{
  this->SetName("Analysis"); 
  this->SetDescription("Analysis of Tumor Growth"); 
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerAnalysisStep::WizardGUICallback);

  this->FrameIntensity = NULL;
  this->FrameIntensityCol1 = NULL;
  this->FrameIntensityCol2 = NULL;
  this->FrameIntensityCol3 = NULL;

  this->IntensityLabel = NULL;
  this->IntensityResultVolume = NULL;
  this->IntensityResultVoxel = NULL;

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
vtkChangeTrackerAnalysisStep::~vtkChangeTrackerAnalysisStep()
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

    if (this->FrameIntensity) 
    {
      this->FrameIntensity->Delete();
      this->FrameIntensity = NULL;
    }

    if (this->FrameIntensityCol1) 
    {
      this->FrameIntensityCol1->Delete();
      this->FrameIntensityCol1 = NULL;
    }

    if (this->FrameIntensityCol2) 
    {
      this->FrameIntensityCol2->Delete();
      this->FrameIntensityCol2 = NULL;
    }

    if (this->FrameIntensityCol3) 
    {
      this->FrameIntensityCol3->Delete();
      this->FrameIntensityCol3 = NULL;
    }


  if (this->IntensityLabel) 
    {
      this->IntensityLabel->Delete();
      this->IntensityLabel = NULL;
    }

  if (this->IntensityResultVolume) 
    {
      this->IntensityResultVolume->Delete();
      this->IntensityResultVolume = NULL;
    }

  if (this->IntensityResultVoxel) 
    {
      this->IntensityResultVoxel->Delete();
      this->IntensityResultVoxel = NULL;
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
void vtkChangeTrackerAnalysisStep::AddGUIObservers() 
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

void vtkChangeTrackerAnalysisStep::RemoveGUIObservers() 
{
  // cout << "vtkChangeTrackerAnalysisStep::RemoveGUIObservers" << endl;
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

void vtkChangeTrackerAnalysisStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
  // cout << "void vtkChangeTrackerAnalysisStep::WizardGUICallback" << endl;
    vtkChangeTrackerAnalysisStep *self = reinterpret_cast<vtkChangeTrackerAnalysisStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}


void vtkChangeTrackerAnalysisStep::ProcessGUIEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData))
{

  // cout << "vtkChangeTrackerAnalysisStep::ProcessGUIEvents" << endl;

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
     vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
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
          vtkChangeTrackerLogic *Logic = this->GetGUI()->GetLogic();
          Logic->SaveVolumeForce(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()),volumeAnalysisNode);
        }   
          }

      if (node->GetAnalysis_Deformable_Flag()) { 
            volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Deformable_Ref()));
        if (volumeAnalysisNode) {
          vtkChangeTrackerLogic *Logic = this->GetGUI()->GetLogic();
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
           cout << "Error: vtkChangeTrackerAnalysisStep::ProcessGUIEvents: Cannot write to file " << fileName.c_str() << endl;
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
void vtkChangeTrackerAnalysisStep::ShowUserInterface()
{
  // ----------------------------------------
  // Display Analysis Volume 
  // ----------------------------------------  
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  vtkMRMLVolumeNode *volumeAnalysisNode = NULL;
  
  if (node) { 
    vtkMRMLVolumeNode *volumeSampleNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_SuperSampleRef()));

    if (node->GetAnalysis_Intensity_Flag()) {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Intensity_Ref()));
    } else if (node->GetAnalysis_Deformable_Flag()) {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetAnalysis_Deformable_Ref()));
    } else {
      volumeAnalysisNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan2_LocalRef()));
    }

    // Colors for volume rendering are the same as those used for label
    // volume display. These are stored in the color lut associated with the
    // volume node.
    // TODO: watch display node and update volume rendering in case lut is
    // modified (see the code in 
    // Modules/Volumes/vtkSlicerLabelMapVolumeDisplayWidget.cxx)
    if(node->GetAnalysis_Intensity_Flag() || node->GetAnalysis_Deformable_Flag())
      {
      vtkMRMLVolumeDisplayNode *display = 
        vtkMRMLVolumeDisplayNode::SafeDownCast(volumeAnalysisNode->GetDisplayNode());
      vtkMRMLColorNode *color = display->GetColorNode();
      vtkLookupTable *color_lut = color->GetLookupTable();
      
      float colorMax[3] = {0.8 , 0.0, 0.0 };
      float colorMin[3] = {0.0 , 0.8, 0.0 };
      double colorMax_d[3], colorMin_d[3];

      color_lut->GetColor(12., colorMin_d);
      color_lut->GetColor(14., colorMax_d);
      colorMin[0] = (float) colorMin_d[0];
      colorMin[1] = (float) colorMin_d[1];
      colorMin[2] = (float) colorMin_d[2];
      colorMax[0] = (float) colorMax_d[0];
      colorMax[1] = (float) colorMax_d[1];
      colorMax[2] = (float) colorMax_d[2];

      this->CreateRender(volumeAnalysisNode, 1);
      this->SetRender_PulsePassFilter(12, 14, colorMin, colorMax);
      }

    if (volumeSampleNode && volumeAnalysisNode) {
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeSampleNode->GetID());

      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();

      // Initialize CompareView layout
      applicationGUI->GetGUILayoutNode()->SetNumberOfCompareViewRows(2);
      applicationGUI->GetGUILayoutNode()->SetNumberOfCompareViewColumns(1);
      applicationGUI->GetGUILayoutNode()->SetNumberOfCompareViewLightboxRows(1);
      applicationGUI->GetGUILayoutNode()->SetNumberOfCompareViewLightboxColumns(5);
      applicationGUI->GetGUILayoutNode()->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutCompareView);

      vtkSlicerSliceGUI *cv0GUI, *cv1GUI, *redGUI;
      
      cv0GUI = applicationGUI->GetMainSliceGUI("Compare0");
      cv1GUI = applicationGUI->GetMainSliceGUI("Compare1");
      redGUI = applicationGUI->GetMainSliceGUI("Red");

      if(cv0GUI && cv1GUI)
        {
        vtkMRMLSliceCompositeNode *cv0SliceCompositeNode, *cv1SliceCompositeNode;
        vtkSlicerSliceControllerWidget *cv0SliceControllerWidget, *cv1SliceControllerWidget;
        vtkMRMLSliceLogic *cv0SliceLogic, *cv1SliceLogic;
        cv0SliceCompositeNode = cv0GUI->GetLogic()->GetSliceCompositeNode();
        cv1SliceCompositeNode = cv1GUI->GetLogic()->GetSliceCompositeNode();
        cv0SliceLogic = cv0GUI->GetLogic();
        cv1SliceLogic = cv1GUI->GetLogic();
        cv0SliceControllerWidget = cv0GUI->GetSliceController();
        cv1SliceControllerWidget = cv1GUI->GetSliceController();

        cv0SliceControllerWidget->UnlinkAllSlices();
//        cv0->SetDoPropagateVolumeSelection(false);
//        cv1->SetDoPropagateVolumeSelection(false);

        cv0SliceCompositeNode->SetBackgroundVolumeID( node->GetScan1_SuperSampleRef() );
        cv1SliceCompositeNode->SetBackgroundVolumeID( node->GetScan2_NormedRef() );

        cv0SliceCompositeNode->SetLabelVolumeID( "" );
        cv1SliceCompositeNode->SetLabelVolumeID( "" );

        cv0SliceCompositeNode->SetForegroundVolumeID( "" );
        cv1SliceCompositeNode->SetForegroundVolumeID( "" );

        cv0SliceCompositeNode->SetLabelVolumeID( volumeAnalysisNode->GetID() );
        cv1SliceCompositeNode->SetLabelVolumeID( "" );

        cv0SliceCompositeNode->SetLabelOpacity(0.6);

        // this is the right way to fit slice to background in this scenario,
        // according to Steve
        // Background is not set yet here, so fit can only be done after MRML
        // processing has been completed
//        this->Script("proc Fit2Bg {sGUIName} { set GUI [$::slicer3::ApplicationGUI GetMainSliceGUI $sGUIName]; [$GUI GetSliceController] FitSliceToBackground}");
        this->Script("update");
        cv0GUI->GetSliceController()->FitSliceToBackground();
        cv1GUI->GetSliceController()->FitSliceToBackground();
        redGUI->GetSliceController()->FitSliceToBackground();

        cv0SliceControllerWidget->LinkAllSlices();

        // set up the crosshairs
        vtkMRMLCrosshairNode *xnode;
        int nnodes = applicationGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLCrosshairNode");
        for(int i=0;i<nnodes;i++){
          xnode = vtkMRMLCrosshairNode::SafeDownCast (
            applicationGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLCrosshairNode" ) );
          xnode->SetCrosshairMode ( vtkMRMLCrosshairNode::ShowHashmarks );
          xnode->NavigationOff();
        }
      }
    } 
  }

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------

  this->vtkChangeTrackerStep::ShowUserInterface();

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
  //   this->SensitivityScale->SetThumbWheelSize (CHANGETRACKER_WIDGETS_SLIDER_WIDTH,CHANGETRACKER_WIDGETS_SLIDER_HEIGHT);
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
  // vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
  // if (mrmlNode) this->SensitivityScale->SetValue(mrmlNode->GetAnalysis_Intensity_Sensitivity());
  // this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->SensitivityScale->GetWidgetName());


  if (!this->FrameIntensity)
  {
    this->FrameIntensity = vtkKWFrame::New();
  }
  if (!this->FrameIntensity->IsCreated())
  {
      this->FrameIntensity->SetParent(this->Frame->GetFrame());
      this->FrameIntensity->Create();
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->FrameIntensity->GetWidgetName());

  if (!this->FrameIntensityCol1)
  {
    this->FrameIntensityCol1 = vtkKWFrame::New();
  }
  if (!this->FrameIntensityCol1->IsCreated())
  {
      this->FrameIntensityCol1->SetParent(this->FrameIntensity);
      this->FrameIntensityCol1->Create();
  }

  if (!this->FrameIntensityCol2)
  {
    this->FrameIntensityCol2 = vtkKWFrame::New();
  }
  if (!this->FrameIntensityCol2->IsCreated())
  {
      this->FrameIntensityCol2->SetParent(this->FrameIntensity);
      this->FrameIntensityCol2->Create();
  }

  if (!this->FrameIntensityCol3)
  {
    this->FrameIntensityCol3 = vtkKWFrame::New();
  }
  if (!this->FrameIntensityCol3->IsCreated())
  {
      this->FrameIntensityCol3->SetParent(this->FrameIntensity);
      this->FrameIntensityCol3->Create();
  }

  this->Script("pack %s %s %s -side left -anchor nw -fill x -padx 0 -pady 0", this->FrameIntensityCol1->GetWidgetName(),this->FrameIntensityCol2->GetWidgetName(),this->FrameIntensityCol3->GetWidgetName());


  if (!this->IntensityLabel)
    {
    this->IntensityLabel = vtkKWLabel::New();
    }
  if (!this->IntensityLabel->IsCreated())
  {
    this->IntensityLabel->SetParent(this->FrameIntensityCol1);
    std::string CMD = "::ChangeTrackerTcl::RonsWishFlag";
    int RonsWishFlag = atoi(this->Script(CMD.c_str()));
    if (RonsWishFlag) {    
      this->IntensityLabel->SetText("Shrinkage:\nGrowth:\nTotal Change:");
    } else {
      this->IntensityLabel->SetText("Total Change:");
    }
    this->IntensityLabel->Create();
  }
  this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->IntensityLabel->GetWidgetName());

  if (!this->IntensityResultVolume)
    {
    this->IntensityResultVolume = vtkKWLabel::New();
    }
  if (!this->IntensityResultVolume->IsCreated())
  {
    this->IntensityResultVolume->SetParent(this->FrameIntensityCol2);
    this->IntensityResultVolume->Create();
  }
  this->Script( "pack %s -side top -anchor ne -padx 2 -pady 2", this->IntensityResultVolume->GetWidgetName());


  if (!this->IntensityResultVoxel)
    {
    this->IntensityResultVoxel = vtkKWLabel::New();
    }
  if (!this->IntensityResultVoxel->IsCreated())
  {
    this->IntensityResultVoxel->SetParent(this->FrameIntensityCol3);
    this->IntensityResultVoxel->Create();
  }
  this->Script( "pack %s -side top -anchor ne -padx 2 -pady 2", this->IntensityResultVoxel->GetWidgetName());



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
    this->ButtonsSnapshot->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH_SMALL);
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
    this->ButtonsAnalysis->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH_SMALL);
    this->ButtonsAnalysis->SetText("Analysis");
    this->ButtonsAnalysis->SetBalloonHelpString("Save the results of the analysis to a disk."); 
  }

  if (!this->ButtonsSave) {
    this->ButtonsSave = vtkKWPushButton::New();
  }
  if (!this->ButtonsSave->IsCreated()) {
    this->ButtonsSave->SetParent(this->FrameButtonsFunctions);
    this->ButtonsSave->Create();
    this->ButtonsSave->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH_SMALL);
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
      
  this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
//  this->GetGUI()->PropagateVolumeSelection();
}


//----------------------------------------------------------------------------
void vtkChangeTrackerAnalysisStep::SelectDirectoryCallback()
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

        vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
        if (node) {
          node->SetWorkingDir(filename.c_str());
          if (this->ButtonsWorkingDir)
            this->ButtonsWorkingDir->GetWidget()->SetText(filename.c_str());
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkChangeTrackerAnalysisStep::SensitivityChangedCallback(int flag)
{
  // cout << "vtkChangeTrackerAnalysisStep::SensitivityChangedCallback" << endl;
  // Sensitivity has changed because of user interaction
  vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
  if (!this->SensitivityMedium || !this->SensitivityLow ||  !this->SensitivityHigh || !mrmlNode || !this->IntensityResultVoxel || !this->IntensityResultVolume || !mrmlNode->GetAnalysis_Intensity_Flag()) return;

  double senValue = mrmlNode->GetAnalysis_Intensity_Sensitivity();
  // original values   int senValueList[3] = {0.1, 0.6, 1.0};
  double senValueList[3] = {0.9, 0.96, 0.99};

  if (flag == -1) {
    if (senValue == senValueList[0]) flag = 1;
    else if (senValue == senValueList[2]) flag = 3;
    else {
      flag = 2;
      senValue = senValueList[1];
    }
  }

  if (flag == 1) {
    if (!this->SensitivityLow->GetSelectedState()) {
      // make sure that it is always on even if you click on it twice
      if (senValue == senValueList[0]) this->SensitivityLow->SelectedStateOn();
      // don't do anything 
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(senValueList[0]);
    this->SensitivityMedium->SelectedStateOff();
    this->SensitivityHigh->SelectedStateOff();
  } else if (flag == 2) {
    if (!this->SensitivityMedium->GetSelectedState()) {
      if (senValue == senValueList[1]) this->SensitivityMedium->SelectedStateOn();
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(senValueList[1]);
    this->SensitivityLow->SelectedStateOff();
    this->SensitivityHigh->SelectedStateOff();
  } else if (flag == 3) {
    if (!this->SensitivityHigh->GetSelectedState()) {
      if (senValue == senValueList[2]) this->SensitivityHigh->SelectedStateOn();
      return;
    }
    mrmlNode->SetAnalysis_Intensity_Sensitivity(senValueList[2]);
    this->SensitivityLow->SelectedStateOff();
    this->SensitivityMedium->SelectedStateOff();
  }
  
  vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication())->Script("::ChangeTrackerTcl::Analysis_Intensity_UpdateThreshold_GUI"); 
  double Shrinkage,Growth;
  this->GetGUI()->GetLogic()->MeassureGrowth(Shrinkage, Growth);
  double Total = Growth + Shrinkage;
  // show here 
  // cout << "Growth " << Growth << " " << this->SensitivityScale->GetValue() << endl;
  char TEXT[1024];

  std::string CMD = "::ChangeTrackerTcl::RonsWishFlag";
  int RonsWishFlag = atoi(this->Script(CMD.c_str()));


  if (RonsWishFlag) {
    sprintf(TEXT,"%.3f mm%c\n%.3f mm%c\n%.3f mm%c", -Shrinkage*mrmlNode->GetSuperSampled_VoxelVolume(),179,Growth*mrmlNode->GetSuperSampled_VoxelVolume(),179, Total*mrmlNode->GetSuperSampled_VoxelVolume(),179);
  } else {
    sprintf(TEXT,"%.3f mm%c", Total*mrmlNode->GetSuperSampled_VoxelVolume(),179);
  }
    this->IntensityResultVolume->SetText(TEXT);

  if (RonsWishFlag) {
     sprintf(TEXT,"(%d Voxels)\n(%d Voxels)\n(%d Voxels)", int(-Shrinkage*mrmlNode->GetSuperSampled_RatioNewOldSpacing()),int(Growth*mrmlNode->GetSuperSampled_RatioNewOldSpacing()),int(Total*mrmlNode->GetSuperSampled_RatioNewOldSpacing()));
  } else {
     sprintf(TEXT,"(%d Voxels)", int(Total*mrmlNode->GetSuperSampled_RatioNewOldSpacing()));
  }

  this->IntensityResultVoxel->SetText(TEXT);
  // Show updated results 
  vtkMRMLVolumeNode *analysisNode = vtkMRMLVolumeNode::SafeDownCast(mrmlNode->GetScene()->GetNodeByID(mrmlNode->GetAnalysis_Intensity_Ref()));
  if (analysisNode) analysisNode->Modified();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerAnalysisStep::TakeScreenshot() {
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
void vtkChangeTrackerAnalysisStep::ResetPipelineCallback() {
  if (this->GetGUI()) this->GetGUI()->ResetPipeline();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerAnalysisStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void  vtkChangeTrackerAnalysisStep::RemoveResults()  { 
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
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
