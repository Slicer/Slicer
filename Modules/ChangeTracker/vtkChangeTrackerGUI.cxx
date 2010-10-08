#include "vtkChangeTrackerGUI.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkMRMLChangeTrackerNode.h"
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
#include "vtkChangeTrackerFirstScanStep.h"
#include "vtkChangeTrackerROIStep.h"
#include "vtkChangeTrackerSegmentationStep.h"
// #include "vtkChangeTrackerSecondScanStep.h"
#include "vtkChangeTrackerTypeStep.h"
#include "vtkChangeTrackerAnalysisStep.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkSlicerSliceControllerWidget.h"

#include "ImageData/BSFLogo.h"
#include "vtkKWIcon.h"
#include "vtkMRMLROINode.h"
#include <assert.h>

vtkCxxSetObjectMacro(vtkChangeTrackerGUI,Node,vtkMRMLChangeTrackerNode);
vtkCxxSetObjectMacro(vtkChangeTrackerGUI,Logic,vtkChangeTrackerLogic);

//----------------------------------------------------------------------------
vtkChangeTrackerGUI* vtkChangeTrackerGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkChangeTrackerGUI");
  if (ret)
    {
    return (vtkChangeTrackerGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkChangeTrackerGUI;
}

//----------------------------------------------------------------------------
vtkChangeTrackerGUI::vtkChangeTrackerGUI()
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

  this->roiNode = NULL;

  // Logo setup. To prepare the header file with the logo information, use
  // KWConvertImageToHeader tool supplied with KWWidgets, with --zlib option.
  vtkKWIcon* logo = vtkKWIcon::New();
  logo->SetImage(image_bsf_logo,
                 image_bsf_logo_width, image_bsf_logo_height,
                 image_bsf_logo_pixel_size, image_bsf_logo_length,
                 0);
  this->Logo = logo;
  logo->Delete();

  this->TutorialButton = NULL;

  this->ModuleEntered = false;
}

//----------------------------------------------------------------------------
vtkChangeTrackerGUI::~vtkChangeTrackerGUI()
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
  if (this->TutorialButton)
    {
    this->TutorialButton->Delete();
    this->TutorialButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerGUI::RemoveMRMLNodeObservers()
{
}

//----------------------------------------------------------------------------
void vtkChangeTrackerGUI::RemoveLogicObservers()
{
}

//----------------------------------------------------------------------------
void vtkChangeTrackerGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkIntArray*  vtkChangeTrackerGUI::NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::AddGUIObservers() 
{
  // Here nothing happens normally other bc the individual pannels are not created yet and the one for the first step is already created 
  // - add if clause so that same event is not added twice 
  // The wizrad creates them once they are shown on the gui for the first time - and does not delete them afterwards - strange 
  // Have to list them here so if they are all deleted and this function is called afterwards the missing ones are created again 
  if (this->FirstScanStep) this->FirstScanStep->AddGUIObservers();
  if (this->ROIStep)       this->ROIStep->AddGUIObservers();
  if (this->TypeStep)      this->TypeStep->AddGUIObservers();
  if (this->AnalysisStep)  this->AnalysisStep->AddGUIObservers();
}

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::RemoveGUIObservers()
{
  if (this->FirstScanStep)    this->FirstScanStep->RemoveGUIObservers();
  if (this->ROIStep)          this->ROIStep->RemoveGUIObservers();
  if (this->SegmentationStep) this->SegmentationStep->RemoveGUIObservers();
  if (this->TypeStep)   this->TypeStep->RemoveGUIObservers();
  if (this->AnalysisStep)     this->AnalysisStep->RemoveGUIObservers();

  this->SliceLogicRemoveGUIObserver(); 
}

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::ProcessGUIEvents(vtkObject *caller,
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
void vtkChangeTrackerGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
  vtkChangeTrackerLogic *callbackLogic = 
    vtkChangeTrackerLogic::SafeDownCast(caller);
  
  if ( callbackLogic == this->GetLogic ( ) && 
    event == vtkCommand::ProgressEvent) 
    {
    this->UpdateRegistrationProgress();
    }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerGUI::UpdateRegistrationProgress()
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
void  vtkChangeTrackerGUI::UpdateNode()
{
  if (this->GetMRMLScene() == NULL)
    {
    vtkSetMRMLNodeMacro(this->Node, NULL);
    return;
    }

  vtkMRMLChangeTrackerNode *tmpNode = vtkMRMLChangeTrackerNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLChangeTrackerNode"));
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
    tmpNode = vtkMRMLChangeTrackerNode::New();
    this->GetMRMLScene()->AddNode(tmpNode);
    this->Logic->SetAndObserveChangeTrackerNode(tmpNode);
    vtkSetAndObserveMRMLNodeMacro(this->Node, tmpNode);
    tmpNode->Delete();
   }
  // save node parameters for Undo
  this->GetLogic()->SetAndObserveChangeTrackerNode(this->Node);
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(this->Node);


}

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::Enter()
{
  this->ModuleEntered = true;
  if (this->FirstScanStep)    this->FirstScanStep->RenderShow(); 
  if (this->ROIStep)          this->ROIStep->RenderShow(); 
  if (this->SegmentationStep) this->SegmentationStep->RenderShow(); 
  if (this->TypeStep)   this->TypeStep->RenderShow(); 
  if (this->AnalysisStep)     this->AnalysisStep->RenderShow(); 

  vtkKWWizardWorkflow *wizard_workflow = 
    this->WizardWidget->GetWizardWorkflow();
  if(wizard_workflow){
    if(wizard_workflow->GetCurrentStep() == this->ROIStep){
      this->ROIStep->AddROISamplingGUIObservers();
    }
  }
}

// according to vtkGradnientAnisotrpoicDiffusionoFilterGUI
//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::Exit()
{
  this->ModuleEntered = false;
  if (this->FirstScanStep)    this->FirstScanStep->RenderHide(); 
  if (this->ROIStep)          this->ROIStep->RenderHide(); 
  if (this->SegmentationStep) this->SegmentationStep->RenderHide(); 
  if (this->TypeStep)   this->TypeStep->RenderHide(); 
  if (this->AnalysisStep)     this->AnalysisStep->RenderHide(); 
  
  vtkKWWizardWorkflow *wizard_workflow = 
    this->WizardWidget->GetWizardWorkflow();
  if(wizard_workflow){
    if(wizard_workflow->GetCurrentStep() == this->ROIStep){
      this->ROIStep->RemoveROISamplingGUIObservers();
    }
  }

}


//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::UpdateMRML()
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
void vtkChangeTrackerGUI::UpdateGUI()
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
void vtkChangeTrackerGUI::ProcessMRMLEvents(vtkObject *caller,
                                       unsigned long event,
                                       void *callData) 
{
  vtkMRMLROINode* roiCaller = vtkMRMLROINode::SafeDownCast(caller);
  if(roiCaller && this->roiNode == roiCaller)
    {
    // need to route the event to the ROI step, since it cannot handle events
    // directly
    if(this->ROIStep)
      this->ROIStep->ProcessMRMLEvents(caller, event, callData);
    return;
    }

  // cout << "============ vtkChangeTrackerGUI::ProcessMRMLEvents Start ========== " << caller->GetClassName() << " " << event << endl;
  if (event == vtkMRMLScene::SceneClosedEvent ) {
    this->ResetPipeline();
    return;
  }
   
  {
    vtkMRMLChangeTrackerNode* node = vtkMRMLChangeTrackerNode::SafeDownCast(caller);
    if (node != NULL && this->GetNode() == node)  
    {
      this->UpdateGUI();
      return;

    }
  }

  // Make sure that if Scan*_ref are defined before volumes rae loaded then this proparly updates the GUIs after Volumes are loaded 
  // Should do the same for NodeRemoveEvent -> update References correctly - currently not done 
  if ((event == vtkMRMLScene::NodeAddedEvent)  && this->FirstScanStep && this->Node)
    {
    if (this->Node->GetScan1_Ref() && (strcmp(this->Node->GetScan1_Ref(),"") != 0) && !this->FirstScanStep->GetFirstVolumeMenuButton()->GetSelected()) {
      this->FirstScanStep->UpdateGUI();
    } else if (this->Node->GetScan2_Ref() && (strcmp(this->Node->GetScan2_Ref(),"") != 0) && !this->FirstScanStep->GetSecondVolumeMenuButton()->GetSelected()) {
      this->FirstScanStep->UpdateGUI();
    }
  }

  // cout << "============ vtkChangeTrackerGUI::ProcessMRMLEvents End ==========" << endl;
}

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::BuildGUI() 
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->Logic->RegisterMRMLNodesWithScene();

  this->UIPanel->AddPage("ChangeTracker", "ChangeTracker", NULL);
  vtkKWWidget *module_page = 
    this->UIPanel->GetPageWidget("ChangeTracker");


  // -----------------------------------------------------------------------
  // Help
 
  const char* help_text = "ChangeTracker allows to detect subtle changes in pathology. The change is quantified in mm^3 for growth/shrinkage, and is also visualized with tumor changes color-coded. The module documentation can be found at <a>http://www.slicer.org/slicerWiki/index.php/Modules:ChangeTracker-Documentation-3.6</a>.";
  const char* ack_text = "ChangTracker has been developed and supported by Kilian Pohl, Ender Konukoglu, Andriy Fedorov and Slicer community. PI: Ron Kikinis. Development of this module was supported through the funding from Brain Science Foundation <a>http://www.brainsciencefoundation.org/</a>";
  this->BuildHelpAndAboutFrame(module_page, help_text, ack_text);

  // Show the logo
  if (this->GetLogo())
  {
    vtkKWLabel *logoLabel = vtkKWLabel::New();
    logoLabel->SetParent( this->GetLogoFrame() );
    logoLabel->Create();
    logoLabel->SetImageToIcon( this->GetLogo() );
    app->Script("pack %s", logoLabel->GetWidgetName() );
    logoLabel->Delete();
  }

  // add the ability to load tutorial data
  if(!this->TutorialButton)
    {
    this->TutorialButton = vtkKWPushButton::New();
    }

  if (!this->TutorialButton->IsCreated()) {
    this->TutorialButton->SetParent(this->GetHelpAndAboutFrame()->GetFrame());
    this->TutorialButton->Create();
    this->TutorialButton->SetText("Load Tutorial data");
    this->TutorialButton->SetBalloonHelpString("Load the tutorial data.");
    this->TutorialButton->SetCommand(this, "LoadTutorialData");
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
               this->TutorialButton->GetWidgetName());

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
  // add variable to vtkChangeTrackerGUI.h
  // add to vtkChangeTrackerGUI.cxx : 
  //    - header file
  //    - vtkChangeTrackerGUI::vtkChangeTrackerGUI(), 
  //    - vtkChangeTrackerGUI::~vtkChangeTrackerGUI(
  //    - here
  //    - Tear Down GUI


  if (!this->FirstScanStep)
  {
    this->FirstScanStep = vtkChangeTrackerFirstScanStep::New();
    this->FirstScanStep->SetGUI(this);
  }
  wizard_workflow->AddStep(this->FirstScanStep);

  if (!this->ROIStep)
    {
    this->ROIStep = vtkChangeTrackerROIStep::New();
    this->ROIStep->SetGUI(this);
    this->FirstScanStep->SetNextStep(this->ROIStep);
    }
  wizard_workflow->AddNextStep(this->ROIStep);

  if (!this->SegmentationStep)
    {
    this->SegmentationStep = vtkChangeTrackerSegmentationStep::New();
    this->SegmentationStep->SetGUI(this);
    this->ROIStep->SetNextStep(this->SegmentationStep);
    }
  wizard_workflow->AddNextStep(this->SegmentationStep);

  if (!this->TypeStep)
    {
    this->TypeStep = vtkChangeTrackerTypeStep::New();
    this->TypeStep->SetGUI(this);
    this->SegmentationStep->SetNextStep(this->TypeStep);
    }
  wizard_workflow->AddNextStep(this->TypeStep);

  if (!this->AnalysisStep)
    {
    this->AnalysisStep = vtkChangeTrackerAnalysisStep::New();
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
  
    // char fileName[1024] = "/home/pohl/Slicer/Slicer-build/blub.mrml";
    char fileName[1024] = "/home/pohl/Slicer/Slicer-build/Modules/ChangeTracker/Testing/test.mrml";
    std::string fl(fileName);
    applicationGUI->GetMRMLScene()->SetURL(fileName);
    applicationGUI->GetMRMLScene()->Connect();
    cout << "====================" << endl;
    // this->VolumeMenuButton->SetSelected(applicationGUI->GetMRMLScene()->GetNodeByID("vtkMRMLScalarVolumeNode1")); 

  }

}


//----------------------------------------------------------------------------
void vtkChangeTrackerGUI::ResetPipeline() 
{
   vtkKWWizardWorkflow *wizard_workflow = this->WizardWidget->GetWizardWorkflow();
   if (!wizard_workflow) return;
   vtkKWWizardStep  *currentState = NULL;
   vtkKWWizardStep  *newState =  wizard_workflow->GetCurrentStep();
   vtkKWWizardStep  *initialState =  wizard_workflow->GetInitialStep();
   if (!initialState) return;
   while ((newState != initialState) && (newState != currentState)) {
      wizard_workflow->AttemptToGoToPreviousStep();
      currentState = newState;
      newState =  wizard_workflow->GetCurrentStep();
   }
}
  

//---------------------------------------------------------------------------
void vtkChangeTrackerGUI::TearDownGUI() 
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
unsigned long vtkChangeTrackerGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, 
                                (vtkCommand *)this->GUICallbackCommand));
} 

void vtkChangeTrackerGUI::SliceLogicRemoveGUIObserver() {
  if (!this) return;
  if (!this->GetApplicationGUI()) return;

  if (this->SliceController_OffsetScale) {
    this->SliceController_OffsetScale->RemoveObservers(vtkKWScale::ScaleValueChangedEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale->RemoveObservers(vtkKWScale::ScaleValueChangingEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent, this->SliceLogicCallbackCommand);
    this->SliceController_OffsetScale = NULL;
  }
}

void vtkChangeTrackerGUI::SliceLogicRemove() {
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

void vtkChangeTrackerGUI::SliceLogicDefine() {
  if (!this->SliceLogic) {
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
      events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

      this->SliceLogic = vtkMRMLSliceLogic::New ( );
      this->SliceLogic->SetName("TG");

      this->SliceLogic->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
      this->SliceLogic->ProcessLogicEvents ();
      this->SliceLogic->ProcessMRMLEvents (this->GetMRMLScene(), vtkCommand::ModifiedEvent, NULL);
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
       this->SliceLogicCallbackCommand->SetCallback(vtkChangeTrackerGUI::SliceLogicCallback);
    }


    // Link to slicer control pannel 
    if (!this->SliceController_OffsetScale) {
      this->SliceController_OffsetScale =  this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale();
      this->SliceController_OffsetScale->AddObserver(vtkKWScale::ScaleValueChangedEvent, this->SliceLogicCallbackCommand);
      this->SliceController_OffsetScale->AddObserver(vtkKWScale::ScaleValueChangingEvent, this->SliceLogicCallbackCommand);
      this->SliceController_OffsetScale->AddObserver(vtkKWScale::ScaleValueStartChangingEvent, this->SliceLogicCallbackCommand);
    }

    this->SliceLogic->GetSliceNode()->SetSliceVisible(1);
  
    this->PropagateVolumeSelection();

    // Note : Setting things manually in TCL 
    // Always do both together 
    // [[[vtkChangeTrackerROIStep ListInstances] GetSliceLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID vtkMRMLScalarVolumeNode1
    // or 
    //  set GUI  [$::slicer3::Application GetModuleGUIByName "ChangeTracker"]
    //  [[$GUI GetSliceLogic] GetSliceCompositeNode] SetReferenceBackgroundVolumeID vtkMRMLScalarVolumeNode1

    // [[[vtkChangeTrackerROIStep ListInstances] GetSliceLogic] GetSliceNode] SetFieldOfView 200 200 1
    //[[$GUI GetSliceLogic] GetSliceNode] SetFieldOfView 200 200 1
} 

void  vtkChangeTrackerGUI::SliceLogicCallback(vtkObject *caller, unsigned long event, void *clientData, void *vtkNotUsed(callData))
{

    vtkChangeTrackerGUI *self = reinterpret_cast< vtkChangeTrackerGUI *>(clientData);
    if (self && self->GetSliceController_OffsetScale()) {
      if (event == vtkKWScale::ScaleValueChangedEvent || event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkKWScale::ScaleValueChangingEvent) {
    vtkKWScale *scale = vtkKWScale::SafeDownCast(caller);
    if (scale && (scale == self->GetSliceController_OffsetScale())) 
      { 
        self->GetSliceLogic()->SetSliceOffset(self->GetSliceController_OffsetScale()->GetValue());
      }
      }
    }
}

void vtkChangeTrackerGUI::PropagateVolumeSelection() {
   vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
   applicationLogic->PropagateVolumeSelection( 0 );
   if (!this->SliceLogic) return;
   this->SliceLogic->GetSliceCompositeNode()->SetReferenceBackgroundVolumeID(this->Node->GetScan1_Ref());
   this->SliceLogic->FitSliceToVolume(vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(this->Node->GetScan1_Ref())),250,250); 
   this->SliceLogic->SetSliceOffset(this->SliceController_OffsetScale->GetValue());
}

void vtkChangeTrackerGUI::ObserveMRMLROINode(vtkMRMLROINode* roi){
    vtkSetAndObserveMRMLNodeMacro(this->roiNode, roi);
}

void vtkChangeTrackerGUI::LoadTutorialData(){
  vtkMRMLScene *scene = this->GetNode()->GetScene();
  int res;
  scene->SetURL(TUTORIAL_XNAT_SCENE);
  res = scene->Connect();
  if(scene->GetErrorCode())
    {
    vtkErrorMacro("ERROR: Failed to connect to the tutorial scene. Error code: " << scene->GetErrorCode() 
      << " Error message: " << scene->GetErrorMessage());
    }
}
  
void vtkChangeTrackerGUI::SetRedGreenYellowAllVolumes(const char* bgVolID, 
    const char* vtkNotUsed(fgVolID), const char* vtkNotUsed(lVolID)){
  vtkSlicerSliceGUI *redGUI, *greenGUI, *yellowGUI;
  double oldSliceSetting[3];
  vtkSlicerApplicationGUI *applicationGUI = this->GetApplicationGUI();

  redGUI = applicationGUI->GetMainSliceGUI("Red");
  greenGUI = applicationGUI->GetMainSliceGUI("Green");
  yellowGUI = applicationGUI->GetMainSliceGUI("Yellow");

  oldSliceSetting[0] = double(redGUI->GetLogic()->GetSliceOffset());
  oldSliceSetting[1] = double(greenGUI->GetLogic()->GetSliceOffset());
  oldSliceSetting[2] = double(yellowGUI->GetLogic()->GetSliceOffset());

  redGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(bgVolID);
  redGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");
  yellowGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(bgVolID);
  yellowGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");
  greenGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(bgVolID);
  greenGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");

  redGUI->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
  greenGUI->GetLogic()->SetSliceOffset(oldSliceSetting[1]);
  yellowGUI->GetLogic()->SetSliceOffset(oldSliceSetting[2]);
}
