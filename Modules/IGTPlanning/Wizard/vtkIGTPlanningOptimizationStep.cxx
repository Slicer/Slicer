#include "vtkIGTPlanningOptimizationStep.h"

#include "vtkIGTPlanningGUI.h"
#include "vtkIGTPlanningMRMLManager.h"

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
vtkStandardNewMacro(vtkIGTPlanningOptimizationStep);
vtkCxxRevisionMacro(vtkIGTPlanningOptimizationStep, "$Revision: 1.4 $");

//----------------------------------------------------------------------------
vtkIGTPlanningOptimizationStep::vtkIGTPlanningOptimizationStep()
{
  this->SetName("3/3. Optimization");
  this->SetDescription("Run the optimization program.");

  this->RunFrame = NULL;

}



//----------------------------------------------------------------------------
vtkIGTPlanningOptimizationStep::~vtkIGTPlanningOptimizationStep()
{
  if(this->RunFrame)
    {
    this->RunFrame->Delete();
    this->RunFrame = NULL;
    }

  if (this->RunButton)
    {
    this->RunButton->SetParent(NULL);
    this->RunButton->Delete();
    this->RunButton = NULL;
    }
  if (this->GoToNavButton)
    {
    this->GoToNavButton->SetParent(NULL);
    this->GoToNavButton->Delete();
    this->GoToNavButton = NULL;
    }
}



//----------------------------------------------------------------------------
void vtkIGTPlanningOptimizationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  // Export and run frame
  // ======================================================================
  if (!this->RunFrame)
    {
    this->RunFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RunFrame->IsCreated())
    {
    this->RunFrame->SetParent(
      wizard_widget->GetClientArea());
    this->RunFrame->Create();
    this->RunFrame->SetLabelText("Run");
//    this->RunFrame->SetHeight(50);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->RunFrame->GetWidgetName());

  this->RunButton = vtkKWPushButton::New();
  this->RunButton->SetParent (this->RunFrame->GetFrame());
  this->RunButton->Create();
  this->RunButton->SetText("Run");
  this->RunButton->SetCommand(this, "RunButtonCallback");
  this->RunButton->SetWidth(20);

  this->GoToNavButton = vtkKWPushButton::New();
  this->GoToNavButton->SetParent (this->RunFrame->GetFrame());
  this->GoToNavButton->Create();
  this->GoToNavButton->SetText("Go to Navigation");
  this->GoToNavButton->SetCommand(this, "GoToNavButtonCallback");
  this->GoToNavButton->SetWidth(20);

  this->Script("pack %s %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->RunButton->GetWidgetName(),
               this->GoToNavButton->GetWidgetName()
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


void vtkIGTPlanningOptimizationStep::RunButtonCallback()
{

}



void vtkIGTPlanningOptimizationStep::GoToNavButtonCallback()
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
void vtkIGTPlanningOptimizationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
