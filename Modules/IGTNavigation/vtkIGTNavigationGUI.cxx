#include "vtkIGTNavigationGUI.h"
#include "vtkIGTNavigationLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerApplication.h"

#include "vtkKWMessageDialog.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLabel.h"

#include "vtkIGTNavigationInitializationStep.h"
#include "vtkIGTNavigationLoadingPreoperativeDataStep.h"
#include "vtkIGTNavigationCalibrationStep.h"
#include "vtkIGTNavigationRegistrationStep.h"
#include "vtkIGTNavigationIntraoperativeProcedureStep.h"

#include "vtkKWIcon.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

//vtkCxxSetObjectMacro(vtkIGTNavigationGUI,Logic,vtkIGTNavigationLogic);
vtkCxxSetObjectMacro(vtkIGTNavigationGUI,MRMLManager,vtkIGTNavigationMRMLManager);
vtkCxxSetObjectMacro(vtkIGTNavigationGUI,Node,vtkMRMLNode);


//----------------------------------------------------------------------------
vtkIGTNavigationGUI* vtkIGTNavigationGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIGTNavigationGUI");
  if (ret)
    {
    return (vtkIGTNavigationGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIGTNavigationGUI;
}

//----------------------------------------------------------------------------
vtkIGTNavigationGUI::vtkIGTNavigationGUI()
{
  this->MRMLManager  = NULL;
  this->Logic        = NULL;
  this->ModuleName   = NULL;

  this->WizardWidget = vtkKWWizardWidget::New();

  this->InitializationStep = NULL;
  this->LoadingPreoperativeDataStep = NULL;
  this->CalibrationStep = NULL;
  this->RegistrationStep = NULL;
  this->IntraoperativeProcedureStep = NULL;
}

//----------------------------------------------------------------------------
vtkIGTNavigationGUI::~vtkIGTNavigationGUI()
{
  this->RemoveLogicObservers();

  this->SetMRMLManager(NULL);
//  this->SetLogic(NULL);

  if (this->WizardWidget)
    {
    this->WizardWidget->Delete();
    this->WizardWidget = NULL;
    }

  if (this->InitializationStep)
    {
    this->InitializationStep->Delete();
    this->InitializationStep = NULL;
    }
  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->Delete();
    this->LoadingPreoperativeDataStep = NULL;
    }
  if (this->CalibrationStep)
    {
    this->CalibrationStep->Delete();
    this->CalibrationStep = NULL;
    }
  if (this->RegistrationStep)
    {
    this->RegistrationStep->Delete();
    this->RegistrationStep = NULL;
    }
  if (this->IntraoperativeProcedureStep)
    {
    this->IntraoperativeProcedureStep->Delete();
    this->IntraoperativeProcedureStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationGUI::SetModuleLogic(vtkSlicerLogic* logic)
{
  this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 

  //this->SetLogic( dynamic_cast<vtkIGTNavigationLogic*> (logic) );
  this->GetLogic()->GetMRMLManager()->SetMRMLScene( this->GetMRMLScene() ); 
  this->SetMRMLManager( this->GetLogic()->GetMRMLManager() );
}


//----------------------------------------------------------------------------
void vtkIGTNavigationGUI::RemoveLogicObservers()
{
  if (this->InitializationStep)
    {
    this->InitializationStep->Delete();
    this->InitializationStep = NULL;
    }

  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->Delete();
    this->LoadingPreoperativeDataStep = NULL;
    }

  if (this->CalibrationStep)
    {
    this->CalibrationStep->Delete();
    this->CalibrationStep = NULL;
    }

  if (this->RegistrationStep)
    {
    this->RegistrationStep->Delete();
    this->RegistrationStep = NULL;
    }

  if (this->IntraoperativeProcedureStep)
    {
    this->IntraoperativeProcedureStep->Delete();
    this->IntraoperativeProcedureStep = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::UpdateMRML()
{
}

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::UpdateGUI()
{
}

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::AddGUIObservers() 
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
void vtkIGTNavigationGUI::BuildGUI() 
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
  this->WizardWidget->SetClientAreaMinimumHeight(320);
  //this->WizardWidget->SetButtonsPositionToTop();
  this->WizardWidget->HelpButtonVisibilityOn();
  app->Script("pack %s -side top -anchor nw -fill both -expand y",
              this->WizardWidget->GetWidgetName());
  wizard_frame->Delete();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->WizardWidget->GetWizardWorkflow();
  vtkNotUsed(vtkKWWizardWidget *wizard_widget = this->WizardWidget;);

  // -----------------------------------------------------------------
  // Parameter Set step

  if (!this->InitializationStep)
    {
    this->InitializationStep = vtkIGTNavigationInitializationStep::New();
    this->InitializationStep->SetGUI(this);
    }

  wizard_workflow->AddStep(this->InitializationStep);

  // -----------------------------------------------------------------
  // Load preoperative data step

  if (!this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep = vtkIGTNavigationLoadingPreoperativeDataStep::New();
    this->LoadingPreoperativeDataStep->SetGUI(this);
    }
 
  wizard_workflow->AddNextStep(this->LoadingPreoperativeDataStep);

  // -----------------------------------------------------------------
  // Calibration step 

  if (!this->CalibrationStep)
    {
    this->CalibrationStep = vtkIGTNavigationCalibrationStep::New();
    this->CalibrationStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->CalibrationStep);

  // -----------------------------------------------------------------
  // Registration step

  if (!this->RegistrationStep)
    {
    this->RegistrationStep = vtkIGTNavigationRegistrationStep::New();
    this->RegistrationStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->RegistrationStep);

  // -----------------------------------------------------------------
  // Intraoperative procedure step

  if (!this->IntraoperativeProcedureStep)
    {
    this->IntraoperativeProcedureStep = vtkIGTNavigationIntraoperativeProcedureStep::New();
    this->IntraoperativeProcedureStep->SetGUI(this);
    }

  wizard_workflow->AddNextStep(this->IntraoperativeProcedureStep);


 // -----------------------------------------------------------------
  // Initial and finish step

  wizard_workflow->SetFinishStep(this->IntraoperativeProcedureStep);
  wizard_workflow->CreateGoToTransitionsToFinishStep();
  wizard_workflow->SetInitialStep(this->InitializationStep);
}

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::TearDownGUI() 
{
  if (this->InitializationStep)
    {
    this->InitializationStep->SetGUI(NULL);
    }

  if (this->LoadingPreoperativeDataStep)
    {
    this->LoadingPreoperativeDataStep->SetGUI(NULL);
    }

  if (this->CalibrationStep)
    {
    this->CalibrationStep->SetGUI(NULL);
    }

  if (this->RegistrationStep)
    {
    this->RegistrationStep->SetGUI(NULL);
    }

  if (this->IntraoperativeProcedureStep)
    {
    this->IntraoperativeProcedureStep->SetGUI(NULL);
    }
}
//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::ProcessMRMLEvents(vtkObject *caller,
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
unsigned long vtkIGTNavigationGUI::
AddObserverByNumber(vtkObject *observee, unsigned long event) 
{
  return (observee->AddObserver(event, 
                                (vtkCommand *)this->GUICallbackCommand));
} 

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::Init()
{
  // vtkMRMLScene *scene = this->Logic->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkIGTNavigationGUI::ProcessLogicEvents (
  vtkObject *caller, unsigned long event, void *callData )
{
  if ( !caller || !this->WizardWidget)
    {
    return;
    }

  // process Logic changes
}


