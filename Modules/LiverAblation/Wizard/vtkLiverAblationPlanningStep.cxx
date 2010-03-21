#include "vtkLiverAblationPlanningStep.h"

#include "vtkLiverAblationGUI.h"
#include "vtkLiverAblationMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWGuideWidget.h"
#include "vtkKWInternationalization.h"

#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLiverAblationPlanningStep);
vtkCxxRevisionMacro(vtkLiverAblationPlanningStep, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkLiverAblationPlanningStep::vtkLiverAblationPlanningStep()
{
  this->SetName("2/4. Planning");
  this->SetDescription("Use existing Slicer modules to perform surgical planning.");

  this->FiducialButton = NULL;
  this->EditorButton = NULL;
  this->SegmentationButton = NULL;

}

//----------------------------------------------------------------------------
vtkLiverAblationPlanningStep::~vtkLiverAblationPlanningStep()
{  
  if(this->FiducialButton)
    {
    this->FiducialButton->Delete();
    this->FiducialButton = NULL;
    }
  if(this->EditorButton)
    {
    this->EditorButton->Delete();
    this->EditorButton = NULL;
    }
  if(this->SegmentationButton)
    {
    this->SegmentationButton->Delete();
    this->SegmentationButton = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  if (!this->FiducialButton)
    {
    this->FiducialButton = vtkKWPushButton::New();
    this->FiducialButton->SetParent (parent);
    this->FiducialButton->Create();
    this->FiducialButton->SetText("Fiducial Module");
    this->FiducialButton->SetCommand(this, "FiducialButtonCallback");
    this->FiducialButton->SetWidth(30);
    }

  if (!this->EditorButton)
    {
    this->EditorButton = vtkKWPushButton::New();
    this->EditorButton->SetParent (parent);
    this->EditorButton->Create();
    this->EditorButton->SetText("Editor Module");
    this->EditorButton->SetCommand(this, "EditorButtonCallback");
    this->EditorButton->SetWidth(30);
    }

  if (!this->SegmentationButton)
    {
    this->SegmentationButton = vtkKWPushButton::New();
    this->SegmentationButton->SetParent (parent);
    this->SegmentationButton->Create();
    this->SegmentationButton->SetText("FastMarchingSegmentation Module");
    this->SegmentationButton->SetCommand(this, "SegmentationButtonCallback");
    this->SegmentationButton->SetWidth(30);
    }

  this->Script(
    "pack %s %s %s -side top -anchor center -padx 2 -pady 5", 
    this->FiducialButton->GetWidgetName(),
    this->EditorButton->GetWidgetName(),
    this->SegmentationButton->GetWidgetName());

   //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Load Preoperative Images Step");
  msg_dlg1->SetText( "This is the first step in IGT applications. In this step, the user loads "
                     "preoperative data required for IGT application. This includes images "
                     "(CT, MRI data), surgical tool/endoscope probe models, and surgical planning "
                     "information");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}
 
 
//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{
  // FiducailButton Pressed

  if (this->FiducialButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    this->GetGUI()->GetApplication()->Script("::LoadVolume::ShowDialog");
//    this->GetGUI()->GetApplication();
    }
}


//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::FiducialButtonCallback()
{
  this->RaiseModule(std::string("Fiducials").c_str());
}



//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::EditorButtonCallback()
{
  this->RaiseModule(std::string("Editor").c_str());
}


void vtkLiverAblationPlanningStep::SegmentationButtonCallback()
{
  this->RaiseModule(std::string("FastMarchingSegmentation").c_str());
}



void vtkLiverAblationPlanningStep::RaiseModule(const char *moduleName)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName(moduleName);        
  if ( currentModule )
    {
    currentModule->Enter( );
    currentModule->GetUIPanel()->Raise();
    //this->SlicerAppGUI->GetMainSlicerWindow()->SetStatusText (moduleName);
//    this->GetGUI()->GetSlicerAppGUI()->GetMainSlicerWindow()->SetStatusText (moduleName);
    }
}


