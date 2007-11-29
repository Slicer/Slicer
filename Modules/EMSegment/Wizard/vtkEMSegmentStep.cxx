#include "vtkEMSegmentStep.h"
#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentStep);
vtkCxxRevisionMacro(vtkEMSegmentStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkEMSegmentStep,GUI,vtkEMSegmentGUI);

//----------------------------------------------------------------------------
vtkEMSegmentStep::vtkEMSegmentStep()
{
  this->GUI = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentStep::~vtkEMSegmentStep()
{
  this->SetGUI(NULL);
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkEMSegmentStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::PopulateMenuWithLoadedVolumes(
  vtkKWMenu *menu, vtkObject* object, const char* callback)
{
  // Populate the menu with available volumes

  menu->DeleteAllItems();
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  int nb_of_volumes = mrmlManager->GetVolumeNumberOfChoices();
  for(int index=0; index < nb_of_volumes; index++)
    {
    vtkIdType vol_id = mrmlManager->GetVolumeNthID(index);
    sprintf(buffer, "%s %d", callback, static_cast<int>(vol_id));
    const char *name = mrmlManager->GetVolumeName(vol_id);
    if (name)
      {
      menu->AddRadioButton(name, object, buffer);
      }
    }
}
 
//----------------------------------------------------------------------------
int vtkEMSegmentStep::SetMenuButtonSelectedItem(
  vtkKWMenu* menu, vtkIdType vol_id)
{
  // Select a specific menu in a menu that was populated with volumes
  // using the PopulateMenuWithLoadedVolumes method

  vtksys_stl::string str_tmp;
  vtksys_stl::string::size_type pos; 
  int nb_of_volumes = menu->GetNumberOfItems();
  for(int index = 0; index < nb_of_volumes; index++)
    {
    str_tmp = menu->GetItemCommand(index);
    pos = str_tmp.rfind(" ");
    if(pos != vtksys_stl::string::npos &&
      vol_id == atoi(str_tmp.substr(pos).c_str()))
      {
      menu->SelectItem(index);
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::PopulateMenuWithTargetVolumes(
  vtkKWMenu *menu, vtkObject* object, const char* callback)
{
  // Populate a menu with the list of target volumes

  menu->DeleteAllItems();

  vtkIdType vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  int nb_of_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  for(int index = 0; index < nb_of_volumes; index++)
    {
    vol_id = mrmlManager->GetTargetSelectedVolumeNthID(index);
    const char *name = mrmlManager->GetVolumeName(vol_id);
    if (name)
      {
      sprintf(buffer, "%s %d", callback, static_cast<int>(vol_id));
      menu->AddRadioButton(name, object, buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
