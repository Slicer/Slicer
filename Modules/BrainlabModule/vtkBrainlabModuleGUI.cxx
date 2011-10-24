#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkMRMLLayoutNode.h"

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
vtkStandardNewMacro(vtkBrainlabModuleGUI);

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

  if (this->GetMRMLManager())
    {
    this->GetMRMLManager()->SetMRMLScene(NULL);
    this->SetMRMLManager(NULL);
    }
  this->SetLogic(NULL);

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
  this->SetLogic ( vtkBrainlabModuleLogic::SafeDownCast(logic) ); 

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



void vtkBrainlabModuleGUI::BuildGUIForHelpFrame()
{
  const char *help = "BrainlabModule is an intraoperative navigation systme with simplified steps for neurosurgeons to use Slicer to perform some research in DTI visualization in OR while Brainlab is used as the primary navigation tool.";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The BrainlabModule module was contributed by Haiying Liu, Noby Hata and Ron Kininis at SPL, BWH (Ron Kikinis).";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "BrainlabModule" );
  this->BuildHelpAndAboutFrame (page, help, about);
}

 

//---------------------------------------------------------------------------
void vtkBrainlabModuleGUI::BuildGUI() 
{
  this->UIPanel->AddPage( "BrainlabModule", "BrainlabModule", NULL );

  // The help frame
  BuildGUIForHelpFrame();

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *module_page = this->UIPanel->GetPageWidget("BrainlabModule");

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

void vtkBrainlabModuleGUI::Enter()
{
  this->NavigationStep->Timer(1);
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (appGUI)
    {
    vtkMRMLLayoutNode *layout = appGUI->GetGUILayoutNode();
    appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
    layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutFourUpView);
    }
}



void vtkBrainlabModuleGUI::Exit()
{
  this->NavigationStep->Timer(0);
}



