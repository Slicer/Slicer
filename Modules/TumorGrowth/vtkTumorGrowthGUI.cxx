#include "vtkTumorGrowthGUI.h"
#include "vtkTumorGrowthLogic.h"
#include "vtkMRMLTumorGrowthNode.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

// For PopulateTestingData()
// #include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkDirectory.h"
#include "vtkIntArray.h"
#include "vtkTumorGrowthFirstScanStep.h"
#include "vtkTumorGrowthROIStep.h"
#include "vtkTumorGrowthSegmentationStep.h"
// #include "vtkTumorGrowthSecondScanStep.h"
#include "vtkTumorGrowthTypeStep.h"
#include "vtkTumorGrowthAnalysisStep.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkSlicerSliceControllerWidget.h"

// #include "CSAILLogo.h"
#include "vtkKWIcon.h"

vtkCxxSetObjectMacro(vtkTumorGrowthGUI,Node,vtkMRMLTumorGrowthNode);
vtkCxxSetObjectMacro(vtkTumorGrowthGUI,Logic,vtkTumorGrowthLogic);

//----------------------------------------------------------------------------
vtkTumorGrowthGUI* vtkTumorGrowthGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkTumorGrowthGUI");
  if (ret)
    {
    return (vtkTumorGrowthGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkTumorGrowthGUI;
}

//----------------------------------------------------------------------------
vtkTumorGrowthGUI::vtkTumorGrowthGUI()
{
  this->Logic        = NULL;
  this->Node         = NULL;
  this->ModuleName   = NULL;

  this->WizardWidget     = vtkKWWizardWidget::New();
  this->FirstScanStep    = NULL;
  this->ROIStep    = NULL;
  this->SegmentationStep = NULL;
  this->TypeStep   = NULL;
  this->AnalysisStep     = NULL;

  this->SliceLogic      = NULL;
  this->SliceController_OffsetScale = NULL;
  this->SliceLogicCallbackCommand= NULL;

//  vtkKWIcon* logo = vtkKWIcon::New();
//   logo->SetImage(image_CSAILLogo,
//                 image_CSAILLogo_width, image_CSAILLogo_height,
//                 image_CSAILLogo_pixel_size, image_CSAILLogo_length,
//                 0);
//  this->Logo = logo;
//  logo->Delete();
}

//----------------------------------------------------------------------------
vtkTumorGrowthGUI::~vtkTumorGrowthGUI()
{
  this->RemoveMRMLNodeObservers();
  this->RemoveLogicObservers();
  this->SliceLogicRemove();

  this->SetLogic(NULL);
  this->SetNode(NULL);

  if (this->WizardWidget)
    {
    this->WizardWidget->Delete();
    this->WizardWidget = NULL;
    }

  if (this->FirstScanStep)
    {
    this->FirstScanStep->Delete();
    this->FirstScanStep = NULL;
  }

  if (this->ROIStep)
    {
    this->ROIStep->Delete();
    this->ROIStep = NULL;
  }

  if (this->SegmentationStep)
    {
    this->SegmentationStep->Delete();
    this->SegmentationStep = NULL;
  }

  if (this->TypeStep)
    {
    this->TypeStep->Delete();
    this->TypeStep = NULL;
  }
  if (this->AnalysisStep)
    {
    this->AnalysisStep->Delete();
    this->AnalysisStep = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthGUI::RemoveMRMLNodeObservers()
{
}

//----------------------------------------------------------------------------
void vtkTumorGrowthGUI::RemoveLogicObservers()
{
}

//----------------------------------------------------------------------------
void vtkTumorGrowthGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::AddGUIObservers() 
{
  // observe when nodes are added or removed from the scene
  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  
  // Here nothing happens normally other bc the individual pannels are not created yet and the one for the first step is already created 
  // - add if clause so that same event is not added twice 
  // The wizrad creates them once they are shown on the gui for the first time - and does not delete them afterwards - strange 
  // Have to list them here so if they are all deleted and this function is called afterwards the missing ones are created again 
  if (this->FirstScanStep) this->FirstScanStep->AddGUIObservers();
  if (this->ROIStep) this->ROIStep->AddGUIObservers();
  if (this->TypeStep) this->TypeStep->AddGUIObservers();

  events->Delete();
}

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::RemoveGUIObservers()
{
  if (this->FirstScanStep)    this->FirstScanStep->RemoveGUIObservers();
  if (this->ROIStep)          this->ROIStep->RemoveGUIObservers();
  if (this->SegmentationStep) this->SegmentationStep->RemoveGUIObservers();
  if (this->TypeStep)   this->TypeStep->RemoveGUIObservers();
  if (this->AnalysisStep)     this->AnalysisStep->RemoveGUIObservers();
  this->SliceLogicRemoveGUIObserver(); 
}

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::ProcessGUIEvents(vtkObject *caller,
                                                      unsigned long event,
                                                      void *callData) 
{
  if (this->FirstScanStep)    this->FirstScanStep->ProcessGUIEvents(caller, event, callData); 
  if (this->ROIStep)          this->ROIStep->ProcessGUIEvents(caller, event, callData); 
  if (this->SegmentationStep) this->SegmentationStep->ProcessGUIEvents(caller, event, callData); 
  if (this->TypeStep)   this->TypeStep->ProcessGUIEvents(caller, event, callData); 
  if (this->AnalysisStep)     this->AnalysisStep->ProcessGUIEvents(caller, event, callData); 
}


//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
  vtkTumorGrowthLogic *callbackLogic = 
    vtkTumorGrowthLogic::SafeDownCast(caller);
  
  if ( callbackLogic == this->GetLogic ( ) && 
    event == vtkCommand::ProgressEvent) 
    {
    this->UpdateRegistrationProgress();
    }
}

//----------------------------------------------------------------------------
void vtkTumorGrowthGUI::UpdateRegistrationProgress()
{
  // Kilian: Do this later for analysis 
  double progress = this->Logic->GetProgressGlobalFractionCompleted();
  if(progress>=0 && progress <=1)
    {
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->
      SetValue(progress*100);
    }
}

//----------------------------------------------------------------------------
void  vtkTumorGrowthGUI::UpdateNode()
{
  if (this->GetMRMLScene() == NULL)
    {
    vtkSetMRMLNodeMacro(this->Node, NULL);
    return;
    }

  vtkMRMLTumorGrowthNode *tmpNode = vtkMRMLTumorGrowthNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLTumorGrowthNode"));
  if (this->GetNode() != NULL && tmpNode != NULL && strcmp(tmpNode->GetID(), this->Node->GetID()) )
    {
    vtkSetAndObserveMRMLNodeMacro(this->Node, tmpNode);
    }
  
  if (this->Node == NULL && tmpNode != NULL)
    {
    vtkSetAndObserveMRMLNodeMacro(this->Node, tmpNode);
    }
  if (this->Node == NULL)
    {
      // std::cout <<"UpdateMRML: n is null, create new one?!" << "\n";
    //    no parameter node selected yet, create new    
    tmpNode = vtkMRMLTumorGrowthNode::New();
    this->GetMRMLScene()->AddNode(tmpNode);
    this->Logic->SetAndObserveTumorGrowthNode(tmpNode);
    vtkSetAndObserveMRMLNodeMacro(this->Node, tmpNode);
    tmpNode->Delete();
   }
  // save node parameters for Undo
  this->GetLogic()->SetAndObserveTumorGrowthNode(this->Node);
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(this->Node);


}


//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::UpdateMRML()
{
  this->UpdateNode();

  // Update individual entries 
  if (this->FirstScanStep)    this->FirstScanStep->UpdateMRML(); 
  if (this->ROIStep)          this->ROIStep->UpdateMRML(); 
  if (this->SegmentationStep) this->SegmentationStep->UpdateMRML(); 
  if (this->TypeStep)   this->TypeStep->UpdateMRML(); 
  if (this->AnalysisStep)     this->AnalysisStep->UpdateMRML(); 
}

// according to vtkGradnientAnisotrpoicDiffusionoFilterGUI
//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::UpdateGUI()
{
  this->UpdateNode();
  // This might have to be changed bc instances might not yet be created 
  if (this->FirstScanStep)    this->FirstScanStep->UpdateGUI(); 
  if (this->ROIStep)          this->ROIStep->UpdateGUI(); 
  if (this->SegmentationStep) this->SegmentationStep->UpdateGUI(); 
  if (this->TypeStep)   this->TypeStep->UpdateGUI(); 
  if (this->AnalysisStep)     this->AnalysisStep->UpdateGUI(); 
}


//  according to vtkGradnientAnisotrpoicDiffusionoFilterGUI

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::ProcessMRMLEvents(vtkObject *caller,
                                       unsigned long event,
                                       void *callData) 
{

  // cout << "============ vtkTumorGrowthGUI::ProcessMRMLEvents Start ========== " << caller->GetClassName() << " " << event << endl;
  {
    vtkMRMLTumorGrowthNode* node = vtkMRMLTumorGrowthNode::SafeDownCast(caller);
    if (node != NULL && this->GetNode() == node)  
    {
      this->UpdateGUI();
      return;

    }
  }

  // Make sure that if Scan*_ref are defined before volumes rae loaded then this proparly updates the GUIs after Volumes are loaded 
  // Should do the same for NodeRemoveEvent -> update References correctly - currently not done 
  if ((event == vtkMRMLScene::NodeAddedEvent)  && this->FirstScanStep) {
    if (this->Node->GetScan1_Ref() && (this->Node->GetScan1_Ref() != "") && !this->FirstScanStep->GetFirstVolumeMenuButton()->GetSelected()) {
      this->FirstScanStep->UpdateGUI();
    } else if (this->Node->GetScan2_Ref() && (this->Node->GetScan2_Ref() != "") && !this->FirstScanStep->GetSecondVolumeMenuButton()->GetSelected()) {
      this->FirstScanStep->UpdateGUI();
    }
  }

  // cout << "============ vtkTumorGrowthGUI::ProcessMRMLEvents End ==========" << endl;
}

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::BuildGUI() 
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  const char *help = "**TumorGrowth Module:** **Under Construction** ";
  
  this->Logic->RegisterMRMLNodesWithScene();

  this->UIPanel->AddPage("TumorGrowth", "TumorGrowth", NULL);
  vtkKWWidget *module_page = 
    this->UIPanel->GetPageWidget("TumorGrowth");

  // -----------------------------------------------------------------------
  // Help

  vtkSlicerModuleCollapsibleFrame *help_frame = 
    vtkSlicerModuleCollapsibleFrame::New();
  help_frame->SetParent(module_page);
  help_frame->Create();
  help_frame->CollapseFrame();
  help_frame->SetLabelText("Help");
  help_frame->Delete();

  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
              help_frame->GetWidgetName(), 
              module_page->GetWidgetName());
  
  // configure the parent classes help text widget

  this->HelpText->SetParent(help_frame->GetFrame());
  this->HelpText->Create();
  this->HelpText->SetHorizontalScrollbarVisibility(0);
  this->HelpText->SetVerticalScrollbarVisibility(1);
  this->HelpText->GetWidget()->SetText(help);
  this->HelpText->GetWidget()->SetReliefToFlat();
  this->HelpText->GetWidget()->SetWrapToWord();
  this->HelpText->GetWidget()->ReadOnlyOn();
  this->HelpText->GetWidget()->QuickFormattingOn();

  app->Script("pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
              this->HelpText->GetWidgetName());

  // -----------------------------------------------------------------------
  // Define Wizard with the order of the steps

  vtkSlicerModuleCollapsibleFrame *wizard_frame = 
    vtkSlicerModuleCollapsibleFrame::New();
  wizard_frame->SetParent(module_page);
  wizard_frame->Create();
  wizard_frame->SetLabelText("Wizard");
  wizard_frame->ExpandFrame();

  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
              wizard_frame->GetWidgetName(), 
              module_page->GetWidgetName());
   
  this->WizardWidget->SetParent(wizard_frame->GetFrame());
  this->WizardWidget->Create();
  this->WizardWidget->GetSubTitleLabel()->SetHeight(1);
  this->WizardWidget->SetClientAreaMinimumHeight(150);
  //this->WizardWidget->SetButtonsPositionToTop();

  this->WizardWidget->HelpButtonVisibilityOff();
  this->WizardWidget->CancelButtonVisibilityOff();
  this->WizardWidget->FinishButtonVisibilityOff();

  app->Script("pack %s -side top -anchor nw -fill both -expand y",
              this->WizardWidget->GetWidgetName());
  wizard_frame->Delete();
 
  vtkKWWizardWorkflow *wizard_workflow = 
    this->WizardWidget->GetWizardWorkflow();
  vtkNotUsed(vtkKWWizardWidget *wizard_widget = this->WizardWidget;);

  // -----------------------------------------------------------------
  // Parameter Set step
   
  // To add a step to the wizard
  // - Create files in Wizard directory
  // - Inlude them into CMakeLists.txt
  // add variable to vtkTumorGrowthGUI.h
  // add to vtkTumorGrowthGUI.cxx : 
  //    - header file
  //    - vtkTumorGrowthGUI::vtkTumorGrowthGUI(), 
  //    - vtkTumorGrowthGUI::~vtkTumorGrowthGUI(
  //    - here
  //    - Tear Down GUI


  if (!this->FirstScanStep)
  {
    this->FirstScanStep = vtkTumorGrowthFirstScanStep::New();
    this->FirstScanStep->SetGUI(this);
  }
  wizard_workflow->AddStep(this->FirstScanStep);

  if (!this->ROIStep)
    {
    this->ROIStep = vtkTumorGrowthROIStep::New();
    this->ROIStep->SetGUI(this);
    this->FirstScanStep->SetNextStep(this->ROIStep);
    }
  wizard_workflow->AddNextStep(this->ROIStep);

  if (!this->SegmentationStep)
    {
    this->SegmentationStep = vtkTumorGrowthSegmentationStep::New();
    this->SegmentationStep->SetGUI(this);
    this->ROIStep->SetNextStep(this->SegmentationStep);
    }
  wizard_workflow->AddNextStep(this->SegmentationStep);

  if (!this->TypeStep)
    {
    this->TypeStep = vtkTumorGrowthTypeStep::New();
    this->TypeStep->SetGUI(this);
    this->SegmentationStep->SetNextStep(this->TypeStep);
    }
  wizard_workflow->AddNextStep(this->TypeStep);

  if (!this->AnalysisStep)
    {
    this->AnalysisStep = vtkTumorGrowthAnalysisStep::New();
    this->AnalysisStep->SetGUI(this);
    this->TypeStep->SetNextStep(this->AnalysisStep);
    }
  wizard_workflow->AddNextStep(this->AnalysisStep);

  // -----------------------------------------------------------------
  // Initial and finish step
  wizard_workflow->SetFinishStep(this->AnalysisStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->FirstScanStep);
  this->ROIStep->GetInteractionState();
  // This way we can restart the machine - did not work 
  // wizard_workflow->CreateGoToTransitions(wizard_workflow->GetInitialStep());

 if ( 0 )  {
    cout << "====================" << endl;
    cout << "DEBUGGING" << endl;
    vtkSlicerApplicationGUI *applicationGUI = this->GetApplicationGUI();
    if (!applicationGUI) return; 
  
    // char fileName[1024] = "/home/pohl/Slicer/Slicer3-build/blub.mrml";
    char fileName[1024] = "/home/pohl/Slicer/Slicer3-build/Modules/TumorGrowth/Testing/test.mrml";
    std::string fl(fileName);
    applicationGUI->GetMRMLScene()->SetURL(fileName);
    applicationGUI->GetMRMLScene()->Connect();
    cout << "====================" << endl;
    // this->VolumeMenuButton->SetSelected(applicationGUI->GetMRMLScene()->GetNodeByID("vtkMRMLScalarVolumeNode1")); 

  }

}

//---------------------------------------------------------------------------
void vtkTumorGrowthGUI::TearDownGUI() 
{
   if (this->FirstScanStep)
   {
     this->FirstScanStep->SetGUI(NULL);
   }

   if (this->ROIStep)
   {
     this->ROIStep->SetGUI(NULL);
   }

   if (this->SegmentationStep)
   {
     this->SegmentationStep->SetGUI(NULL);
   }

   if (this->TypeStep)
   {
     this->TypeStep->SetGUI(NULL);
   }

   if (this->AnalysisStep)
   {
     this->AnalysisStep->SetGUI(NULL);
   }

}

//---------------------------------------------------------------------------
unsigned long vtkTumorGrowthGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, 
                                (vtkCommand *)this->GUICallbackCommand));
} 

void vtkTumorGrowthGUI::SliceLogicRemoveGUIObserver() {
  if (!this) return;
  if (!this->GetApplicationGUI()) return;

  if (this->SliceController_OffsetScale) {
    this->SliceController_OffsetScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale = NULL;
  }
}

void vtkTumorGrowthGUI::SliceLogicRemove() {
  this->SliceLogicRemoveGUIObserver();
  if (!this) return;
  if (this->SliceLogicCallbackCommand) {
    this->SliceLogicCallbackCommand->Delete();
    this->SliceLogicCallbackCommand = NULL; 
  }

  if (this->SliceLogic) {
    // this->SliceLogic->GetSliceNode()->SetSliceVisible(0);
     vtkSlicerApplicationLogic *applicationLogic = this->GetLogic()->GetApplicationLogic();
     if (applicationLogic)
       {
       applicationLogic->RemoveSliceLogic(this->SliceLogic->GetName());
       }
     this->SliceLogic->Delete();
     this->SliceLogic = NULL;
  } 
}

void vtkTumorGrowthGUI::SliceLogicDefine() {
  if (!this->SliceLogic) {
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
      events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

      this->SliceLogic = vtkSlicerSliceLogic::New ( );
      this->SliceLogic->SetName("TG");

      this->SliceLogic->SetMRMLScene ( this->GetMRMLScene());
      this->SliceLogic->ProcessLogicEvents ();
      this->SliceLogic->ProcessMRMLEvents (this->GetMRMLScene(), vtkCommand::ModifiedEvent, NULL);
      this->SliceLogic->SetAndObserveMRMLSceneEvents (this->GetMRMLScene(), events );
      events->Delete();

      vtkSlicerApplicationLogic *applicationLogic = this->GetLogic()->GetApplicationLogic();
      if (applicationLogic)
        {
        applicationLogic->AddSliceLogic(this->SliceLogic->GetName(),
                                        this->SliceLogic);
        }
    } 

    if (!this->SliceLogicCallbackCommand) {
       this->SliceLogicCallbackCommand=vtkCallbackCommand::New();
       this->SliceLogicCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
       this->SliceLogicCallbackCommand->SetCallback(vtkTumorGrowthGUI::SliceLogicCallback);
    }


    // Link to slicer control pannel 
    if (!this->SliceController_OffsetScale) {
      this->SliceController_OffsetScale =  this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale();
      this->SliceController_OffsetScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, this->SliceLogicCallbackCommand);
      this->SliceController_OffsetScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, this->SliceLogicCallbackCommand);
      this->SliceController_OffsetScale->GetWidget()->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, this->SliceLogicCallbackCommand);
    }

    this->SliceLogic->GetSliceNode()->SetSliceVisible(1);
  
    this->PropagateVolumeSelection();

    // Note : Setting things manually in TCL 
    // Always do both together 
    // [[[vtkTumorGrowthROIStep ListInstances] GetSliceLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID vtkMRMLScalarVolumeNode1
    // or 
    //  set GUI  [$::slicer3::Application GetModuleGUIByName "TumorGrowth"]
    //  [[$GUI GetSliceLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID vtkMRMLScalarVolumeNode1

    // [[[vtkTumorGrowthROIStep ListInstances] GetSliceLogic] GetSliceNode] SetFieldOfView 200 200 1
    //[[$GUI GetSliceLogic] GetSliceNode] SetFieldOfView 200 200 1
} 

void  vtkTumorGrowthGUI::SliceLogicCallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{

    vtkTumorGrowthGUI *self = reinterpret_cast< vtkTumorGrowthGUI *>(clientData);
    if (self && self->GetSliceController_OffsetScale()) {
      if (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkKWScale::ScaleValueChangingEvent) {
    vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);
    if (scale && (scale == self->GetSliceController_OffsetScale()->GetWidget())) 
      { 
        self->GetSliceLogic()->SetSliceOffset(self->GetSliceController_OffsetScale()->GetValue());
      }
      }
    }
}

void vtkTumorGrowthGUI::PropagateVolumeSelection() {
   vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
   applicationLogic->PropagateVolumeSelection( 0 );
   if (!this->SliceLogic) return;
   this->SliceLogic->GetSliceCompositeNode()->SetReferenceBackgroundVolumeID(this->Node->GetScan1_Ref());
   this->SliceLogic->FitSliceToVolume(vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(this->Node->GetScan1_Ref())),250,250); 
   this->SliceLogic->SetSliceOffset(this->SliceController_OffsetScale->GetValue());
}
