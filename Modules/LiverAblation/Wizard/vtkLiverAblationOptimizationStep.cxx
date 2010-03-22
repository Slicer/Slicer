#include "vtkLiverAblationOptimizationStep.h"

#include "vtkLiverAblationGUI.h"
#include "vtkLiverAblationMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWMessageDialog.h" 
#include "vtkSlicerApplication.h" 

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLiverAblationOptimizationStep);
vtkCxxRevisionMacro(vtkLiverAblationOptimizationStep, "$Revision: 1.4 $");

//----------------------------------------------------------------------------
vtkLiverAblationOptimizationStep::vtkLiverAblationOptimizationStep()
{
  this->SetName("4/6. Optimization and Plan Loading");
  this->SetDescription("Run the optimization program and then load the surgical plan.");

  this->OptimizeButton = NULL;
  this->LoadButton = NULL;

}



//----------------------------------------------------------------------------
vtkLiverAblationOptimizationStep::~vtkLiverAblationOptimizationStep()
{
  if (this->OptimizeButton)
    {
    this->OptimizeButton->SetParent(NULL);
    this->OptimizeButton->Delete();
    this->OptimizeButton = NULL;
    }
  if (this->LoadButton)
    {
    this->LoadButton->SetParent(NULL);
    this->LoadButton->Delete();
    this->LoadButton = NULL;
    }
}



//----------------------------------------------------------------------------
void vtkLiverAblationOptimizationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  if (! this->OptimizeButton)
    {
    this->OptimizeButton = vtkKWPushButton::New();
    this->OptimizeButton->SetParent(wizard_widget->GetClientArea());
    this->OptimizeButton->Create();
    this->OptimizeButton->SetText("Optimize");
    this->OptimizeButton->SetCommand(this, "OptimizeButtonCallback");
    this->OptimizeButton->SetWidth(25);
    }

  if (! this->LoadButton)
    {
    this->LoadButton = vtkKWPushButton::New();
    this->LoadButton = vtkKWPushButton::New();
    this->LoadButton->SetParent(wizard_widget->GetClientArea());
    this->LoadButton->Create();
    this->LoadButton->SetText("Load Plan");
    this->LoadButton->SetCommand(this, "LoadButtonCallback");
    this->LoadButton->SetWidth(25);
    }

  this->Script("pack %s %s -side top -anchor center -expand n -padx 2 -pady 5", 
               this->OptimizeButton->GetWidgetName(),
               this->LoadButton->GetWidgetName()
               );
 
  //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Optimization step");
  msg_dlg1->SetText( "This is the first step in IGT applications. This step includes establishing "
                     "communication with Planning system, tracking device, endoscope, external tracking "
                     "software such as IGSTK/OpenIGT Lin and robot controller if available");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();
}


void vtkLiverAblationOptimizationStep::OptimizeButtonCallback()
{

}



void vtkLiverAblationOptimizationStep::LoadButtonCallback()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  std::string name = "IGT Navigation";
  vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName(name.c_str());        
  if ( currentModule )
    {
    currentModule->Enter( );
    currentModule->GetUIPanel()->Raise();
    this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (name.c_str());
    }
}



//----------------------------------------------------------------------------
void vtkLiverAblationOptimizationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
