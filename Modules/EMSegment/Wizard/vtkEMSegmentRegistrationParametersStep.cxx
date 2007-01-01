#include "vtkEMSegmentRegistrationParametersStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentRegistrationParametersStep);
vtkCxxRevisionMacro(vtkEMSegmentRegistrationParametersStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentRegistrationParametersStep::vtkEMSegmentRegistrationParametersStep()
{
  this->SetName("7/8. Edit Registration Parameters");
  this->SetDescription("Specify atlas-to-target registration parameters.");

  this->RegistrationParametersFrame                   = NULL;
  this->RegistrationParametersAtlasImageMenuButton    = NULL;
  this->RegistrationParametersTargetImageMenuButton   = NULL;
  this->RegistrationParametersAffineMenuButton        = NULL;
  this->RegistrationParametersDeformableMenuButton    = NULL;
  this->RegistrationParametersInterpolationMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentRegistrationParametersStep::~vtkEMSegmentRegistrationParametersStep()
{
  if (this->RegistrationParametersAtlasImageMenuButton)
    {
    this->RegistrationParametersAtlasImageMenuButton->Delete();
    this->RegistrationParametersAtlasImageMenuButton = NULL;
    }

  if (this->RegistrationParametersTargetImageMenuButton)
    {
    this->RegistrationParametersTargetImageMenuButton->Delete();
    this->RegistrationParametersTargetImageMenuButton = NULL;
    }

  if (this->RegistrationParametersAffineMenuButton)
    {
    this->RegistrationParametersAffineMenuButton->Delete();
    this->RegistrationParametersAffineMenuButton = NULL;
    }

  if (this->RegistrationParametersDeformableMenuButton)
    {
    this->RegistrationParametersDeformableMenuButton->Delete();
    this->RegistrationParametersDeformableMenuButton = NULL;
    }

  if (this->RegistrationParametersInterpolationMenuButton)
    {
    this->RegistrationParametersInterpolationMenuButton->Delete();
    this->RegistrationParametersInterpolationMenuButton = NULL;
    }

  if (this->RegistrationParametersFrame)
    {
    this->RegistrationParametersFrame->Delete();
    this->RegistrationParametersFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Create the frame

  if (!this->RegistrationParametersFrame)
    {
    this->RegistrationParametersFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RegistrationParametersFrame->IsCreated())
    {
    this->RegistrationParametersFrame->SetParent(parent);
    this->RegistrationParametersFrame->Create();
    this->RegistrationParametersFrame->SetLabelText(
      "Atlas-to-target Registration Parameters");
    }
  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->RegistrationParametersFrame->GetWidgetName());

  char buffer[256];

  int enabled = parent->GetEnabled();

  // Create the atlas image volume selector

  if (!this->RegistrationParametersAtlasImageMenuButton)
    {
    this->RegistrationParametersAtlasImageMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersAtlasImageMenuButton->IsCreated())
    {
    this->RegistrationParametersAtlasImageMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersAtlasImageMenuButton->Create();
    this->RegistrationParametersAtlasImageMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersAtlasImageMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersAtlasImageMenuButton->
      SetLabelText("Atlas Image:");
    this->RegistrationParametersAtlasImageMenuButton->
      SetBalloonHelpString("Select volume for the atlas image.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersAtlasImageMenuButton->GetWidgetName());
  
  this->PopulateMenuWithLoadedVolumes(
    this->RegistrationParametersAtlasImageMenuButton->
    GetWidget()->GetMenu(), this,"RegistrationAtlasImageCallback");

  if(!logic->GetVolumeNumberOfChoices() ||
     !this->SetMenuButtonSelectedItem(
       this->RegistrationParametersAtlasImageMenuButton->
       GetWidget()->GetMenu(), logic->GetRegistrationAtlasVolumeID()))
    {
    this->RegistrationParametersAtlasImageMenuButton->
      GetWidget()->SetValue("");
    }
  this->RegistrationParametersAtlasImageMenuButton->SetEnabled(
    logic->GetVolumeNumberOfChoices() ? enabled : 0);

  // Create the target image volume selector

  if (!this->RegistrationParametersTargetImageMenuButton)
    {
    this->RegistrationParametersTargetImageMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersTargetImageMenuButton->IsCreated())
    {
    this->RegistrationParametersTargetImageMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersTargetImageMenuButton->Create();
    this->RegistrationParametersTargetImageMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersTargetImageMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersTargetImageMenuButton->
      SetLabelText("Target Image:");
    this->RegistrationParametersTargetImageMenuButton->
      SetBalloonHelpString("Select volume for the target image.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersTargetImageMenuButton->GetWidgetName());
  
  this->PopulateMenuWithTargetVolumes(
    this->RegistrationParametersTargetImageMenuButton->
    GetWidget()->GetMenu(), this, "RegistrationTargetImageCallback");

  if(!logic->GetTargetNumberOfSelectedVolumes() ||
     !this->SetMenuButtonSelectedItem(
       this->RegistrationParametersTargetImageMenuButton->
       GetWidget()->GetMenu(), logic->GetRegistrationTargetVolumeID()))
    {
    this->RegistrationParametersTargetImageMenuButton->
      GetWidget()->SetValue("");
    }
  this->RegistrationParametersTargetImageMenuButton->SetEnabled(
    logic->GetTargetNumberOfSelectedVolumes() ? enabled : 0);

  // Create the affine registration menu button

  if (!this->RegistrationParametersAffineMenuButton)
    {
    this->RegistrationParametersAffineMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersAffineMenuButton->IsCreated())
    {
    this->RegistrationParametersAffineMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersAffineMenuButton->Create();
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersAffineMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersAffineMenuButton->
      SetLabelText("Affine Registration:");

    // Need Brad to define the enum types
    sprintf(buffer, "RegistrationAffineCallback %d", 0);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("0", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 1);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("1", this, buffer);

    this->RegistrationParametersAffineMenuButton->SetBalloonHelpString(
      "Select affine registration.");
    }
  //Disable for now
  //sprintf(buffer, "%d", logic->GetRegistrationAffineType());
  //this->RegistrationParametersAffineMenuButton->GetWidget()->SetValue(buffer);
  //this->RegistrationParametersAffineMenuButton->SetEnabled(
  //  logic->HasGlobalParametersNode() ? enabled : 0);
  
  this->RegistrationParametersAffineMenuButton->SetEnabled(0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersAffineMenuButton->GetWidgetName());

  // Create the deformable registration menu button

  if (!this->RegistrationParametersDeformableMenuButton)
    {
    this->RegistrationParametersDeformableMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersDeformableMenuButton->IsCreated())
    {
    this->RegistrationParametersDeformableMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersDeformableMenuButton->Create();
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersDeformableMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersDeformableMenuButton->
      SetLabelText("Deformable Registration:");
    this->RegistrationParametersDeformableMenuButton->SetBalloonHelpString(
      "Select deformable registration.");

    // Need Brad to define the enum types
    sprintf(buffer, "RegistrationDeformableCallback %d", 0);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("0", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d", 1);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("1", this, buffer);
    }
  //Disable for now
  //sprintf(buffer, "%d", logic->GetRegistrationDeformableType());
  //this->RegistrationParametersDeformableMenuButton->GetWidget()->SetValue(
  //  buffer);
  //this->RegistrationParametersDeformableMenuButton->SetEnabled(
  //  logic->HasGlobalParametersNode() ? enabled : 0);

  this->RegistrationParametersDeformableMenuButton->SetEnabled(0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersDeformableMenuButton->GetWidgetName());
  
  // Create the interpolation menu button

  if (!this->RegistrationParametersInterpolationMenuButton)
    {
    this->RegistrationParametersInterpolationMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersInterpolationMenuButton->IsCreated())
    {
    this->RegistrationParametersInterpolationMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersInterpolationMenuButton->Create();
    this->RegistrationParametersInterpolationMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersInterpolationMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersInterpolationMenuButton->
      SetLabelText("Interpolation:");
    this->RegistrationParametersInterpolationMenuButton->SetBalloonHelpString(
      "Select interpolation type.");

    sprintf(buffer, "RegistrationInterpolationCallback %d", 
            vtkEMSegmentLogic::InterpolationNearestNeighbor);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Nearest Neighbor", this, buffer);
    sprintf(buffer, "RegistrationInterpolationCallback %d", 
            vtkEMSegmentLogic::InterpolationLinear);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Linear", this, buffer);
    // !!! cubic interpolation is not currently supported by algorithm
    //     sprintf(buffer, "RegistrationInterpolationCallback %d", 
    //             vtkEMSegmentLogic::InterpolationCubic);
    //     this->RegistrationParametersInterpolationMenuButton->
    //       GetWidget()->GetMenu()->AddRadioButton("Cubic", this, buffer);
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersInterpolationMenuButton->GetWidgetName());

  vtksys_stl::string value;
  int v = logic->GetRegistrationInterpolationType();
  if (v == vtkEMSegmentLogic::InterpolationNearestNeighbor)
    {
    value = "Nearest Neighbor";
    }
  else if (v == vtkEMSegmentLogic::InterpolationLinear)
    {
    value = "Linear";
    }
  else if (v == vtkEMSegmentLogic::InterpolationCubic)
    {
    value = "Cubic";
    }
  this->RegistrationParametersInterpolationMenuButton->GetWidget()->SetValue(
    value.c_str());
  this->RegistrationParametersInterpolationMenuButton->SetEnabled(
    logic->HasGlobalParametersNode() ? enabled : 0);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAtlasImageCallback(
  vtkIdType volume_id)
{
  // The atlas image has changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->SetRegistrationAtlasVolumeID(volume_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationTargetImageCallback(
  vtkIdType volume_id)
{
  // The target image has changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->SetRegistrationTargetVolumeID(volume_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAffineCallback(
  int type)
{
  // The affine registration type has changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->SetRegistrationAffineType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationDeformableCallback(
  int type)
{
  // The deformable registration type has changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->SetRegistrationDeformableType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationInterpolationCallback(
  int type)
{
  // The interpolation type has changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->SetRegistrationInterpolationType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
