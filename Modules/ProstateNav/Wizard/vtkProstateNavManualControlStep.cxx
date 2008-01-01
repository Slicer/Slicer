#include "vtkProstateNavManualControlStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavManualControlStep);
vtkCxxRevisionMacro(vtkProstateNavManualControlStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavManualControlStep::vtkProstateNavManualControlStep()
{
  this->SetName("5/5. Manual Control");
  this->SetDescription("Insert the needle.");

  this->MainFrame = NULL;
  this->ControlFrame = NULL;
  this->PRScale = NULL;
  this->PAScale = NULL;
  this->PSScale = NULL;
  this->NRScale = NULL;
  this->NAScale = NULL;
  this->NSScale = NULL;

}

//----------------------------------------------------------------------------
vtkProstateNavManualControlStep::~vtkProstateNavManualControlStep()
{
}

//----------------------------------------------------------------------------
void vtkProstateNavManualControlStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  if (!this->MainFrame)
    {
    this->MainFrame = vtkKWFrame::New();
    this->MainFrame->SetParent(parent);
    this->MainFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->MainFrame->GetWidgetName());

  if (!this->ControlFrame)
    {
    this->ControlFrame = vtkKWFrame::New();
    this->ControlFrame->SetParent(this->MainFrame);
    this->ControlFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->ControlFrame->GetWidgetName());

  int min = -150;
  int max = 150;
  float res = 0.5;
  if (!this->PRScale)
    {
    this->PRScale = vtkKWScaleWithEntry::New();
    this->PRScale->SetParent(ControlFrame);
    this->PRScale->Create();
    this->PRScale->SetLabelText("POS L(-)-R(+):");
    this->PRScale->SetBalloonHelpString ( "Set the position in R-L direction.");
    this->PRScale->ExpandWidgetOn();
    this->PRScale->SetLabelWidth(15);
    this->PRScale->SetRange(min, max);
    this->PRScale->ClampValueOn();
    this->PRScale->GetWidget()->SetRange((float)min, (float)max);
    this->PRScale->GetWidget()->SetOrientationToHorizontal();
    this->PRScale->GetWidget()->SetResolution(res);
    this->PRScale->SetEntryWidth(6);
    }

  if (!this->PAScale)
    {
    this->PAScale = vtkKWScaleWithEntry::New();
    this->PAScale->SetParent(ControlFrame);
    this->PAScale->Create();
    this->PAScale->SetLabelText("POS P(-)-A(+):");
    this->PAScale->SetBalloonHelpString ( "Set the position in A-P direction.");
    this->PAScale->ExpandWidgetOn();
    this->PAScale->SetLabelWidth(15);
    this->PAScale->SetRange(min, max);
    this->PAScale->ClampValueOn();
    this->PAScale->GetWidget()->SetRange((float)min, (float)max);
    this->PAScale->GetWidget()->SetOrientationToHorizontal();
    this->PAScale->GetWidget()->SetResolution(res);
    this->PAScale->SetEntryWidth(6);
    }
  if (!this->PSScale)
    {
    this->PSScale = vtkKWScaleWithEntry::New();
    this->PSScale->SetParent(ControlFrame);
    this->PSScale->Create();
    this->PSScale->SetLabelText("POS I(-)-S(+):");
    this->PSScale->SetBalloonHelpString ( "Set the position in S-I direction.");
    this->PSScale->ExpandWidgetOn();
    this->PSScale->SetLabelWidth(15);
    this->PSScale->SetRange(min, max);
    this->PSScale->ClampValueOn();
    this->PSScale->GetWidget()->SetRange((float)min, (float)max);
    this->PSScale->GetWidget()->SetOrientationToHorizontal();
    this->PSScale->GetWidget()->SetResolution(res);
    this->PSScale->SetEntryWidth(6);
    }

  min = -1;
  max = 1;
  res = 0.001;
  if (!this->NRScale)
    {
    this->NRScale = vtkKWScaleWithEntry::New();
    this->NRScale->SetParent(ControlFrame);
    this->NRScale->Create();
    this->NRScale->SetLabelText("NORM L(-)-R(+):");
    this->NRScale->SetBalloonHelpString ( "Set the normal in R-L direction.");
    this->NRScale->ExpandWidgetOn();
    this->NRScale->SetLabelWidth(15);
    this->NRScale->SetRange(min, max);
    this->NRScale->ClampValueOn();
    this->NRScale->GetWidget()->SetRange((float)min, (float)max);
    this->NRScale->GetWidget()->SetOrientationToHorizontal();
    this->NRScale->GetWidget()->SetResolution(res);
    this->NRScale->SetEntryWidth(6);
    }
  if (!this->NAScale)
    {
    this->NAScale = vtkKWScaleWithEntry::New();
    this->NAScale->SetParent(ControlFrame);
    this->NAScale->Create();
    this->NAScale->SetLabelText("NORM P(-)-A(+):");
    this->NAScale->SetBalloonHelpString ( "Set the normal in A-P direction.");
    this->NAScale->ExpandWidgetOn();
    this->NAScale->SetLabelWidth(15);
    this->NAScale->SetRange(min, max);
    this->NAScale->ClampValueOn();
    this->NAScale->GetWidget()->SetRange((float)min, (float)max);
    this->NAScale->GetWidget()->SetOrientationToHorizontal();
    this->NAScale->GetWidget()->SetResolution(res);
    this->NAScale->SetEntryWidth(6);
    }
  if (!this->NSScale)
    {
    this->NSScale = vtkKWScaleWithEntry::New();
    this->NSScale->SetParent(ControlFrame);
    this->NSScale->Create();
    this->NSScale->SetLabelText("NORM I(-)-S(+):");
    this->NSScale->SetBalloonHelpString ( "Set the normal in S-I direction.");
    this->NSScale->ExpandWidgetOn();
    this->NSScale->SetLabelWidth(15);
    this->NSScale->SetRange(min, max);
    this->NSScale->ClampValueOn();
    this->NSScale->GetWidget()->SetRange((float)min, (float)max);
    this->NSScale->GetWidget()->SetOrientationToHorizontal();
    this->NSScale->GetWidget()->SetResolution(res);
    this->NSScale->SetEntryWidth(6);
    }

  this->Script ("pack %s %s %s %s %s %s -side top -anchor nw -fill x -pady 0 -in %s",
                this->PRScale->GetWidgetName(),
                this->PAScale->GetWidgetName(),
                this->PSScale->GetWidgetName(),
                this->NRScale->GetWidgetName(),
                this->NAScale->GetWidgetName(),
                this->NSScale->GetWidgetName(),
                ControlFrame->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkProstateNavManualControlStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavManualControlStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{
}
