#include "vtkIntegratedNavigationGUI.h"
#include "vtkIntegratedNavigationLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkIntegratedNavigationLoadingDataStep.h"
#include "vtkIntegratedNavigationUserInputStep.h"

#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>


vtkCxxSetObjectMacro(vtkIntegratedNavigationGUI,MRMLManager,vtkIntegratedNavigationMRMLManager);
vtkCxxSetObjectMacro(vtkIntegratedNavigationGUI,Node,vtkMRMLNode);


//----------------------------------------------------------------------------
vtkIntegratedNavigationGUI* vtkIntegratedNavigationGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIntegratedNavigationGUI");
  if (ret)
    {
    return (vtkIntegratedNavigationGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIntegratedNavigationGUI;
}

//----------------------------------------------------------------------------
vtkIntegratedNavigationGUI::vtkIntegratedNavigationGUI()
{
  this->MRMLManager  = NULL;
  this->Logic        = NULL;
  this->ModuleName   = NULL;

  this->LoadingDataStep = NULL;
  this->UserInputStep = NULL;

  this->WizardWidget = vtkKWWizardWidget::New();
}

//----------------------------------------------------------------------------
vtkIntegratedNavigationGUI::~vtkIntegratedNavigationGUI()
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
  if (UserInputStep)
    {
    UserInputStep->Delete();
    UserInputStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::SetModuleLogic(vtkSlicerLogic* logic)
{
  this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 

  //this->SetLogic( dynamic_cast<vtkIntegratedNavigationLogic*> (logic) );
  this->GetLogic()->GetMRMLManager()->SetMRMLScene( this->GetMRMLScene() ); 
  this->SetMRMLManager( this->GetLogic()->GetMRMLManager() );
}


//----------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::RemoveLogicObservers()
{
  if (this->LoadingDataStep)
    {
    this->LoadingDataStep->Delete();
    this->LoadingDataStep = NULL;
    }

  if (UserInputStep)
    {
    UserInputStep->Delete();
    UserInputStep = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::UpdateMRML()
{
}

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::UpdateGUI()
{
}

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::AddGUIObservers() 
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
void vtkIntegratedNavigationGUI::BuildGUI() 
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
    this->LoadingDataStep = vtkIntegratedNavigationLoadingDataStep::New();
    this->LoadingDataStep->SetGUI(this);
    }
 
  wizard_workflow->AddStep(this->LoadingDataStep);


  // -----------------------------------------------------------------
  // Parameter Set step
  //
  if (!this->UserInputStep)
    {
    this->UserInputStep = vtkIntegratedNavigationUserInputStep::New();
    this->UserInputStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->UserInputStep);


 // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->UserInputStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->LoadingDataStep);

}

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::TearDownGUI() 
{
  if (this->LoadingDataStep)
    {
    this->LoadingDataStep->SetGUI(NULL);
    }

  if (UserInputStep)
    {
    UserInputStep->SetGUI(NULL);
    }
}
//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::ProcessMRMLEvents(vtkObject *caller,
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
unsigned long vtkIntegratedNavigationGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, (vtkCommand *)this->GUICallbackCommand));
} 

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::Init()
{
  // vtkMRMLScene *scene = this->Logic->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkIntegratedNavigationGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
}


