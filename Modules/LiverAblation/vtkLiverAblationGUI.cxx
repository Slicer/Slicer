#include "vtkLiverAblationGUI.h"
#include "vtkLiverAblationLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkLiverAblationOptimizationStep.h"
#include "vtkLiverAblationPlanningStep.h"
#include "vtkLiverAblationTrackingStep.h"
#include "vtkLiverAblationNavigationStep.h"
#include "vtkLiverAblationLoadingPreoperativeDataStep.h"
#include "vtkLiverAblationPreplanningStep.h"

#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>


//vtkCxxSetObjectMacro(vtkLiverAblationGUI,Logic,vtkLiverAblationLogic);
vtkCxxSetObjectMacro(vtkLiverAblationGUI,MRMLManager,vtkLiverAblationMRMLManager);
vtkCxxSetObjectMacro(vtkLiverAblationGUI,Node,vtkMRMLNode);


//----------------------------------------------------------------------------
vtkLiverAblationGUI* vtkLiverAblationGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkLiverAblationGUI");
  if (ret)
    {
    return (vtkLiverAblationGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkLiverAblationGUI;
}

//----------------------------------------------------------------------------
vtkLiverAblationGUI::vtkLiverAblationGUI()
{
  this->MRMLManager  = NULL;
  this->Logic        = NULL;
  this->ModuleName   = NULL;

  this->WizardWidget = vtkKWWizardWidget::New();

  this->OptimizationStep = NULL;
  this->LoadingPreoperativeDataStep = NULL;
  this->PreplanningStep = NULL;
  this->PlanningStep = NULL;
  this->TrackingStep = NULL;
  this->NavigationStep = NULL;

}

//----------------------------------------------------------------------------
vtkLiverAblationGUI::~vtkLiverAblationGUI()
{
  this->RemoveLogicObservers();

  this->SetMRMLManager(NULL);
//  this->SetLogic(NULL);

  if (this->WizardWidget)
    {
    this->WizardWidget->Delete();
    this->WizardWidget = NULL;
    }

  if (this->OptimizationStep)
    {
    this->OptimizationStep->Delete();
    this->OptimizationStep = NULL;
    }
  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->Delete();
    this->LoadingPreoperativeDataStep = NULL;
    }
  if (this->PreplanningStep)
    {
    this->PreplanningStep->Delete();
    this->PreplanningStep = NULL;
    }
  if (this->PlanningStep)
    {
    this->PlanningStep->Delete();
    this->PlanningStep = NULL;
    }
  if (this->TrackingStep)
    {
    this->TrackingStep->Delete();
    this->TrackingStep = NULL;
    }
  if (this->NavigationStep)
    {
    this->NavigationStep->Delete();
    this->NavigationStep = NULL;
    }


}

//----------------------------------------------------------------------------
void vtkLiverAblationGUI::SetModuleLogic(vtkSlicerLogic* logic)
{
  this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 

  //this->SetLogic( dynamic_cast<vtkLiverAblationLogic*> (logic) );
  this->GetLogic()->GetMRMLManager()->SetMRMLScene( this->GetMRMLScene() ); 
  this->SetMRMLManager( this->GetLogic()->GetMRMLManager() );
}


//----------------------------------------------------------------------------
void vtkLiverAblationGUI::RemoveLogicObservers()
{
  if (this->OptimizationStep)
    {
    this->OptimizationStep->Delete();
    this->OptimizationStep = NULL;
    }

  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->Delete();
    this->LoadingPreoperativeDataStep = NULL;
    }

  if (this->PreplanningStep)
    {
    this->PreplanningStep->Delete();
    this->PreplanningStep = NULL;
    }
  if (this->PlanningStep)
    {
    this->PlanningStep->Delete();
    this->PlanningStep = NULL;
    }
  if (this->TrackingStep)
    {
    this->TrackingStep->Delete();
    this->TrackingStep = NULL;
    }
  if (this->NavigationStep)
    {
    this->NavigationStep->Delete();
    this->NavigationStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkLiverAblationGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::UpdateMRML()
{
}

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::UpdateGUI()
{
}

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::AddGUIObservers() 
{
  // observe when nodes are added or removed from the scene
  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();
}

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::BuildGUI() 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  const char *help = 
    "**IGT Module:** **This module provides a workflow-based guidance for RF liver ablation.**"; 
  
  this->UIPanel->AddPage("IGT", 
                         "IGT", NULL);
  vtkKWWidget *module_page = 
    this->UIPanel->GetPageWidget("IGT");

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
  // Wizard

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
  this->WizardWidget->SetClientAreaMinimumHeight(400);
  //this->WizardWidget->SetButtonsPositionToTop();
  this->WizardWidget->HelpButtonVisibilityOn();
  app->Script("pack %s -side top -anchor nw -fill both -expand y",
              this->WizardWidget->GetWidgetName());
  wizard_frame->Delete();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->WizardWidget->GetWizardWorkflow();
  vtkNotUsed(vtkKWWizardWidget *wizard_widget = this->WizardWidget;);

  // -----------------------------------------------------------------
  // Load preoperative images step

  if (!this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep = vtkLiverAblationLoadingPreoperativeDataStep::New();
    this->LoadingPreoperativeDataStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->LoadingPreoperativeDataStep);

  // -----------------------------------------------------------------
  // Preplanning step
  //
  if (!this->PreplanningStep)
    {
    this->PreplanningStep = vtkLiverAblationPreplanningStep::New();
    this->PreplanningStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->PreplanningStep);

  // -----------------------------------------------------------------
  // Planning step
  //
  if (!this->PlanningStep)
    {
    this->PlanningStep = vtkLiverAblationPlanningStep::New();
    this->PlanningStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->PlanningStep);


  // -----------------------------------------------------------------
  // Optimization step
  //
  if (!this->OptimizationStep)
    {
    this->OptimizationStep = vtkLiverAblationOptimizationStep::New();
    this->OptimizationStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->OptimizationStep);

  // -----------------------------------------------------------------
  // Tracking step
  //
  if (!this->TrackingStep)
    {
    this->TrackingStep = vtkLiverAblationTrackingStep::New();
    this->TrackingStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->TrackingStep);

  // -----------------------------------------------------------------
  // Navigation step
  //
  if (!this->NavigationStep)
    {
    this->NavigationStep = vtkLiverAblationNavigationStep::New();
    this->NavigationStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->NavigationStep);


  // -----------------------------------------------------------------
  // Initial and finish step
  //
  wizard_workflow->SetFinishStep(this->NavigationStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->LoadingPreoperativeDataStep);


  if (wizard_workflow->GetCurrentStep())
    {
    wizard_workflow->GetCurrentStep()->ShowUserInterface();
    }
}



//---------------------------------------------------------------------------
void vtkLiverAblationGUI::TearDownGUI() 
{
  if (this->OptimizationStep)
    {
    this->OptimizationStep->SetGUI(NULL);
    }
  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->SetGUI(NULL);
    }
  if (this->PreplanningStep)
    {
    this->PreplanningStep->SetGUI(NULL);
    }
  if (this->PlanningStep)
    {
    this->PlanningStep->SetGUI(NULL);
    }
  if (this->TrackingStep)
    {
    this->TrackingStep->SetGUI(NULL);
    }
}



//---------------------------------------------------------------------------
void vtkLiverAblationGUI::ProcessMRMLEvents(vtkObject *caller,
                                       unsigned long event,
                                       void *callData) 
{
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent 
    || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*)(callData);
    if (node != NULL && node->IsA("vtkMRMLVolumeNode"))
      {
      //Check MRML node type
      }
    }
}


//---------------------------------------------------------------------------
unsigned long vtkLiverAblationGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand));
} 

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::Init()
{
  // vtkMRMLScene *scene = this->Logic->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkLiverAblationGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
}


