#include "vtkIGTPlanningGUI.h"
#include "vtkIGTPlanningLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkIGTPlanningOptimizationStep.h"
#include "vtkIGTPlanningLoadingPreoperativeDataStep.h"
#include "vtkIGTPlanningUserInputStep.h"

#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>


//vtkCxxSetObjectMacro(vtkIGTPlanningGUI,Logic,vtkIGTPlanningLogic);
vtkCxxSetObjectMacro(vtkIGTPlanningGUI,MRMLManager,vtkIGTPlanningMRMLManager);
vtkCxxSetObjectMacro(vtkIGTPlanningGUI,Node,vtkMRMLNode);


//----------------------------------------------------------------------------
vtkIGTPlanningGUI* vtkIGTPlanningGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIGTPlanningGUI");
  if (ret)
    {
    return (vtkIGTPlanningGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIGTPlanningGUI;
}

//----------------------------------------------------------------------------
vtkIGTPlanningGUI::vtkIGTPlanningGUI()
{
  this->MRMLManager  = NULL;
  this->Logic        = NULL;
  this->ModuleName   = NULL;

  this->WizardWidget = vtkKWWizardWidget::New();

  this->OptimizationStep = NULL;
  this->LoadingPreoperativeDataStep = NULL;
  UserInputStep = NULL;
}

//----------------------------------------------------------------------------
vtkIGTPlanningGUI::~vtkIGTPlanningGUI()
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
  if (UserInputStep)
    {
    UserInputStep->Delete();
    UserInputStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkIGTPlanningGUI::SetModuleLogic(vtkSlicerLogic* logic)
{
  this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 

  //this->SetLogic( dynamic_cast<vtkIGTPlanningLogic*> (logic) );
  this->GetLogic()->GetMRMLManager()->SetMRMLScene( this->GetMRMLScene() ); 
  this->SetMRMLManager( this->GetLogic()->GetMRMLManager() );
}


//----------------------------------------------------------------------------
void vtkIGTPlanningGUI::RemoveLogicObservers()
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

  if (UserInputStep)
    {
    UserInputStep->Delete();
    UserInputStep = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkIGTPlanningGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::UpdateMRML()
{
}

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::UpdateGUI()
{
}

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::AddGUIObservers() 
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
void vtkIGTPlanningGUI::BuildGUI() 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  const char *help = 
    "**IGT Module:** **This module provides a generic framework for a workflow-based IGT applications**"; 
  
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
  wizard_frame->SetLabelText("Wizard (Liver Ablation)");
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
  // Load preoperative data step

  if (!this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep = vtkIGTPlanningLoadingPreoperativeDataStep::New();
    this->LoadingPreoperativeDataStep->SetGUI(this);
    }
 
  wizard_workflow->AddStep(this->LoadingPreoperativeDataStep);


  // -----------------------------------------------------------------
  // Parameter Set step
  //
  if (!this->UserInputStep)
    {
    this->UserInputStep = vtkIGTPlanningUserInputStep::New();
    this->UserInputStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->UserInputStep);


  // -----------------------------------------------------------------
  // Optimization step
  //
  if (!this->OptimizationStep)
    {
    this->OptimizationStep = vtkIGTPlanningOptimizationStep::New();
    this->OptimizationStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->OptimizationStep);


 // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->OptimizationStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->LoadingPreoperativeDataStep);

}

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::TearDownGUI() 
{
  if (this->OptimizationStep)
    {
    this->OptimizationStep->SetGUI(NULL);
    }

  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->SetGUI(NULL);
    }

  if (UserInputStep)
    {
    UserInputStep->SetGUI(NULL);
    }
}
//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::ProcessMRMLEvents(vtkObject *caller,
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
unsigned long vtkIGTPlanningGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand));
} 

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::Init()
{
  // vtkMRMLScene *scene = this->Logic->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkIGTPlanningGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
}


