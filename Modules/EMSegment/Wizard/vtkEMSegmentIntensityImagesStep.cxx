#include "vtkEMSegmentIntensityImagesStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWListBoxToListBoxSelectionEditor.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityImagesStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityImagesStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityImagesStep::vtkEMSegmentIntensityImagesStep()
{
  this->SetName("4/8. Select Target Images");
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
    this->IntensityImagesTargetVolumeSelector->AllowReorderingOff();
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

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  int nb_of_volumes = logic->GetVolumeNumberOfChoices();
  int nb_of_target_volumes = logic->GetTargetNumberOfSelectedVolumes();
  
  // Update the source volume list 

  this->IntensityImagesTargetVolumeSelector->RemoveItemsFromSourceList();
  for (int index = 0; index < nb_of_volumes; index++)
    {
    vol_id = logic->GetVolumeNthID(index);
    found = false;
    for(int i = 0; i < nb_of_target_volumes; i++)
      {
      target_vol_id = logic->GetTargetSelectedVolumeNthID(i);
      if (vol_id == target_vol_id)
        {
        found = true;
        break;
        }
      }
    if (!found)
      {
      const char *name = logic->GetVolumeName(vol_id);
      if (name)
        {
        sprintf(buffer, "%s (%d)", name, vol_id);
        this->IntensityImagesTargetVolumeSelector->AddSourceElement(buffer);
        }
      }
    }

  // Update the target volume list

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = logic->GetTargetSelectedVolumeNthID(i);
    const char *name = logic->GetVolumeName(target_vol_id);
    if (name)
      {
      sprintf(buffer, "%s (%d)", name, target_vol_id);
      this->IntensityImagesTargetVolumeSelector->AddFinalElement(buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::IntensityImagesTargetSelectionChangedCallback()
{
  // The target volumes have changed because of user interaction

  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  vtkIdType vol_id;
  int nb_of_volumes = logic->GetTargetNumberOfSelectedVolumes();
  while(nb_of_volumes > 0)
    {
    vol_id = logic->GetTargetSelectedVolumeNthID(0);
    logic->RemoveTargetSelectedVolume(vol_id);
    nb_of_volumes = logic->GetTargetNumberOfSelectedVolumes();
    }
  
  // Add selected target volumes

  int size = this->IntensityImagesTargetVolumeSelector->
    GetNumberOfElementsOnFinalList();
  vtksys_stl::string targettext;
  vtksys_stl::string::size_type pos1, pos2;
  for(int i = 0; i < size; i++) 
    {
    targettext = 
      this->IntensityImagesTargetVolumeSelector->GetElementFromFinalList(i);
    pos1 = targettext.rfind("(");
    pos2 = targettext.rfind(")");
    if (pos1 != vtksys_stl::string::npos && pos2 != vtksys_stl::string::npos)
      {
      vol_id = atoi(targettext.substr(pos1+1, pos2-pos1-1).c_str());
      logic->AddTargetSelectedVolume(vol_id);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityImagesStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
