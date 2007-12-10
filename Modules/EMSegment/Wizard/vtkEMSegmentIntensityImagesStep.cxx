#include "vtkEMSegmentIntensityImagesStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWListBoxToListBoxSelectionEditor.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"
#include "vtkKWCheckButtonWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityImagesStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityImagesStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityImagesStep::vtkEMSegmentIntensityImagesStep()
{
  this->SetName("4/9. Select Target Images");
  this->SetDescription("Choose the set of images that will be segmented.");

  this->IntensityImagesTargetSelectorFrame          = NULL;
  this->IntensityImagesTargetVolumeSelector         = NULL;
  this->TargetToTargetRegistrationFrame             = NULL;
  this->IntensityImagesAlignTargetImagesCheckButton = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityImagesStep::~vtkEMSegmentIntensityImagesStep()
{
  if (this->IntensityImagesTargetVolumeSelector)
    {
    this->IntensityImagesTargetVolumeSelector->Delete();
    this->IntensityImagesTargetVolumeSelector = NULL;
    }

  if (this->IntensityImagesTargetSelectorFrame)
    {
    this->IntensityImagesTargetSelectorFrame->Delete();
    this->IntensityImagesTargetSelectorFrame = NULL;
    }

  if (this->TargetToTargetRegistrationFrame)
    {
    this->TargetToTargetRegistrationFrame->Delete();
    this->TargetToTargetRegistrationFrame = NULL;
    }

  if (this->IntensityImagesAlignTargetImagesCheckButton)
    {
    this->IntensityImagesAlignTargetImagesCheckButton->Delete();
    this->IntensityImagesAlignTargetImagesCheckButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();
  int enabled = parent->GetEnabled();

  // Create the frame

  if (!this->IntensityImagesTargetSelectorFrame)
    {
    this->IntensityImagesTargetSelectorFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->IntensityImagesTargetSelectorFrame->IsCreated())
    {
    this->IntensityImagesTargetSelectorFrame->SetParent(parent);
    this->IntensityImagesTargetSelectorFrame->Create();
    this->IntensityImagesTargetSelectorFrame->SetLabelText("Target Images");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->IntensityImagesTargetSelectorFrame->GetWidgetName());

  // Create the target images volume selector

  if (!this->IntensityImagesTargetVolumeSelector)
    {
    this->IntensityImagesTargetVolumeSelector = 
      vtkKWListBoxToListBoxSelectionEditor::New();
    }
  if (!this->IntensityImagesTargetVolumeSelector->IsCreated())
    {
    this->IntensityImagesTargetVolumeSelector->SetParent(
      this->IntensityImagesTargetSelectorFrame->GetFrame());
    this->IntensityImagesTargetVolumeSelector->Create();
    vtkKWListBoxWithScrollbarsWithLabel *listbox = 
      this->IntensityImagesTargetVolumeSelector->GetSourceList();
    listbox->SetLabelVisibility(1);
    listbox->SetLabelText("Available Images:");
    listbox = 
      this->IntensityImagesTargetVolumeSelector->GetFinalList();
    listbox->SetLabelVisibility(1);
    listbox->SetLabelText("Selected Images:");
    this->IntensityImagesTargetVolumeSelector->AllowReorderingOn();
    this->IntensityImagesTargetVolumeSelector->SetFinalListChangedCommand(
      this, "IntensityImagesTargetSelectionChangedCallback");
    }

  this->Script(
    "pack %s -side top -anchor nw -expand true -fill x -padx 2 -pady 2", 
    this->IntensityImagesTargetVolumeSelector->GetWidgetName());

  // Update the UI with the proper value

  this->PopulateIntensityImagesTargetVolumeSelector();

  // Create the target-to-target registration frame

  if (!this->TargetToTargetRegistrationFrame)
    {
    this->TargetToTargetRegistrationFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->TargetToTargetRegistrationFrame->IsCreated())
    {
    this->TargetToTargetRegistrationFrame->SetParent(parent);
    this->TargetToTargetRegistrationFrame->Create();
    this->TargetToTargetRegistrationFrame->SetLabelText("Target-to-target Registration");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->TargetToTargetRegistrationFrame->GetWidgetName());

  if (!this->IntensityImagesAlignTargetImagesCheckButton)
    {
    this->IntensityImagesAlignTargetImagesCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->IntensityImagesAlignTargetImagesCheckButton->IsCreated())
    {
    this->IntensityImagesAlignTargetImagesCheckButton->SetParent(
      this->TargetToTargetRegistrationFrame->GetFrame());
    this->IntensityImagesAlignTargetImagesCheckButton->Create();
    this->IntensityImagesAlignTargetImagesCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityImagesAlignTargetImagesCheckButton->
      SetLabelText("Align Target Images:");
    this->IntensityImagesAlignTargetImagesCheckButton->
      GetWidget()->SetCommand(this, "AlignTargetImagesCallback");
    }
  this->IntensityImagesAlignTargetImagesCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->IntensityImagesAlignTargetImagesCheckButton->GetWidgetName());

  this->IntensityImagesAlignTargetImagesCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetEnableTargetToTargetRegistration());

  wizard_widget->SetErrorText(
    "Please note that the order of the images is important.");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::PopulateIntensityImagesTargetVolumeSelector()
{  
  vtkIdType vol_id, target_vol_id;
  char buffer[256];
  bool found = false;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  int nb_of_volumes = mrmlManager->GetVolumeNumberOfChoices();
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  // clear the lists 
  this->IntensityImagesTargetVolumeSelector->RemoveItemsFromSourceList();
  this->IntensityImagesTargetVolumeSelector->RemoveItemsFromFinalList();

  // Update the source volume list 
  for (int index = 0; index < nb_of_volumes; index++)
    {
    vol_id = mrmlManager->GetVolumeNthID(index);
    found = false;
    for(int i = 0; i < nb_of_target_volumes; i++)
      {
      target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
      if (vol_id == target_vol_id)
        {
        found = true;
        break;
        }
      }
    if (!found)
      {
      const char *name = mrmlManager->GetVolumeName(vol_id);
      if (name)
        {
        sprintf(buffer, "%s (%d)", name, static_cast<int>(vol_id));
        this->IntensityImagesTargetVolumeSelector->AddSourceElement(buffer);
        }
      }
    }

  // Update the target volume list
  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
      {
      sprintf(buffer, "%s (%d)", name, static_cast<int>(target_vol_id));
      this->IntensityImagesTargetVolumeSelector->AddFinalElement(buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::
  IntensityImagesTargetSelectionChangedCallback()
{
  // nothing for now; changes are made on transitions
  // (see svn revisions)
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::
AlignTargetImagesCallback(int state)
{
  // The align target images checkbutton has changed because of user
  // interaction
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetEnableTargetToTargetRegistration(state);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  // decide if the number of target volumes changed
  unsigned int nb_of_parameter_target_volumes = 
    mrmlManager->GetTargetNumberOfSelectedVolumes();
  unsigned int nb_of_currently_selected_target_volumes = 
    this->IntensityImagesTargetVolumeSelector->
    GetNumberOfElementsOnFinalList();
  bool number_of_target_images_changed = 
    nb_of_parameter_target_volumes != nb_of_currently_selected_target_volumes;

  if (number_of_target_images_changed &&
      !vtkKWMessageDialog::PopupYesNo
      (this->GetApplication(), 
       NULL, 
       "Change the number of target images?",
       "Are you sure you want to change the number of target images?",
       vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
    {
    // don't change number of volumes; stay on this step
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    wizard_workflow->ProcessInputs();
    }
  else
    {
    // record indices of currently selected volumes
    std::vector<vtkIdType> selectedIDs;
    for(unsigned int i = 0; i < nb_of_currently_selected_target_volumes; ++i) 
      {
      std::string targettext = 
        this->IntensityImagesTargetVolumeSelector->GetElementFromFinalList(i);
      std::string::size_type pos1 = targettext.rfind("(");
      std::string::size_type pos2 = targettext.rfind(")");
      if (pos1 != vtksys_stl::string::npos && pos2 != vtksys_stl::string::npos)
        {
        vtkIdType vol_id = atoi(targettext.substr(pos1+1, pos2-pos1-1).c_str());
        selectedIDs.push_back(vol_id);
        }
      }
    mrmlManager->ResetTargetSelectedVolumes(selectedIDs);
    this->Superclass::Validate();
    }
}


//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
