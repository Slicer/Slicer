#include "vtkEMSegmentPreProcessingStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkKWMessageDialog.h"
#include "vtkEMSegmentLogic.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrameWithLabel.h" 
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSNode.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntryWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentPreProcessingStep);
vtkCxxRevisionMacro(vtkEMSegmentPreProcessingStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::vtkEMSegmentPreProcessingStep()
{
  this->SetName("6/9. Define Preprocessing");
  this->SetDescription("Answer questions for preprocessing of input images");
  this->askQuestionsBeforeRunningPreprocessingFlag = 1;
}

//----------------------------------------------------------------------------
vtkEMSegmentPreProcessingStep::~vtkEMSegmentPreProcessingStep()
{
}

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  //
  // Source TCL Files 
  //
  this->SourceTaskFiles();

  // Source all files here as we otherwise sometimes do not find the function as Tcl did not finish sourcing but our cxx file is already trying to call the function 
  vtksys_stl::string tclFile =  this->GetGUI()->GetLogic()->GetModuleShareDirectory();
#ifdef _WIN32
  tclFile.append("\\Tcl\\EMSegmentAutoSample.tcl");
#else
  tclFile.append("/Tcl/EMSegmentAutoSample.tcl");
#endif
  this->SourceTclFile(tclFile.c_str());


  //
  // Define General Framework For GUI 
  //

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

  //
  // Define Task Specific GUI 
  //

  this->CreateEntryLists();
  this->Script("::EMSegmenterPreProcessingTcl::ShowUserInterface");

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
  if (this->askQuestionsBeforeRunningPreprocessingFlag)
    {
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

//----------------------------------------------------------------------------
void vtkEMSegmentPreProcessingStep::SetTaskPreprocessingSetting()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      return;
    }


  std::string oldText = mrmlManager->GetNode()->GetTaskPreprocessingSetting();

  vtksys_stl::stringstream defText;

  for (int i =0 ; i < (int)  this->checkButton.size(); i++)
    {
      defText << "|C";
      if (this->checkButton[i] && this->checkButton[i]->GetWidget())
    {
      defText << this->checkButton[i]->GetWidget()->GetSelectedState();
    } 
    }

  for (int i =0 ; i < (int) volumeMenuButtonID.size() ; i++)
    {
       defText << "|V";
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
      defText << "|E";
      if (this->textEntry[i] && this->textEntry[i]->GetWidget())
    {
      defText << this->textEntry[i]->GetWidget()->GetValue();
    } 
    }

  mrmlManager->GetNode()->SetTaskPreprocessingSetting(defText.str().c_str());
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

