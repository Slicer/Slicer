#include "vtkEMSegmentPreProcessingStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWMessageDialog.h"
#include "vtkEMSegmentLogic.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkMRMLEMSNode.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWFrameWithLabel.h" 
#include "vtkKWLabelWithLabel.h" 
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentPreProcessingStep);
vtkCxxRevisionMacro(vtkEMSegmentPreProcessingStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::vtkEMSegmentPreProcessingStep()
{
  this->SetName("6/9. Define Preprocessing");
  this->SetDescription("Answer questions for preprocessing of input images");
  this->checkButton.clear();
  this->textLabel.clear();
  this->volumeMenuButton.clear();
  this->volumeMenuButtonID.clear();
  this->textEntry.clear();
  this->CheckListFrame = NULL;

}

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::~vtkEMSegmentPreProcessingStep()
{
  if (this->CheckListFrame) {
    this->CheckListFrame->Delete();
    this->CheckListFrame = NULL;
  }
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }

  vtkKWWidget *parent = wizard_widget->GetClientArea();;
  int enabled = parent->GetEnabled();
  wizard_widget->GetCancelButton()->SetEnabled(enabled);

  if (!this->CheckListFrame)
    {
    this->CheckListFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->CheckListFrame->IsCreated())
    {
    this->CheckListFrame->SetParent(parent);
    this->CheckListFrame->Create();
    this->CheckListFrame->SetLabelText("Check List");
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->CheckListFrame->GetWidgetName());

  vtksys_stl::string generalFile = this->GetGUI()->GetLogic()->DefineTclTaskFullPathName(vtkMRMLEMSNode::GetDefaultTclTaskFilename());
  vtksys_stl::string specificFile = this->GetGUI()->GetLogic()->DefineTclTasksFileFromMRML();
  cout << "Preprocessing:  Sourcing general file : " << generalFile.c_str() << endl;
  // Have to first source the default file to set up the basic structure"
  this->SourceTclFile(generalFile.c_str());
  // Now we overwrite anything from the default
  if (specificFile.compare(generalFile))
    {
      cout << "Preprocessing:  Sourcing task specific file: " <<   specificFile << endl;
    this->SourceTclFile(specificFile.c_str()); 
    }
  this->Script("::EMSegmenterPreProcessingTcl::ShowUserInterface");

  // Source all files here as we otherwise sometimes do not find the function as Tcl did not finish sourcing but our cxx file is already trying to call the function 
  vtksys_stl::string tclFile =  this->GetGUI()->GetLogic()->GetModuleShareDirectory();
#ifdef _WIN32
  tclFile.append("\\Tcl\\EMSegmentAutoSample.tcl");
#else
  tclFile.append("/Tcl/EMSegmentAutoSample.tcl");
#endif
  this->SourceTclFile(tclFile.c_str());


}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::CreateCheckButton(const char *label, int initState, vtkIdType ID)
{
  int oldSize = (int)this->checkButton.size();
   if (ID >= oldSize) 
     {
       this->checkButton.resize(ID+1);
       for (int i = oldSize; i <= ID ; i++)
     {
       this->checkButton[i] = NULL;
     }
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

void vtkEMSegmentPreProcessingStep::CreateTextLabel(const char *label, vtkIdType ID)
{
  int oldSize = (int)this->textLabel.size();
   if (ID >= oldSize) 
     {
       this->textLabel.resize(ID+1);
       for (int i = oldSize; i <= ID ; i++)
     {
       this->textLabel[i] = NULL;
     }
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
void vtkEMSegmentPreProcessingStep::CreateVolumeMenuButton(const char *label, vtkIdType initialVolID ,vtkIdType ID)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  int oldSize = (int)this->volumeMenuButton.size();
   if (ID >= oldSize) 
     {
       this->volumeMenuButton.resize(ID+1);
       this->volumeMenuButtonID.resize(ID+1);
       for (int i = oldSize; i <= ID ; i++)
     {
       this->volumeMenuButton[i] = NULL;
       this->volumeMenuButtonID[i] = 0;
     }
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
   setCmd << "PreProcessingVolumeMenuButtonCallback " << ID;

   this->PopulateMenuWithLoadedVolumes(this->volumeMenuButton[ID]->GetWidget()->GetMenu(), this, setCmd.str().c_str());
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::CreateTextEntry(const char *label, const char *initText, vtkIdType textID)
{
  int oldSize = (int)this->textEntry.size();
   if (textID >= oldSize) 
     {
       this->textEntry.resize(textID+1);
       for (int i = oldSize; i <= textID ; i++)
     {
       this->textEntry[i] = NULL;
     }
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
int vtkEMSegmentPreProcessingStep::GetCheckButtonValue(vtkIdType ID)
{
  if (ID >= (int)this->checkButton.size() || !this->checkButton[ID]) 
    { 
      return 0;
    }
  return this->checkButton[ID]->GetWidget()->GetSelectedState();
}

//----------------------------------------------------------------------------
vtkIdType vtkEMSegmentPreProcessingStep::GetVolumeMenuButtonValue(vtkIdType ID)
{
  if (ID >= (int) this->volumeMenuButtonID.size()) 
    { 
      return 0;
    }
  return this->volumeMenuButtonID[ID];
}

//----------------------------------------------------------------------------
const char* vtkEMSegmentPreProcessingStep::GetTextEntryValue(vtkIdType ID)
{
  if (ID >= (int)this->textEntry.size() || !this->textEntry[ID]) 
    { 
      return NULL;
    }
  return this->textEntry[ID]->GetWidget()->GetValue();
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::Validate()
{
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  // If they are still valid do not repeat preprocessing unless otherwhise wanted 
  // Kilian - still to do - save intermediate results 
  // so do special check here 
  if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid() && mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNodeIsValid())
    {
      // If it is ask if preprocessing should be done again
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "Redo Preprocessing of images?",
                      "Do you want to redo preprocessing of input images ?", 
                       vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
      {
        // If not just proceed
        this->Superclass::Validate();
        return;
      }
    } else {
      if (!vtkKWMessageDialog::PopupYesNo(this->GetApplication(), NULL, "Start Preprocessing of images?",
                       "Preprocessing of images might take a while. Do you want to proceed ?", 
                       vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
      {
        wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
        wizard_workflow->ProcessInputs();
        return;
      }
   }

  this->SetTaskPreprocessingSetting();

  int flag = atoi(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication())->Script("::EMSegmenterPreProcessingTcl::Run"));
  if (flag)
    {
      cout << "Pre-processing did not execute correctly" << endl;
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      return;
    } 

    // Set it to valid so next time we do not have to recompute it 
    mrmlManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
    mrmlManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

  // Everything went smoothly
  this->Superclass::Validate();
}

void vtkEMSegmentPreProcessingStep::SetButtonsFromMRML()
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
      size_t  startPos =0;
      size_t  endPos =defText.find("|",startPos);
   
      while (startPos != std::string::npos) 
    {
      if (!defText.substr(startPos,1).compare("C")) 
        {
          if (cIndex < (int)this->checkButton.size()) 
        {
          startPos +=1;
          int flag  = atoi(defText.substr(startPos,endPos-startPos).c_str());
          this->checkButton[cIndex]->GetWidget()->SetSelectedState(flag); 
          cIndex ++;
        }
        }
      else if (!defText.substr(startPos,1).compare("V")) 
        {
          if (vIndex < (int) this->volumeMenuButtonID.size()) 
        {
          startPos +=1;
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
          vIndex ++;
        }
        }
      else  if (!defText.substr(startPos,1).compare("E")) 
        {
          if (eIndex < (int)this->textEntry.size()) 
        {
          startPos +=1;
          this->textEntry[eIndex]->GetWidget()->SetValue(defText.substr(startPos,endPos-startPos).c_str());
          eIndex ++;
        }
        }

      if (endPos ==  std::string::npos) 
        {
        startPos =endPos;
        } 
      else
        {
          startPos = endPos +1;
          endPos =defText.find("|",startPos);
        }
    }
    }
  for (int i = cIndex; i < (int) this->checkButton.size(); i++)
    {
      this->checkButton[i]->GetWidget()->SetSelectedState(0);
    }

  for (int i = vIndex; i < (int) this->volumeMenuButton.size(); i++)
    {
      this->volumeMenuButton[i]->GetWidget()->SetValue("None");
      this->volumeMenuButtonID[i] = 0; 
    }

  for (int i = eIndex; i < (int) this->textEntry.size(); i++)
    {
      this->textEntry[i]->GetWidget()->SetValue("");
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::SetTaskPreprocessingSetting()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      return;
    }
  vtksys_stl::stringstream defText;
  int initFlag = 0;
  for (int i =0 ; i < (int)  this->checkButton.size(); i++)
    {
      if (initFlag) 
    {
      defText << "|C";
    }
      else 
    {
      defText << "C";
      initFlag = 1;
    }
      defText << this->checkButton[i]->GetWidget()->GetSelectedState();
    }

  for (int i =0 ; i < (int)  this->volumeMenuButton.size(); i++)
    {
      if (initFlag) 
    {
      defText << "|V";
    }
      else 
    {
      initFlag = 1;
      defText << "V";
    }

      if (this->volumeMenuButtonID[i]) {
    vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(this->volumeMenuButtonID[i]);
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
      defText << "NULL";
    }
    }

 for (int i =0 ; i < (int)  this->textEntry.size(); i++)
    {
      if (initFlag) 
    {
      defText << "|E";
    }
      else 
    {
      defText << "E";
      initFlag = 1;
    }
      defText << this->textEntry[i]->GetWidget()->GetValue();
    }

  mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
}

//----------------------------------------------------------------------------
void  vtkEMSegmentPreProcessingStep::PreProcessingVolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID) 
{
  if (buttonID < (int) volumeMenuButtonID.size()) 
    {
      volumeMenuButtonID[buttonID] = volID;
    }
}

//-----------------------------------------------------------------------------
int vtkEMSegmentPreProcessingStep::ComputeIntensityDistributionsFromSpatialPrior()
{
  // iterate over tree nodes
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;

  mrmlManager->GetListOfTreeNodeIDs(mrmlManager->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
      if (mrmlManager->GetTreeNodeIsLeaf(*i)) 
    {      
      this->UpdateIntensityDistributionAuto(*i);
    }
    }
  return 0;
}

//-----------------------------------------------------------------------------
void  vtkEMSegmentPreProcessingStep::UpdateIntensityDistributionAuto(vtkIdType nodeID)
{
 vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager->GetTreeNodeSpatialPriorVolumeID(nodeID)) {
    vtkWarningMacro("Nothing to update for " << nodeID << " as atlas is not defined");
    return ;
  }
  // get working node 
  vtkMRMLEMSTargetNode* workingTarget = NULL;
  if (mrmlManager->GetWorkingDataNode()->GetAlignedTargetNode() &&
      mrmlManager->GetWorkingDataNode()->GetAlignedTargetNodeIsValid())
    {
    workingTarget = mrmlManager->GetWorkingDataNode()->GetAlignedTargetNode();
    }
  else 
    {
       vtkErrorMacro("Cannot update intensity distribution bc Aligned Target is not correctly defined for node " << nodeID);
       return ;
    }

  int numTargetImages = workingTarget->GetNumberOfVolumes();

   // Sample
  {
    vtksys_stl::stringstream CMD ;
    CMD <<  "::EMSegmenterAutoSampleTcl::EMSegmentGaussCurveCalculationFromID 0.95 1 { " ;
    for (int i = 0 ; i < numTargetImages; i++) {
      CMD << workingTarget->GetNthVolumeNodeID(i) << " " ;
    }
    CMD << " } " << mrmlManager->GetVolumeNode(mrmlManager->GetTreeNodeSpatialPriorVolumeID(nodeID))->GetID() << " {" <<  mrmlManager->GetTreeNodeName(nodeID) << "} \n";
    // cout << CMD.str().c_str() << endl;
    if (atoi(this->Script(CMD.str().c_str()))) { return; }
  }

  //
  // propogate data to mrml node
  //

  vtkMRMLEMSTreeParametersLeafNode* leafNode = mrmlManager->GetTreeNode(nodeID)->GetParametersNode()->GetLeafParametersNode();  
  for (int r = 0; r < numTargetImages; ++r)
    {
      {
    double value = atof(this->Script("expr $::EMSegment(GaussCurveCalc,Mean,%d)",r));
    leafNode->SetAutoLogMean(r, value);
      }
      for (int c = 0; c < numTargetImages; ++c)
      {
    double value = atof(this->Script("expr $::EMSegment(GaussCurveCalc,Covariance,%d,%d)",r,c));
    leafNode->SetAutoLogCovariance(r, c, value);
      }
    }
}

