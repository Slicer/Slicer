#include "vtkBrainlabModuleStep.h"
#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleStep);
vtkCxxRevisionMacro(vtkBrainlabModuleStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkBrainlabModuleStep,GUI,vtkBrainlabModuleGUI);

//----------------------------------------------------------------------------
vtkBrainlabModuleStep::vtkBrainlabModuleStep()
{
  this->GUI = NULL;
  this->GUICallbackCommand = vtkCallbackCommand::New();
  this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->GUICallbackCommand->SetCallback(&vtkBrainlabModuleStep::GUICallback);
}

//----------------------------------------------------------------------------
vtkBrainlabModuleStep::~vtkBrainlabModuleStep()
{
  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->Delete ( );
    this->GUICallbackCommand = NULL;
    }
  this->SetGUI(NULL);
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkBrainlabModuleStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleStep::PopulateMenuWithLoadedVolumes(
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
  vtkBrainlabModuleMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
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
int vtkBrainlabModuleStep::SetMenuButtonSelectedItem(
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
void vtkBrainlabModuleStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


void vtkBrainlabModuleStep::GUICallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData )
{
  //vtkDebugWithObjectMacro(self, "In vtkBrainlabModuleStep GUICallback");
  vtkBrainlabModuleStep *self = reinterpret_cast<vtkBrainlabModuleStep *>(clientData);

  if (self)
    {
    self->ProcessGUIEvents(caller, eid, callData);
    }
}



void vtkBrainlabModuleStep::RaiseModule(const char *moduleName)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName(moduleName);        
  if ( currentModule )
    {
    currentModule->Enter( );
    currentModule->GetUIPanel()->Raise();
    }

}

