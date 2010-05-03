#include "vtkEMSegmentStep.h"
#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkKWMessageDialog.h" 
#include "vtkSlicerApplication.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWLabelWithLabel.h" 
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkEMSegmentLogic.h"
#include "vtkMRMLEMSNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentStep);
vtkCxxRevisionMacro(vtkEMSegmentStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkEMSegmentStep,GUI,vtkEMSegmentGUI);

//----------------------------------------------------------------------------
vtkEMSegmentStep::vtkEMSegmentStep()
{
  this->GUI = NULL;
  this->NextStep = NULL;
  this->checkButton.clear();
  this->textLabel.clear();
  this->volumeMenuButton.clear();
  this->volumeMenuButtonID.clear();
  this->textEntry.clear();
  this->CheckListFrame = NULL;

}

//----------------------------------------------------------------------------
vtkEMSegmentStep::~vtkEMSegmentStep()
{
  this->SetGUI(NULL);
  this->NextStep = NULL;

  if ( checkButton.size()) {
    for (int i = 0 ; i < (int) checkButton.size(); i++)
      {
    if (this->checkButton[i])
      {
        this->checkButton[i]->Delete();
      }
      }
    this->checkButton.clear();
  }
  if ( this->textLabel.size()) {
    for (int i = 0 ; i < (int) this->textLabel.size(); i++)
      {
    if (this->textLabel[i])
      {
        this->textLabel[i]->Delete();
      }
      }
    this->textLabel.clear();
  }

  if ( this->volumeMenuButton.size()) {
    for (int i = 0 ; i < (int) this->volumeMenuButton.size(); i++)
      {
    if (this->volumeMenuButton[i])
      {
        this->volumeMenuButton[i]->Delete();
      }
      }
    this->volumeMenuButton.clear();
  }
  this->volumeMenuButtonID.clear();

  if ( this->textEntry.size()) {
    for (int i = 0 ; i < (int) this->textEntry.size(); i++)
      {
    if (this->textEntry[i])
      {
        this->textEntry[i]->Delete();
      }
      }
    this->textEntry.clear();
  }
  if (this->CheckListFrame) {
    this->CheckListFrame->Delete();
    this->CheckListFrame = NULL;
  }

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

  //
  // first add a "NONE" option
  sprintf(buffer, "%s %d", callback, -1);
  menu->AddRadioButton("None", object, buffer);

  //
  // now populate menu with volumes loaded into slicer
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
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
  if (!mrmlManager)
    {
    return;
    }
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

//----------------------------------------------------------------------------
void vtkEMSegmentStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  if (this->NextStep) 
    {
      this->NextStep->RemoveResults();
    } else {
  }
}


//----------------------------------------------------------------------------

void vtkEMSegmentStep::SetNextStep(vtkEMSegmentStep *init) { 
  this->NextStep = init;
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::CreateEntryLists() 
{

  int newSize = atoi(this->Script("expr $::EMSegmenterPreProcessingTcl::TextLabelSize"));
  int oldSize = this->textLabel.size();
  if (oldSize != newSize) {
    for (int i = 0; i < oldSize ; i++)
      {
    if (this->textLabel[i])
      {
        this->textLabel[i]->Delete();
        this->textLabel[i] = NULL;
      }
      }

    this->textLabel.resize(newSize);
    for (int i = 0; i < newSize ; i++)
      {
    this->textLabel[i] = NULL;
      }
  }

  newSize = atoi(this->Script("expr $::EMSegmenterPreProcessingTcl::CheckButtonSize"));
  oldSize = this->checkButton.size();
  // cout << "CheckButtonSize " << newSize << " " << oldSize << endl;
  if (oldSize != newSize) {
    for (int i = 0; i < oldSize ; i++)
      {
    if (this->checkButton[i])
      {
        this->checkButton[i]->Delete();
        this->checkButton[i] = NULL;
      }
      }
    this->checkButton.resize(newSize);
    for (int i = 0; i < newSize ; i++)
      {
       this->checkButton[i] = NULL;
      }
 }

  newSize = atoi(this->Script("expr $::EMSegmenterPreProcessingTcl::VolumeMenuButtonSize"));
  oldSize = this->volumeMenuButton.size();
  if (oldSize != newSize) {
    for (int i = 0; i < oldSize ; i++)
      {
    if (this->volumeMenuButton[i])
      {
        this->volumeMenuButton[i]->Delete();
        this->volumeMenuButton[i] = NULL;
      }
    this->volumeMenuButtonID[i] = 0;
      }

    this->volumeMenuButton.resize(newSize);
    this->volumeMenuButtonID.resize(newSize);

    for (int i = 0; i < newSize ; i++)
     {
       this->volumeMenuButton[i] = NULL;
       this->volumeMenuButtonID[i] = 0;
     }
  }

  newSize = atoi(this->Script("expr $::EMSegmenterPreProcessingTcl::TextEntrySize"));
  oldSize =  this->textEntry.size();
  if (oldSize != newSize) {
    for (int i = 0; i < oldSize ; i++)
      {
    if (this->textEntry[i])
      {
        this->textEntry[i]->Delete();
        this->textEntry[i] = NULL;
      }
      }
    this->textEntry.resize(newSize);
    for (int i = 0; i < newSize ; i++)
     {
       this->textEntry[i] = NULL;
     }
  }
 
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::DefineCheckButton(const char *label, int initState, vtkIdType ID)
{
  if (ID >= (vtkIdType)this->checkButton.size())
    {
      vtkErrorMacro("ID("<< ID <<") is out of range ("<< this->checkButton.size() << ") ");
      return ;
    }

   if ( !this->checkButton[ID]) 
     {
       this->checkButton[ID] = vtkKWCheckButtonWithLabel::New();
     }
   if (!this->checkButton[ID]->IsCreated()) 
     {
       this->checkButton[ID]->SetParent(this->CheckListFrame->GetFrame());
       this->checkButton[ID]->Create();
       // this->checkButton[ID]->GetLabel()->SetWidth(20);
     }
   this->checkButton[ID]->GetLabel()->SetText(label);
   this->checkButton[ID]->GetWidget()->SetSelectedState(initState);
   this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->checkButton[ID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::DefineTextLabel(const char *label, vtkIdType ID)
{
  if (ID >= (vtkIdType)this->textLabel.size())
    {
      vtkErrorMacro("ID is out of range");
      return ;
    }

   if ( !this->textLabel[ID]) 
     {
       this->textLabel[ID] = vtkKWLabelWithLabel::New();
     }
   if (!this->textLabel[ID]->IsCreated()) 
     {
       this->textLabel[ID]->SetParent(this->CheckListFrame->GetFrame());
       this->textLabel[ID]->Create();
       // this->checkButton[ID]->GetLabel()->SetWidth(20);
     }
   this->textLabel[ID]->SetLabelText(label);
   this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->textLabel[ID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::DefineVolumeMenuButton(const char *label, vtkIdType initialVolID ,vtkIdType ID)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (ID >= (vtkIdType)this->volumeMenuButton.size())
    {
      vtkErrorMacro("ID is out of range");
      return ;
    }

   if ( !this->volumeMenuButton[ID]) 
     {
       this->volumeMenuButton[ID] = vtkKWMenuButtonWithLabel::New();
     }
   if (!this->volumeMenuButton[ID]->IsCreated()) 
     {
       this->volumeMenuButton[ID]->SetParent(this->CheckListFrame->GetFrame());
       this->volumeMenuButton[ID]->Create();
       this->volumeMenuButton[ID]->GetWidget()->SetWidth(EMSEG_MENU_BUTTON_WIDTH);
       this->volumeMenuButton[ID]->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
       this->volumeMenuButton[ID]->SetLabelText(label);
       this->volumeMenuButtonID[ID] = initialVolID;
       if (this->volumeMenuButtonID[ID]) 
        {
          this->volumeMenuButton[ID]->GetWidget()->SetValue(mrmlManager->GetVolumeName(this->volumeMenuButtonID[ID]));
         } 
       else 
        {
          this->volumeMenuButton[ID]->GetWidget()->SetValue("None");
        }
     }
   this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->volumeMenuButton[ID]->GetWidgetName()); 
   std::stringstream setCmd;
   setCmd << "VolumeMenuButtonCallback " << ID;

   this->PopulateMenuWithLoadedVolumes(this->volumeMenuButton[ID]->GetWidget()->GetMenu(), this, setCmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::DefineTextEntry(const char *label, const char *initText, vtkIdType textID)
{

  if (textID >= (vtkIdType)this->textEntry.size())
    {
      vtkErrorMacro("ID is out of range");
      return ;
    }

   if ( !this->textEntry[textID]) 
     {
       this->textEntry[textID] = vtkKWEntryWithLabel::New();
     }
   if (!this->textEntry[textID]->IsCreated()) 
     {
       this->textEntry[textID]->SetParent(this->CheckListFrame->GetFrame());
       this->textEntry[textID]->Create();
       this->textEntry[textID]->GetWidget()->SetWidth(10);
       this->textEntry[textID]->SetLabelText(label);
       this->textEntry[textID]->GetWidget()->SetValue(initText);
     }
   this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->textEntry[textID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
int vtkEMSegmentStep::GetCheckButtonValue(vtkIdType ID)
{
  if (ID >= (int)this->checkButton.size() || !this->checkButton[ID]) 
    { 
      return 0;
    }
  return this->checkButton[ID]->GetWidget()->GetSelectedState();
}

//----------------------------------------------------------------------------
vtkIdType vtkEMSegmentStep::GetVolumeMenuButtonValue(vtkIdType ID)
{
  if (ID >= (int) this->volumeMenuButtonID.size()) 
    { 
      return 0;
    }
  return this->volumeMenuButtonID[ID];
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentStep::GetTextEntryValue(vtkIdType ID)
{
  if (ID >= (int)this->textEntry.size() || !this->textEntry[ID]) 
    { 
      return NULL;
    }
  return this->textEntry[ID]->GetWidget()->GetValue();
}

//----------------------------------------------------------------------------
void vtkEMSegmentStep::SetButtonsFromMRML()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      return;
    }
  
  const char *defTextChar =  mrmlManager->GetNode()->GetTaskPreprocessingSetting();
  int cIndex = 0; 
  int eIndex = 0; 
  int vIndex = 0; 

  if (defTextChar)
    {
      std::string defText(defTextChar);
      if (defText.size()) 
    {
          size_t  startPos =1;
          size_t  endPos =defText.find("|",1);
  
          while ( 1 ) 
        {
          // cout << "This is the tag "<< defText.substr(startPos,endPos-startPos);

          if (!defText.substr(startPos,1).compare("C")) 
        {
          if (cIndex < (int)this->checkButton.size()) 
            {
              startPos +=1;
              if (this->checkButton[cIndex]) 
            {
              int flag  = atoi(defText.substr(startPos,endPos-startPos).c_str());
              this->checkButton[cIndex]->GetWidget()->SetSelectedState(flag); 
            }
              cIndex ++;
            }
        }
          else if (!defText.substr(startPos,1).compare("V")) 
        {
          if (vIndex < (int) this->volumeMenuButtonID.size()) 
            {
              startPos +=1;
              if (this->volumeMenuButton[vIndex]) 
            {
              const char* volID  = defText.substr(startPos,endPos-startPos).c_str();
              if (strcmp(volID,"NULL"))
                {
                  this->volumeMenuButtonID[vIndex] = mrmlManager->MapMRMLNodeIDToVTKNodeID(volID);
                }
              else
                {
                  this->volumeMenuButtonID[vIndex] = 0;
                }
              if (this->volumeMenuButtonID[vIndex]) 
                {
                  this->volumeMenuButton[vIndex]->GetWidget()->SetValue(mrmlManager->GetVolumeName(this->volumeMenuButtonID[vIndex]));
                } 
              else 
                {
                  this->volumeMenuButton[vIndex]->GetWidget()->SetValue("None");
                }
            }
              vIndex ++;
            }
        }
          else  if (!defText.substr(startPos,1).compare("E")) 
        {
          if (eIndex < (int)this->textEntry.size()) 
            {
              startPos +=1;
              if (this->textEntry[eIndex]) 
            {          
              this->textEntry[eIndex]->GetWidget()->SetValue(defText.substr(startPos,endPos-startPos).c_str());
            }
              eIndex ++;
            }
        }
    
          if (endPos ==  std::string::npos) 
        {
          break; 
        } 
          else
        {
          startPos = endPos +1;
          endPos =defText.find("|",startPos);
        }
        }
    }
    }
  for (int i = cIndex; i < (int) this->checkButton.size(); i++)
    {
      if (this->checkButton[i])
    {
      this->checkButton[i]->GetWidget()->SetSelectedState(0);
    }
    }

  for (int i = vIndex; i < (int) this->volumeMenuButton.size(); i++)
    {
      if (this->volumeMenuButton[i])
    {
      this->volumeMenuButton[i]->GetWidget()->SetValue("None");
      this->volumeMenuButtonID[i] = 0; 
    }
    }

  for (int i = eIndex; i < (int) this->textEntry.size(); i++)
    {
      if (this->textEntry[i])
    {
      this->textEntry[i]->GetWidget()->SetValue("");
    }
    }
}

//----------------------------------------------------------------------------
void  vtkEMSegmentStep::VolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID) 
{
  if ((buttonID < (int) this->volumeMenuButtonID.size()) &&  this->volumeMenuButton[buttonID] )
    {
      this->volumeMenuButtonID[buttonID] = volID;
    }
}

//----------------------------------------------------------------------------
int vtkEMSegmentStep::SourceTclFile(const char *tclFile)
{
  //vtkSlicerApplication::SafeDownCast(this->GetApplication()),tclFile);
  return this->GUI->GetLogic()->SourceTclFile(this->GetApplication(),tclFile);
}

//----------------------------------------------------------------------------
int vtkEMSegmentStep::SourceTaskFiles() 
{
  return this->GUI->GetLogic()->SourceTaskFiles(this->GetApplication());
}

//----------------------------------------------------------------------------
int vtkEMSegmentStep::SourcePreprocessingTclFiles() 
{
  return this->GUI->GetLogic()->SourcePreprocessingTclFiles(this->GetApplication());
}
