#include "vtkLiverAblationCalibrationStep.h"

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
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLiverAblationCalibrationStep);
vtkCxxRevisionMacro(vtkLiverAblationCalibrationStep, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkLiverAblationCalibrationStep::vtkLiverAblationCalibrationStep()
{
  this->SetName("3/3. Calibration");
  this->SetDescription("Calibrate required tools such as tracker tool.");

  this->LoadPrecomputedCalibrationDataFrame = NULL;
  this->CalibrateFrame = NULL;
  this->TrackingFrame = NULL;
  this->EndoscopeCameraFrame = NULL;

  this->CalibrationDataMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkLiverAblationCalibrationStep::~vtkLiverAblationCalibrationStep()
{
  if( this->LoadPrecomputedCalibrationDataFrame)
    {
    this->LoadPrecomputedCalibrationDataFrame->Delete();
    this->LoadPrecomputedCalibrationDataFrame = NULL;
    }

  if( this->CalibrateFrame)
    {
    this->CalibrateFrame->Delete();
    this->CalibrateFrame = NULL;
    }

  if( this->TrackingFrame)
    {
    this->TrackingFrame->Delete();
    this->TrackingFrame = NULL;
    }

  if( this->EndoscopeCameraFrame)
    {
    this->EndoscopeCameraFrame->Delete();
    this->EndoscopeCameraFrame = NULL;
    }

  if( this->CalibrationDataMenuButton)
    {
    this->CalibrationDataMenuButton->Delete();
    this->CalibrationDataMenuButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkLiverAblationCalibrationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  if (!this->LoadPrecomputedCalibrationDataFrame)
    {
    this->LoadPrecomputedCalibrationDataFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->LoadPrecomputedCalibrationDataFrame->IsCreated())
    {
    this->LoadPrecomputedCalibrationDataFrame->SetParent(
      wizard_widget->GetClientArea());
    this->LoadPrecomputedCalibrationDataFrame->Create();
    this->LoadPrecomputedCalibrationDataFrame->SetLabelText("Load precomputed calibration data ");
    this->LoadPrecomputedCalibrationDataFrame->SetHeight(100);
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->LoadPrecomputedCalibrationDataFrame->GetWidgetName());

  if (!this->CalibrationDataMenuButton)
    {
    this->CalibrationDataMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->CalibrationDataMenuButton->IsCreated())
    {
    this->CalibrationDataMenuButton->SetParent(LoadPrecomputedCalibrationDataFrame->GetFrame());
    this->CalibrationDataMenuButton->Create();
    this->CalibrationDataMenuButton->
      SetLabelText("Precomputed calibration data :");
    this->CalibrationDataMenuButton->
      SetBalloonHelpString("Select precomputed calibration data.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 5", 
    this->CalibrationDataMenuButton->GetWidgetName());
  
  this->PopulateCalibrationDataSelector();

  if (!this->CalibrateFrame)
    {
    this->CalibrateFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->CalibrateFrame->IsCreated())
    {
    this->CalibrateFrame->SetParent(
      wizard_widget->GetClientArea());
    this->CalibrateFrame->Create();
    this->CalibrateFrame->SetLabelText("Perform Calibration");
    this->CalibrateFrame->SetHeight( 100 );

    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->CalibrateFrame->GetWidgetName());

  if (!this->TrackingFrame)
    {
    this->TrackingFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->TrackingFrame->IsCreated())
    {
    this->TrackingFrame->SetParent(
      CalibrateFrame->GetFrame());
    this->TrackingFrame->Create();
    this->TrackingFrame->SetLabelText("Tracking Tools");
    this->TrackingFrame->SetHeight( 50 );
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->TrackingFrame->GetWidgetName());


  if (!this->EndoscopeCameraFrame)
    {
    this->EndoscopeCameraFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->EndoscopeCameraFrame->IsCreated())
    {
    this->EndoscopeCameraFrame->SetParent(
      CalibrateFrame->GetFrame());
    this->EndoscopeCameraFrame->Create();
    this->EndoscopeCameraFrame->SetLabelText("Endoscope Camera");
    this->EndoscopeCameraFrame->SetHeight( 50 );
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->EndoscopeCameraFrame->GetWidgetName());

  //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Calibration Step");
  msg_dlg1->SetText( "This is the third step in IGT applications. In this step, the user can "
                     "undertake calibration or load precomputed calibration data ");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}

//----------------------------------------------------------------------------
void vtkLiverAblationCalibrationStep::PopulateCalibrationDataSelector()
{
}

//----------------------------------------------------------------------------
void vtkLiverAblationCalibrationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
