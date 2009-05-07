#include "vtkIGTPlanningStep.h"
#include "vtkIGTPlanningGUI.h"
#include "vtkIGTPlanningMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIGTPlanningStep);
vtkCxxRevisionMacro(vtkIGTPlanningStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkIGTPlanningStep,GUI,vtkIGTPlanningGUI);

//----------------------------------------------------------------------------
vtkIGTPlanningStep::vtkIGTPlanningStep()
{
  this->GUI = NULL;
  this->GUICallbackCommand = vtkCallbackCommand::New();
  this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->GUICallbackCommand->SetCallback(&vtkIGTPlanningStep::GUICallback);
}

//----------------------------------------------------------------------------
vtkIGTPlanningStep::~vtkIGTPlanningStep()
{
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->Delete ( );
    this->GUICallbackCommand = NULL;
    }
  this->SetGUI(NULL);
}

//----------------------------------------------------------------------------
void vtkIGTPlanningStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkIGTPlanningStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkIGTPlanningStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

//----------------------------------------------------------------------------
void vtkIGTPlanningStep::PopulateMenuWithLoadedVolumes(
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
  vtkIGTPlanningMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
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
int vtkIGTPlanningStep::SetMenuButtonSelectedItem(
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
void vtkIGTPlanningStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


void vtkIGTPlanningStep::GUICallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData )
{
  //vtkDebugWithObjectMacro(self, "In vtkIGTPlanningStep GUICallback");
  vtkIGTPlanningStep *self = reinterpret_cast<vtkIGTPlanningStep *>(clientData);

  if (self)
    {
    self->ProcessGUIEvents(caller, eid, callData);
    }
}

