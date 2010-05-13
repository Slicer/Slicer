#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkBrainlabModuleLoadingDataStep.h"
#include "vtkBrainlabModuleNavigationStep.h"
#include "vtkBrainlabModuleConnectionStep.h"

#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>


vtkCxxSetObjectMacro(vtkBrainlabModuleGUI,MRMLManager,vtkBrainlabModuleMRMLManager);
vtkCxxSetObjectMacro(vtkBrainlabModuleGUI,Node,vtkMRMLNode);


//----------------------------------------------------------------------------
vtkBrainlabModuleGUI* vtkBrainlabModuleGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkBrainlabModuleGUI");
  if (ret)
    {
    return (vtkBrainlabModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkBrainlabModuleGUI;
}

//----------------------------------------------------------------------------
vtkBrainlabModuleGUI::vtkBrainlabModuleGUI()
{
  this->MRMLManager  = NULL;
  this->Logic        = NULL;
  this->ModuleName   = NULL;

  this->LoadingDataStep = NULL;
  this->ConnectionStep = NULL;
  this->NavigationStep = NULL;

  this->WizardWidget = vtkKWWizardWidget::New();
}

//----------------------------------------------------------------------------
vtkBrainlabModuleGUI::~vtkBrainlabModuleGUI()
{
  this->RemoveLogicObservers();

  this->SetMRMLManager(NULL);
//  this->SetLogic(NULL);

  if (this->WizardWidget)
    {
    this->WizardWidget->Delete();
    this->WizardWidget = NULL;
    }

  if (this->LoadingDataStep)
    {
    this->LoadingDataStep->Delete();
    this->LoadingDataStep = NULL;
    }
  if (this->ConnectionStep)
    {
    this->ConnectionStep->Delete();
    this->ConnectionStep = NULL;
    }
  if (NavigationStep)
    {
    NavigationStep->Delete();
    NavigationStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleGUI::SetModuleLogic(vtkSlicerLogic* logic)
{
  this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 

  //this->SetLogic( dynamic_cast<vtkBrainlabModuleLogic*> (logic) );
  this->GetLogic()->GetMRMLManager()->SetMRMLScene( this->GetMRMLScene() ); 
  this->SetMRMLManager( this->GetLogic()->GetMRMLManager() );
}


//----------------------------------------------------------------------------
void vtkBrainlabModuleGUI::RemoveLogicObservers()
{
  if (this->LoadingDataStep)
    {
    this->LoadingDataStep->Delete();
    this->LoadingDataStep = NULL;
    }
  if (this->ConnectionStep)
    {
    this->ConnectionStep->Delete();
    this->ConnectionStep = NULL;
    }
  if (NavigationStep)
    {
    NavigationStep->Delete();
    NavigationStep = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkBrainlabModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::UpdateMRML()
{
}

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::UpdateGUI()
{
}

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::AddGUIObservers() 
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
void vtkBrainlabModuleGUI::BuildGUI() 
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
  // Load preoperative data step
  if (!this->LoadingDataStep)
    {
    this->LoadingDataStep = vtkBrainlabModuleLoadingDataStep::New();
    this->LoadingDataStep->SetGUI(this);
    }
  wizard_workflow->AddStep(this->LoadingDataStep);


  // -----------------------------------------------------------------
  // Connection step
  if (!this->ConnectionStep)
    {
    this->ConnectionStep = vtkBrainlabModuleConnectionStep::New();
    this->ConnectionStep->SetGUI(this);
    }
  wizard_workflow->AddNextStep(this->ConnectionStep);


  // -----------------------------------------------------------------
  // Parameter Set step
  //
  if (!this->NavigationStep)
    {
    this->NavigationStep = vtkBrainlabModuleNavigationStep::New();
    this->NavigationStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->NavigationStep);


 // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->NavigationStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
//  wizard_workflow->SetInitialStep(this->ConnectionStep);
  wizard_workflow->SetInitialStep(this->LoadingDataStep);

}

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::TearDownGUI() 
{
  if (this->ConnectionStep)
    {
    this->ConnectionStep->SetGUI(NULL);
    }
  if (this->LoadingDataStep)
    {
    this->LoadingDataStep->SetGUI(NULL);
    }
  if (NavigationStep)
    {
    NavigationStep->SetGUI(NULL);
    }
}
//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::ProcessMRMLEvents(vtkObject *caller,
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
unsigned long vtkBrainlabModuleGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand));
} 

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::Init()
{
  // vtkMRMLScene *scene = this->Logic->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
}



void vtkBrainlabModuleGUI::Enter()
{
  this->NavigationStep->Timer(1);
}



void vtkBrainlabModuleGUI::Exit()
{
  this->NavigationStep->Timer(0);
}



