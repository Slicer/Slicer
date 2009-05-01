#include "vtkIGTNavigationStep.h"
#include "vtkIGTNavigationGUI.h"
#include "vtkIGTNavigationMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIGTNavigationStep);
vtkCxxRevisionMacro(vtkIGTNavigationStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkIGTNavigationStep,GUI,vtkIGTNavigationGUI);

//----------------------------------------------------------------------------
vtkIGTNavigationStep::vtkIGTNavigationStep()
{
  this->GUI = NULL;
}

//----------------------------------------------------------------------------
vtkIGTNavigationStep::~vtkIGTNavigationStep()
{
  this->SetGUI(NULL);
}

//----------------------------------------------------------------------------
void vtkIGTNavigationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkIGTNavigationStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

//----------------------------------------------------------------------------
void vtkIGTNavigationStep::PopulateMenuWithLoadedVolumes(
  vtkKWMenu *menu, vtkObject* object, const char* callback)
{
  // Populate the menu with available volumes

  menu->DeleteAllItems();
  char buffer[256];

  //
  // first add a "NONE" option
  sprintf(buffer, "%s %d", callback, -1);
  menu->AddRadioButton("None", object, buffer);

  //
  // now populate menu with volumes loaded into slicer
  vtkIGTNavigationMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
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
int vtkIGTNavigationStep::SetMenuButtonSelectedItem(
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
void vtkIGTNavigationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
