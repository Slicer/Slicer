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


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIGTPlanningOptimizationStep);
vtkCxxRevisionMacro(vtkIGTPlanningOptimizationStep, "$Revision: 1.4 $");

//----------------------------------------------------------------------------
vtkIGTPlanningOptimizationStep::vtkIGTPlanningOptimizationStep()
{
  this->SetName("2/3. Optimization");
  this->SetDescription("Collect user information and run the optimization program.");

  this->ProbeFrame = NULL;
  this->SpacingFrame = NULL;
  this->MoreFrame = NULL;
  this->RunFrame = NULL;

  this->ProbeAEntry   = vtkKWEntryWithLabel::New();
  this->ProbeBEntry   = vtkKWEntryWithLabel::New();
  this->ProbeCEntry   = vtkKWEntryWithLabel::New();
  this->SpacingXEntry = vtkKWEntryWithLabel::New();
  this->SpacingYEntry = vtkKWEntryWithLabel::New();
  this->SpacingZEntry = vtkKWEntryWithLabel::New();

}



//----------------------------------------------------------------------------
vtkIGTPlanningOptimizationStep::~vtkIGTPlanningOptimizationStep()
{
  if(this->ProbeFrame)
    {
    this->ProbeFrame->Delete();
    this->ProbeFrame = NULL;
    }
  if(this->SpacingFrame)
    {
    this->SpacingFrame->Delete();
    this->SpacingFrame = NULL;
    }
  if(this->MoreFrame)
    {
    this->MoreFrame->Delete();
    this->MoreFrame = NULL;
    }
  if(this->RunFrame)
    {
    this->RunFrame->Delete();
    this->RunFrame = NULL;
    }

  if (this->ProbeAEntry) 
  {
    this->ProbeAEntry->SetParent(NULL);
    this->ProbeAEntry->Delete();
    this->ProbeAEntry = NULL;
  }
  if (this->ProbeBEntry) 
  {
    this->ProbeBEntry->SetParent(NULL);
    this->ProbeBEntry->Delete();
    this->ProbeBEntry = NULL;
  }
  if (this->ProbeCEntry) 
  {
    this->ProbeCEntry->SetParent(NULL);
    this->ProbeCEntry->Delete();
    this->ProbeCEntry = NULL;
  }

  if (this->SpacingXEntry) 
  {
    this->SpacingXEntry->SetParent(NULL);
    this->SpacingXEntry->Delete();
    this->SpacingXEntry = NULL;
  }
  if (this->SpacingYEntry) 
  {
    this->SpacingYEntry->SetParent(NULL);
    this->SpacingYEntry->Delete();
    this->SpacingYEntry = NULL;
  }
  if (this->SpacingZEntry) 
  {
    this->SpacingZEntry->SetParent(NULL);
    this->SpacingZEntry->Delete();
    this->SpacingZEntry = NULL;
  }

}

//----------------------------------------------------------------------------
void vtkIGTPlanningOptimizationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  // Probe information frame
  // ======================================================================
  if (!this->ProbeFrame)
    {
    this->ProbeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->ProbeFrame->IsCreated())
    {
    this->ProbeFrame->SetParent(
      wizard_widget->GetClientArea());
    this->ProbeFrame->Create();
    this->ProbeFrame->SetLabelText("Probe information [mm]");
//    this->ProbeFrame->SetHeight(100);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->ProbeFrame->GetWidgetName());


  this->ProbeAEntry->SetParent (this->ProbeFrame->GetFrame());
  this->ProbeAEntry->Create ( );
  this->ProbeAEntry->SetLabelText("a:");
// this->ProbeAEntry->GetWidget()->SetValueAsInt(100);
// this->ProbeAEntry->SetBalloonHelpString("Maximum number of seeds");

  this->ProbeBEntry->SetParent (this->ProbeFrame->GetFrame());
  this->ProbeBEntry->Create ( );
  this->ProbeBEntry->SetLabelText("b:");

  this->ProbeCEntry->SetParent (this->ProbeFrame->GetFrame());
  this->ProbeCEntry->Create ( );
  this->ProbeCEntry->SetLabelText("c:");

  this->Script ( "pack %s %s %s -side top -anchor w -expand n -padx 2 -pady 2",
                 this->ProbeAEntry->GetWidgetName(),
                 this->ProbeBEntry->GetWidgetName(),
                 this->ProbeCEntry->GetWidgetName());


  // Sample spacing frame
  // ======================================================================
  if (!this->SpacingFrame)
    {
    this->SpacingFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SpacingFrame->IsCreated())
    {
    this->SpacingFrame->SetParent(
      wizard_widget->GetClientArea());
    this->SpacingFrame->Create();
    this->SpacingFrame->SetLabelText("Sample spacing [mm]");
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->SpacingFrame->GetWidgetName());

  this->SpacingXEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingXEntry->Create ( );
  this->SpacingXEntry->SetLabelText("X:");
// this->SpacingXEntry->GetWidget()->SetValueAsInt(100);
// this->SpacingXEntry->SetBalloonHelpString("Maximum number of seeds");

  this->SpacingYEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingYEntry->Create ( );
  this->SpacingYEntry->SetLabelText("Y:");

  this->SpacingZEntry->SetParent (this->SpacingFrame->GetFrame());
  this->SpacingZEntry->Create ( );
  this->SpacingZEntry->SetLabelText("Z:");
 
  this->Script ( "pack %s %s %s -side top -anchor nw -expand n -padx 2 -pady 2",
                 this->SpacingXEntry->GetWidgetName(),
                 this->SpacingYEntry->GetWidgetName(),
                 this->SpacingZEntry->GetWidgetName());

  // More info frame
  // ======================================================================
  if (!this->MoreFrame)
    {
    this->MoreFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->MoreFrame->IsCreated())
    {
    this->MoreFrame->SetParent(
      wizard_widget->GetClientArea());
    this->MoreFrame->Create();
    this->MoreFrame->SetLabelText("More info");
    this->MoreFrame->SetHeight(100);
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->MoreFrame->GetWidgetName());


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
    this->MoreFrame->SetLabelText("Export and run");
    this->RunFrame->SetHeight(50);
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->RunFrame->GetWidgetName());

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

//----------------------------------------------------------------------------
void vtkIGTPlanningOptimizationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
