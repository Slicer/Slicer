#include "vtkEMSegmentIntensityImagesStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWListBoxToListBoxSelectionEditor.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityImagesStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityImagesStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityImagesStep::vtkEMSegmentIntensityImagesStep()
{
  this->SetName("4/9. Select Target Images");
  this->SetDescription("Choose the set of images that will be segmented.");

  this->IntensityImagesTargetSelectorFrame  = NULL;
  this->IntensityImagesTargetVolumeSelector = NULL;
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

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
  
  // Update the source volume list 

  this->IntensityImagesTargetVolumeSelector->RemoveItemsFromSourceList();
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
  this->IntensityImagesTargetVolumeSelector->RemoveItemsFromFinalList();
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
  // The target volumes have changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkIdType vol_id, target_vol_id;

  vtksys_stl::string targettext;
  vtksys_stl::string::size_type pos1, pos2;
  
  // First, remove any target volumes that are not in the 
  // selected target list anymore

  bool found = false;
  unsigned int i, iTarget;
  vtksys_stl::vector<vtkIdType> remVec, selVec, addVec;

  unsigned int size = this->IntensityImagesTargetVolumeSelector->
    GetNumberOfElementsOnFinalList();
  for(i = 0; i < size; i++) 
    {
    targettext = 
      this->IntensityImagesTargetVolumeSelector->GetElementFromFinalList(i);
    pos1 = targettext.rfind("(");
    pos2 = targettext.rfind(")");
    if (pos1 != vtksys_stl::string::npos && pos2 != vtksys_stl::string::npos)
      {
      vol_id = atoi(targettext.substr(pos1+1, pos2-pos1-1).c_str());
      selVec.push_back(vol_id);
      }
    }

  unsigned int nb_of_target_volumes = 
    mrmlManager->GetTargetNumberOfSelectedVolumes();
  for(iTarget = 0; iTarget < nb_of_target_volumes; iTarget++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(iTarget);
    found = false;
    for(i = 0; i < selVec.size(); i++) 
      {
      if(target_vol_id == selVec[i])
        {
        found = true;
        break;
        }
      }
    if(!found)
      {
      remVec.push_back(target_vol_id);
      }
    }

  if(remVec.size()>0)
    {
    for(i=0; i<remVec.size(); i++)
      {
      mrmlManager->RemoveTargetSelectedVolume(remVec[i]);
      }
    }
  
  // Then, add the target volume list according to the selected list 

  nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  if(selVec.size() > nb_of_target_volumes)
    {
    for(i = 0; i < selVec.size(); i++) 
      {
      found = false;
      for(iTarget = 0; iTarget < nb_of_target_volumes; iTarget++)
        {
        target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(iTarget);
        if (selVec[i] == target_vol_id)
          {
          found = true;
          break;
          }
        }
      if (!found)
        {
        mrmlManager->AddTargetSelectedVolume(selVec[i]);
        }
      }
    }

  // Finally, adjusting selected volume orders

  nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  if(selVec.size() == nb_of_target_volumes)
    {
    for(i = 0; i < selVec.size(); i++) 
      {
      target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
      if (selVec[i] != target_vol_id)
        { 
        mrmlManager->MoveTargetSelectedVolume(selVec[i], i);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  int number_of_target_images_changed = 0; // to be completed by Brad

  if (number_of_target_images_changed &&
      !vtkKWMessageDialog::PopupYesNo( 
        this->GetApplication(), 
        NULL, 
        "Change the number of target images?",
        "Are you sure you want to change the number of target images?",
        vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
    {
    wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
    wizard_workflow->ProcessInputs();
    }
  else
    {
    this->Superclass::Validate();
    }
}


//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
