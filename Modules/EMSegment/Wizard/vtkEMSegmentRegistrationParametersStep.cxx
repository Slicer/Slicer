#include "vtkEMSegmentRegistrationParametersStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

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
  this->SetName("8/9. Edit Registration Parameters");
  this->SetDescription("Specify atlas-to-target registration parameters.");

  this->RegistrationParametersFrame                   = NULL;
  this->RegistrationParametersAtlasImageMenuButton    = NULL;
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

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
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
      SetLabelText("Atlas (Moving) Image:");
    this->RegistrationParametersAtlasImageMenuButton->
      SetBalloonHelpString("Select volume for the atlas image.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersAtlasImageMenuButton->GetWidgetName());
  
  this->PopulateMenuWithLoadedVolumes(
    this->RegistrationParametersAtlasImageMenuButton->
    GetWidget()->GetMenu(), this,"RegistrationAtlasImageCallback");

  if(!mrmlManager->GetVolumeNumberOfChoices() ||
     !this->SetMenuButtonSelectedItem(
       this->RegistrationParametersAtlasImageMenuButton->
       GetWidget()->GetMenu(), mrmlManager->GetRegistrationAtlasVolumeID()))
    {
    this->RegistrationParametersAtlasImageMenuButton->
      GetWidget()->SetValue("");
    }
  this->RegistrationParametersAtlasImageMenuButton->SetEnabled(
    mrmlManager->GetVolumeNumberOfChoices() ? enabled : 0);

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

    sprintf(buffer, "RegistrationAffineCallback %d",
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("None", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationCenters);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Align Image Centers", this, buffer);

    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMIFast);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, MI Fast", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMI);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, MI", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMISlow);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, MI Slow", this, buffer);

    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCFast);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, NCC Fast", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCC);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, NCC", this, buffer);
    sprintf(buffer, "RegistrationAffineCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCSlow);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("Rigid, NCC Slow", this, buffer);

    this->RegistrationParametersAffineMenuButton->SetBalloonHelpString(
      "Select affine registration.");
    }

  sprintf(buffer, "%d", mrmlManager->GetRegistrationAffineType());
  this->RegistrationParametersAffineMenuButton->GetWidget()->SetValue(buffer);
  this->RegistrationParametersAffineMenuButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);
  
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

    sprintf(buffer, "RegistrationDeformableCallback %d", 
            vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("None", this, buffer);

    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineMMIFast);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, MI Fast", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineMMI);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, MI", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineMMISlow);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, MI Slow", this, buffer);

    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineNCCFast);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, NCC Fast", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineNCC);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, NCC", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d",
            vtkEMSegmentMRMLManager::
            AtlasToTargetDeformableRegistrationBSplineNCCSlow);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("B-Spline, NCC Slow", this, buffer);
    }

  sprintf(buffer, "%d", mrmlManager->GetRegistrationDeformableType());
  this->RegistrationParametersDeformableMenuButton->GetWidget()->SetValue(
    buffer);
  this->RegistrationParametersDeformableMenuButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

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
            vtkEMSegmentMRMLManager::InterpolationNearestNeighbor);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Nearest Neighbor", this, buffer);
    sprintf(buffer, "RegistrationInterpolationCallback %d", 
            vtkEMSegmentMRMLManager::InterpolationLinear);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Linear", this, buffer);
    // no cubic for now
    //sprintf(buffer, "RegistrationInterpolationCallback %d", 
    //        vtkEMSegmentMRMLManager::InterpolationCubic);
    //this->RegistrationParametersInterpolationMenuButton->
    //  GetWidget()->GetMenu()->AddRadioButton("Cubic", this, buffer);
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersInterpolationMenuButton->GetWidgetName());

    {
    vtksys_stl::string value;
    int v = mrmlManager->GetRegistrationInterpolationType();
    if (v == vtkEMSegmentMRMLManager::InterpolationNearestNeighbor)
      {
      value = "Nearest Neighbor";
      }
    else if (v == vtkEMSegmentMRMLManager::InterpolationLinear)
      {
      value = "Linear";
      }
    else if (v == vtkEMSegmentMRMLManager::InterpolationCubic)
      {
      value = "Cubic";
      }
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->SetValue(value.c_str());
    this->RegistrationParametersInterpolationMenuButton->
      SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }

    {
    vtksys_stl::string value;
    int v = mrmlManager->GetRegistrationAffineType();
    if (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff)
      {
      value = "None";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationCenters)
      {
      value = "Align Image Centers";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMIFast)
      {
      value = "Rigid, MI Fast";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMI)
      {
      value = "Rigid, MI";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidMMISlow)
      {
      value = "Rigid, MI Slow";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCFast)
      {
      value = "Rigid, NCC Fast";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCC)
      {
      value = "Rigid, NCC";
      }
    else if 
      (v == vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationRigidNCCSlow)
      {
      value = "Rigid, NCC Slow";
      }
    this->RegistrationParametersAffineMenuButton->
      GetWidget()->SetValue(value.c_str());
    this->RegistrationParametersAffineMenuButton->
      SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }

    {
    vtksys_stl::string value;
    int v = mrmlManager->GetRegistrationDeformableType();
    if (v == vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff)
      {
      value = "None";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineMMIFast)
      {
      value = "B-Spline, MI Fast";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineMMI)
      {
      value = "B-Spline, MI";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineMMISlow)
      {
      value = "B-Spline, MI Slow";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineNCCFast)
      {
      value = "B-Spline, NCC Fast";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineNCC)
      {
      value = "B-Spline, NCC";
      }
    else if (v == vtkEMSegmentMRMLManager::
             AtlasToTargetDeformableRegistrationBSplineNCCSlow)
      {
      value = "B-Spline, NCC Slow";
      }
    this->RegistrationParametersDeformableMenuButton->
      GetWidget()->SetValue(value.c_str());
    this->RegistrationParametersDeformableMenuButton->
      SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAtlasImageCallback(
  vtkIdType volume_id)
{
  // The atlas image has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetRegistrationAtlasVolumeID(volume_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAffineCallback(
  int type)
{
  // The affine registration type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetRegistrationAffineType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationDeformableCallback(
  int type)
{
  // The deformable registration type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetRegistrationDeformableType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationInterpolationCallback(
  int type)
{
  // The interpolation type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetRegistrationInterpolationType(type);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
