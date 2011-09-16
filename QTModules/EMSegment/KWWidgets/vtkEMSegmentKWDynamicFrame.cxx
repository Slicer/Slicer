#include "vtkEMSegmentKWDynamicFrame.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWLabelWithLabel.h" 
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkEMSegmentMRMLManager.h"
#include "vtkEMSegmentGUI.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWApplication.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentKWDynamicFrame);
vtkCxxRevisionMacro(vtkEMSegmentKWDynamicFrame, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkEMSegmentKWDynamicFrame::vtkEMSegmentKWDynamicFrame()
{
  this->checkButton.clear();
  this->textLabel.clear();
  this->volumeMenuButton.clear();
  this->volumeMenuButtonID.clear();
  this->textEntry.clear();
  this->MRMLManager= NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentKWDynamicFrame::~vtkEMSegmentKWDynamicFrame()
{
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
  this->SetMRMLManager(NULL);
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::CreateEntryLists() 
{

  int newSize = atoi(this->GetApplication()->Script("expr $::EMSegmenterPreProcessingTcl::TextLabelSize"));
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

  newSize = atoi(this->GetApplication()->Script("expr $::EMSegmenterPreProcessingTcl::CheckButtonSize"));
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

  newSize = atoi(this->GetApplication()->Script("expr $::EMSegmenterPreProcessingTcl::VolumeMenuButtonSize"));
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

  newSize = atoi(this->GetApplication()->Script("expr $::EMSegmenterPreProcessingTcl::TextEntrySize"));
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
void vtkEMSegmentKWDynamicFrame::DefineCheckButton(const char *label, int initState, vtkIdType ID)
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
       this->checkButton[ID]->SetParent(this->GetFrame());
       this->checkButton[ID]->Create();
       // this->checkButton[ID]->GetLabel()->SetWidth(20);
     }
   this->checkButton[ID]->GetLabel()->SetText(label);
   this->checkButton[ID]->GetWidget()->SetSelectedState(initState);
   this->GetApplication()->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->checkButton[ID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::DefineTextLabel(const char *label, vtkIdType ID)
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
       this->textLabel[ID]->SetParent(this->GetFrame());
       this->textLabel[ID]->Create();
       // this->checkButton[ID]->GetLabel()->SetWidth(20);
     }
   this->textLabel[ID]->SetLabelText(label);
   this->GetApplication()->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->textLabel[ID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::DefineVolumeMenuButton(const char *label, vtkIdType initialVolID, vtkIdType ID)
{
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
       this->volumeMenuButton[ID]->SetParent(this->GetFrame());
       this->volumeMenuButton[ID]->Create();
       this->volumeMenuButton[ID]->GetWidget()->SetWidth(EMSEG_MENU_BUTTON_WIDTH);
       this->volumeMenuButton[ID]->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
       this->volumeMenuButton[ID]->SetLabelText(label);
       this->volumeMenuButtonID[ID] = initialVolID;
       if (this->volumeMenuButtonID[ID]) 
        {
          this->volumeMenuButton[ID]->GetWidget()->SetValue(this->MRMLManager->GetVolumeName(this->volumeMenuButtonID[ID]));
         } 
       else 
        {
          this->volumeMenuButton[ID]->GetWidget()->SetValue("None");
        }
     }
   this->GetApplication()->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->volumeMenuButton[ID]->GetWidgetName()); 
   std::stringstream setCmd;
   setCmd << "VolumeMenuButtonCallback " << ID;

   vtkEMSegmentGUI::PopulateMenuWithLoadedVolumes(this->MRMLManager,this->volumeMenuButton[ID]->GetWidget()->GetMenu(), this, setCmd.str().c_str());
}


//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::DefineTextEntry(const char *label, const char *initText, vtkIdType textID)
{
  int widgetWidth = 10;
  this->DefineTextEntry(label, initText, textID, widgetWidth);
}


//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::DefineTextEntry(const char *label, const char *initText, vtkIdType textID, int widgetWidth)
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
       this->textEntry[textID]->SetParent(this->GetFrame());
       this->textEntry[textID]->Create();
       this->textEntry[textID]->GetWidget()->SetWidth(widgetWidth);
       this->textEntry[textID]->SetLabelText(label);
       this->textEntry[textID]->GetWidget()->SetValue(initText);
     }
   this->GetApplication()->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->textEntry[textID]->GetWidgetName()); 
}

//----------------------------------------------------------------------------
int vtkEMSegmentKWDynamicFrame::GetCheckButtonValue(vtkIdType ID)
{
  if (ID >= (int)this->checkButton.size() || !this->checkButton[ID]) 
    { 
      return 0;
    }
  return this->checkButton[ID]->GetWidget()->GetSelectedState();
}

//----------------------------------------------------------------------------
vtkIdType vtkEMSegmentKWDynamicFrame::GetVolumeMenuButtonValue(vtkIdType ID)
{
  if (ID >= (int) this->volumeMenuButtonID.size()) 
    { 
      return 0;
    }
  return this->volumeMenuButtonID[ID];
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentKWDynamicFrame::GetTextEntryValue(vtkIdType ID)
{
  if (ID >= (int)this->textEntry.size() || !this->textEntry[ID]) 
    { 
      return NULL;
    }
  return this->textEntry[ID]->GetWidget()->GetValue();
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::SetButtonsFromMRML()
{
  if (!this->MRMLManager || !this->MRMLManager->GetGlobalParametersNode())
    {
      return;
    }
  
  const char *defTextChar =  this->MRMLManager->GetGlobalParametersNode()->GetTaskPreProcessingSetting();
  int cIndex = 0; 
  int eIndex = 0; 
  int vIndex = 0; 

  if (defTextChar)
    {
      std::string defText(defTextChar);
      if (defText.size()) 
    {
          size_t  startPos =1;
          size_t  endPos =defText.find(":",1);
  
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
                  this->volumeMenuButtonID[vIndex] = this->MRMLManager->MapMRMLNodeIDToVTKNodeID(volID);
                }
              else
                {
                  this->volumeMenuButtonID[vIndex] = 0;
                }
              if (this->volumeMenuButtonID[vIndex]) 
                {
                  this->volumeMenuButton[vIndex]->GetWidget()->SetValue(this->MRMLManager->GetVolumeName(this->volumeMenuButtonID[vIndex]));
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
          endPos =defText.find(":",startPos);
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
void  vtkEMSegmentKWDynamicFrame::VolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID) 
{
  if ((buttonID < (int) this->volumeMenuButtonID.size()) &&  this->volumeMenuButton[buttonID] )
    {
      this->volumeMenuButtonID[buttonID] = volID;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::PopUpWarningWindow(const char * msg)
{
 vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Warning", msg , vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer);
}

//----------------------------------------------------------------------------
void vtkEMSegmentKWDynamicFrame::SaveSettingToMRML()
{
  if (!this->MRMLManager || !this->MRMLManager->GetGlobalParametersNode())
    {
      return;
    }


  std::string oldText;

  if ( MRMLManager->GetGlobalParametersNode()->GetTaskPreProcessingSetting() ) 
    {
      oldText = std::string(MRMLManager->GetGlobalParametersNode()->GetTaskPreProcessingSetting());
    }

  vtksys_stl::stringstream defText;
  size_t  startPos =0;
  size_t  endPos   =0;

  if (oldText.size())
    {
      endPos =oldText.find(":",1);
    }
  else {
    // Nothing to update
    return;
  } 

  // when panel is first created it creates vtkEMSegmentStep->CreateEntryLists()  is called which creates an array of checkbutton with size $::EMSegmenterPreProcessingTcl::CheckButtonSize
  // all arrays are set to NULL 
  // The task specific tcl script EMSegmenterSimpleTcl:ShowCheckList  then creates specific entries of that list 
  // so in case checkButton[i] = NULL then  defText  should copy over the existing setting 
  // otherwise if   checkButton[i] != NULL then   defText  is updated with the setting of the check button

  for (int i =0 ; i < (int)  this->checkButton.size(); i++)
    {
      // assumes that the entry with index 0 is used during processing otherwise have problems
      if (this->checkButton[i]) {
         defText << ":C";
         defText << this->checkButton[i]->GetWidget()->GetSelectedState();
      } else {
         defText << oldText.substr(startPos,endPos-startPos).c_str();
      }

      if (endPos == std::string::npos)
      {
        startPos = std::string::npos;
        break;
      } 
      else {
        startPos = endPos;
        endPos ++;
        endPos =oldText.find(":",startPos); 
      }
    }

  for (int i =0 ; i < (int) this->volumeMenuButton.size(); i++)
    {
      // assumes that the entry with index 0 is used during processin otherwise have problems
      if (this->volumeMenuButton.size()) 
    {
      defText << ":V";
      vtkMRMLVolumeNode* volumeNode = MRMLManager->GetVolumeNode(this->volumeMenuButtonID[i]);
      if (!volumeNode) 
        {
          vtkErrorMacro("Volume Node for ID " << this->volumeMenuButtonID[i] << " does not exists" );
          defText << "NULL";
        } 
      else 
        {
          defText << volumeNode->GetID();
        }
    }
      else 
    {
      defText << oldText.substr(startPos,endPos-startPos).c_str();
    }
      if (endPos == std::string::npos)
    {
      startPos = std::string::npos;
      break;
    } 
      else {
    startPos = endPos;
    endPos ++;
    endPos =oldText.find(":",startPos); 
      }
    }


  for (int i =0 ; i < (int) this->textEntry.size(); i++)
    {
      // assumes that the entry with index 0 is used during processin otherwise have problems
      if (this->textEntry[i] && this->textEntry[i]->GetWidget()) {
    defText << ":E";
    defText <<  this->textEntry[i]->GetWidget()->GetValue();
      } else {
    defText << oldText.substr(startPos,endPos-startPos).c_str();
      }

      if (endPos == std::string::npos)
    {
      startPos = std::string::npos;
      break;
    } 
      else {
    startPos = endPos;
    endPos ++;
    endPos =oldText.find(":",startPos); 
      }
    }
  this->MRMLManager->GetGlobalParametersNode()->SetTaskPreProcessingSetting(defText.str().c_str());
}

//----------------------------------------------------------------------------
// void
// vtkEMSegmentPreProcessingStep::SetTaskPreprocessingSetting()
// {
//   vtkEMSegmentMRMLManager *MRMLManager = this->GetGUI()->GetMRMLManager();
//   if (!MRMLManager || !MRMLManager->GetGlobalParametersNode())
//     {
//       return;
//     }
// 
//   vtksys_stl::stringstream defText;
// 
//   for (int i =0 ; i < (int)  this->checkButton.size(); i++)
//     {
//       defText << ":C";
//       if (this->checkButton[i] && this->checkButton[i]->GetWidget())
//     {
//       defText << this->checkButton[i]->GetWidget()->GetSelectedState();
//     } 
//     }
// 
//   for (int i =0 ; i < (int) volumeMenuButtonID.size() ; i++)
//     {
//        defText << ":V";
//        if (this->volumeMenuButtonID[i]) {
//      vtkMRMLVolumeNode* volumeNode = MRMLManager->GetVolumeNode(this->volumeMenuButtonID[i]);
//      if (!volumeNode) 
//        {
//          vtkErrorMacro("Volume Node for ID " << this->volumeMenuButtonID[i] << " does not exists" );
//          defText << "NULL";
//        } 
//      else 
//        {
//          defText << volumeNode->GetID();
//        }
//        }
//       else 
//     {
//       defText << "NULL";
//     }
//     }
// 
// 
//   for (int i =0 ; i < (int)  this->textEntry.size(); i++)
//     {
//       defText << ":E";
//       if (this->textEntry[i] && this->textEntry[i]->GetWidget())
//     {
//       defText << this->textEntry[i]->GetWidget()->GetValue();
//     } 
//     }
// 
//   MRMLManager->GetGlobalParametersNode()->SetTaskPreProcessingSetting(defText.str().c_str());
// }



