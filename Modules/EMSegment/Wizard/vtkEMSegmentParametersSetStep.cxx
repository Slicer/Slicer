/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentParametersSetStep.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $

=======================================================================auto=*/

#include "vtkEMSegmentParametersSetStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWTree.h"

#include "vtkKWTopLevel.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkEMSegmentAnatomicalStructureStep.h"
#include "vtkSlicerApplication.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWMessageDialog.h"
#include "vtkEMSegmentLogic.h"

#include "vtkDirectory.h"
#include "vtkMRMLEMSNode.h"
// Need to include this bc otherwise cannot find std functions  for some g ++ compilers 
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentParametersSetStep);
vtkCxxRevisionMacro(vtkEMSegmentParametersSetStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentParametersSetStep::vtkEMSegmentParametersSetStep()
{
  this->SetName("1/9. Define Task");
  this->SetDescription("Select a (new) task.");

  this->ParameterSetFrame      = NULL;
  this->ParameterSetMenuButton = NULL;

  this->RenameIndex = -1;
  this->RenameEntry = NULL;
  this->RenameTopLevel = NULL;
  this->RenameApply = NULL;
  this->RenameCancel = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentParametersSetStep::~vtkEMSegmentParametersSetStep()
{
  if (this->ParameterSetMenuButton)
    {
    this->ParameterSetMenuButton->Delete();
    this->ParameterSetMenuButton = NULL;
    }

  if (this->ParameterSetFrame)
    {
    this->ParameterSetFrame->Delete();
    this->ParameterSetFrame = NULL;
    }

 if (this->RenameEntry)
    {
    this->RenameEntry->SetParent(NULL);
    this->RenameEntry->Delete();
    this->RenameEntry = NULL;
    }
  if (this->RenameTopLevel)
    {
    this->RenameTopLevel->SetParent(NULL);
    this->RenameTopLevel->Delete();
    this->RenameTopLevel = NULL;
    }

  if (this->RenameApply)
    {
    this->RenameApply->SetParent(NULL);
    this->RenameApply->Delete();
    this->RenameApply = NULL;
    }
  if (this->RenameCancel)
    {
    this->RenameCancel->SetParent(NULL);
    this->RenameCancel->Delete();
    this->RenameCancel = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();


  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();

  wizardWidget->GetCancelButton()->SetEnabled(0);
  wizardWidget->SetNextButtonVisibility(0);
  wizardWidget->SetBackButtonVisibility(0);
  wizardWidget->SetFinishButtonVisibility(0);

  // Create the Parameters set frame

  if (!this->ParameterSetFrame)
    {
    this->ParameterSetFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->ParameterSetFrame->IsCreated())
    {
    this->ParameterSetFrame->SetParent(wizardWidget->GetClientArea());
    this->ParameterSetFrame->Create();
    this->ParameterSetFrame->SetLabelText("Select Task");
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2",
      this->ParameterSetFrame->GetWidgetName());

  // Create the Parameters Set Menu button

  if (!this->ParameterSetMenuButton)
    {
    this->ParameterSetMenuButton = vtkKWMenuButtonWithLabel::New();
    }

  if (!this->ParameterSetMenuButton->IsCreated())
    {
    this->ParameterSetMenuButton->SetParent(
      this->ParameterSetFrame->GetFrame());
    this->ParameterSetMenuButton->Create();
    this->ParameterSetMenuButton->GetLabel()->SetWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    this->ParameterSetMenuButton->SetLabelText("Task:");
    this->ParameterSetMenuButton->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH + 10);
    this->ParameterSetMenuButton->SetBalloonHelpString(
      "Select Task.");
    }
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
               this->ParameterSetMenuButton->GetWidgetName());

  this->UpdateLoadedParameterSets();
}

//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::PopulateLoadedParameterSets()
{
  if (!this->ParameterSetMenuButton ||
     !this->ParameterSetMenuButton->IsCreated())
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
      vtkWarningMacro("PopulateLoadedParameterSets: returning, no mrml manager");
      return;
    }

  // Check on Parameter Files 
  this->DefineDefaultTasksList();

  // Define Menue
  vtkKWMenu *menu = this->ParameterSetMenuButton->GetWidget()->GetMenu();
  menu->DeleteAllItems();

  char buffer[256];
  int numSets = mrmlManager->GetNumberOfParameterSets();

  for(int index = 0; index < numSets; index++)
    {
    const char *name = mrmlManager->GetNthParameterSetName(index);

    if (name)
      {
      sprintf(buffer, "%s %d", "SelectedParameterSetChangedCallback", index);
      menu->AddRadioButton(name, this, buffer);
      }
    }
 
    for (int i = 0 ; i < (int)this->pssDefaultTasksName.size(); i++)
    {
      int index = 0; 
      while ((index < numSets) && strcmp(mrmlManager->GetNthParameterSetName(index),pssDefaultTasksName[i].c_str() ))
      {
      index++;
      }
      
      if (index == numSets)
      {
      sprintf(buffer, "SelectedDefaultTaskChangedCallback %d 1", i);
      menu->AddRadioButton(pssDefaultTasksName[i].c_str(), this, buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::UpdateLoadedParameterSets()
{
  if(!this->ParameterSetMenuButton ||
     !this->ParameterSetMenuButton->IsCreated())
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  vtkKWMenuButton *menuButton = this->ParameterSetMenuButton->GetWidget();
  vtksys_stl::string sel_value = "";

  if (menuButton->GetValue())
    {
    sel_value = menuButton->GetValue();
    }

  this->PopulateLoadedParameterSets();

  if (strcmp(sel_value.c_str(), "") != 0)
    {
    // Select the original
    int numSets = menuButton->GetMenu()->GetNumberOfItems();

    for (int index = 0; index < numSets; index++)
      {
      const char *name = menuButton->GetMenu()->GetItemLabel(index);

      if (name && strcmp(sel_value.c_str(), name) == 0)
        {
        menuButton->GetMenu()->SelectItem(index);
        return;
        }
      }
    }

  // if there is no previous selection, select the first loaded set,
  // or if there is no loaded set, leave it blank

  int numSets = mrmlManager->GetNumberOfParameterSets();

  if (numSets > 0 &&
     menuButton->GetMenu()->GetNumberOfItems() > 1)
    {
    this->ParameterSetMenuButton->GetWidget()->GetMenu()->SelectItem(1);
    }
}


//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::
SelectedDefaultTaskChangedCallback(int index, bool warningFlag)
{

  if (index < 0 || index >  int(this->pssDefaultTasksName.size() -1) )
    {
      vtkErrorMacro("Index is not defined");
      return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  // Create New task 
  if (index ==  int(this->pssDefaultTasksName.size() -1))
    {   
      vtkWarningMacro("\n===========\n====Ignore message: vtkEMSegmentMRMLManager: Output volume is NULL\n===========");
      mrmlManager->CreateAndObserveNewParameterSet();
      vtkWarningMacro("\n===End of Error=======");
      this->PopUpRenameEntry(mrmlManager->GetNumberOfParameterSets() - 1);
      return;
    }

  this->LoadTask(index, warningFlag);
}

void vtkEMSegmentParametersSetStep::UpdateTaskListIndex(int index) 
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  this->UpdateLoadedParameterSets();

  //Assuming the mrml manager adds the node to the end.
  if (mrmlManager->GetNthParameterSetName(index))
    {
      // Select the newly created parameter set
      vtkKWMenuButton *menuButton = this->ParameterSetMenuButton->GetWidget();
      menuButton->GetMenu()->SelectItem(index);
    }

  vtkEMSegmentAnatomicalStructureStep *anat_step = this->GetGUI()->GetAnatomicalStructureStep();  
  if (anat_step &&
      anat_step->GetAnatomicalStructureTree() &&
      anat_step->GetAnatomicalStructureTree()->IsCreated())
    {
      anat_step->GetAnatomicalStructureTree()->GetWidget()->DeleteAllNodes();
    }
}


void vtkEMSegmentParametersSetStep::SelectedParameterSetChangedCallback(int index, int flag)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  // New Parameters

  if (index < 0)
    {
      vtkErrorMacro("Index has to be greater 0");
      return;
    }

  mrmlManager->SetLoadedParameterSetIndex(index);

  vtkEMSegmentAnatomicalStructureStep *anat_step =
    this->GetGUI()->GetAnatomicalStructureStep();

  if (anat_step &&
      anat_step->GetAnatomicalStructureTree() &&
      anat_step->GetAnatomicalStructureTree()->IsCreated())
    {
    anat_step->GetAnatomicalStructureTree()->GetWidget()->DeleteAllNodes();
    }

  std::string tclFileName = this->GetGUI()->GetLogic()->DefineTclTaskFullPathName(mrmlManager->GetNode()->GetTclTaskFilename());

  this->SourceTclFile(tclFileName.c_str());
  if (flag && (!this->SettingSegmentationMode(0)))
      {
    return ;
      }
  this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep(); 
}


int vtkEMSegmentParametersSetStep::SettingSegmentationMode(int flag) 
{
  vtkKWMessageDialog *dlg2 = vtkKWMessageDialog::New();
  dlg2->SetApplication( this->GetApplication());
  dlg2->SetMasterWindow(NULL);
  dlg2->SetOptions(vtkKWMessageDialog::InvokeAtPointer | vtkKWMessageDialog::Beep | vtkKWMessageDialog::YesDefault);
  dlg2->SetTitle("What Segmentation Mode To Proceed?");
  dlg2->SetStyleToOkOtherCancel();
  dlg2->SetOKButtonText("Advanced");
  dlg2->SetOtherButtonText("Simple");

  if (flag)
    {
      dlg2->SetText("In which mode do you want to proceed segmenting your data?\n Note, downloading the default setting might take a while!");
    }
  else
    {
      dlg2->SetText("In which mode do you want to proceed segmenting your data?");
    }

  dlg2->Invoke();
  int status = dlg2->GetStatus();
  dlg2->Delete();

  switch  (status)
    {
    case vtkKWMessageDialog::StatusOther : 
      this->GetGUI()->SetSegmentationModeToSimple();
      return 1;
      
    case vtkKWMessageDialog::StatusOK :
      this->GetGUI()->SetSegmentationModeToAdvanced();
      return 1;
    }

  return 0;       
}

//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//---------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::RenameApplyCallback(const char* newName)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!this->RenameEntry || !this->RenameEntry->GetWidget() || !mrmlManager)
   {
     return;
   }
  mrmlManager->SetNthParameterName(this->RenameIndex,this->RenameEntry->GetWidget()->GetValue());
  this->HideRenameEntry();
  this->UpdateTaskListIndex(this->RenameIndex);
  this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
}

//---------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::HideRenameEntry()
{
  if ( !this->RenameTopLevel )
    {
    return;
    }
  this->RenameTopLevel->Withdraw();
}


//---------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::PopUpRenameEntry(int index)
{
  vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
  this->RenameIndex = index;
  if ( !this->RenameTopLevel )
    {
    this->RenameTopLevel = vtkKWTopLevel::New ( );
    this->RenameTopLevel->SetApplication ( app );
    this->RenameTopLevel->ModalOn();
    this->RenameTopLevel->Create ( );
    this->RenameTopLevel->SetMasterWindow ( app->GetApplicationGUI()->GetMainSlicerWindow() );
    this->RenameTopLevel->HideDecorationOn ( );
    this->RenameTopLevel->Withdraw ( );
    this->RenameTopLevel->SetBorderWidth ( 2 );
    this->RenameTopLevel->SetReliefToGroove ( );

    vtkKWFrame *popUpFrameP = vtkKWFrame::New ( );
    popUpFrameP->SetParent ( this->RenameTopLevel );
    popUpFrameP->Create ( );
    app->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", popUpFrameP->GetWidgetName ( ) );

    this->RenameEntry = vtkKWEntryWithLabel::New();
    this->RenameEntry->SetParent( popUpFrameP );
    this->RenameEntry->Create();
    this->RenameEntry->SetLabelText( "New Task Name: " );
    this->RenameEntry->GetWidget()->SetCommandTrigger(vtkKWEntry::TriggerOnReturnKey); 
    this->RenameEntry->GetWidget()->SetCommand (this, "RenameApplyCallback");
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 8", this->RenameEntry->GetWidgetName() );
    popUpFrameP->Delete();


    vtkKWFrame *fP = vtkKWFrame::New();
    fP->SetParent ( popUpFrameP);
    fP->Create();
    app->Script ( "grid %s -row 1 -column 0 -columnspan 1 -pady 8 -sticky ew", fP->GetWidgetName() );

    this->RenameApply = vtkKWPushButton::New ();
    this->RenameApply->SetParent (fP);
    this->RenameApply->Create ( );
    this->RenameApply->SetText ("Apply");
    this->RenameApply->SetCommand (this, "RenameApplyCallback blub");
    app->Script ( "pack %s -side left -padx 4 -anchor c", RenameApply->GetWidgetName());

    this->RenameCancel = vtkKWPushButton::New();
    this->RenameCancel->SetParent (  fP );
    this->RenameCancel->Create();
    this->RenameCancel->SetText ( "Cancel");
    this->RenameCancel->SetCommand (this, "HideRenameEntry");
    // app->Script ( "pack %s  -side left -padx 4 -anchor c",this->RenameCancel->GetWidgetName() );

    fP->Delete();
    

    }    

  this->RenameEntry->GetWidget()->SetValue("");

  // Get the position of the mouse, position the popup
  int x, y;
  vtkKWTkUtilities::GetMousePointerCoordinates(this->ParameterSetMenuButton->GetWidget()->GetMenu(), &x, &y);
  this->RenameTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->RenameTopLevel->DeIconify();
  this->RenameTopLevel->Raise();

  this->RenameEntry->GetWidget()->SelectAll();
  this->RenameEntry->GetWidget()->Focus();
}

//----------------------------------------------------------------------------
int vtkEMSegmentParametersSetStep::LoadTask(int index, bool warningFlag)
{

  if (index < 0 || index >  int(this->pssDefaultTasksName.size() -2) )
    {
      vtkErrorMacro("Index is not defined");
      return 1;
    }


  // Load Task 
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!this->LoadDefaultData(pssDefaultTasksFile[index].c_str(),warningFlag))
    {
      // Remove the default selection entry from the menue, 
      this->PopulateLoadedParameterSets();

      // Figure out the index number
      int numSets = mrmlManager->GetNumberOfParameterSets();
      for(int index = 0; index < numSets; index++)
      {
        const char *name = mrmlManager->GetNthParameterSetName(index);
        if (name && !strcmp(name,pssDefaultTasksName[index].c_str()))
        {
          // Select the Node 
          this->SelectedParameterSetChangedCallback(index,0);
          break;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkEMSegmentParametersSetStep::LoadDefaultData(const char *tclFile, bool warningFlag)
{
  
  // Load Tcl File defining the setting
  this->SourceTclFile(tclFile);

  this->GetGUI()->SetSegmentationModeToAdvanced();
  if (warningFlag)
    {
      // do not want to proceed
      if (!this->SettingSegmentationMode(1))
    {
      return 1;
    }
    }

  // Load MRML File whose location is defined in the tcl file 
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLScene *scene = mrmlManager->GetMRMLScene();
  vtksys_stl::string mrmlFile(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication())->Script("::EMSegmenterParametersStepTcl::DefineMRMLFile"));
  scene->SetURL(mrmlFile.c_str());
 
  if (!scene->Import()) 
    {
      vtksys_stl::string msg= vtksys_stl::string("Could not load mrml file ") +  mrmlFile  ;
      vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Load Error", msg.c_str(), vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      return 1;
    }

  if(scene->GetErrorCode())
    {
      vtksys_stl::string msg= vtksys_stl::string("Corrupted File", "MRML file") +  mrmlFile +  vtksys_stl::string(" was corrupted or could not be loaded");
      vtkErrorMacro("ERROR: Failed to connect to the data. Error code: " << scene->GetErrorCode()  << " Error message: " << scene->GetErrorMessage());
      vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL, "Corrupted File", msg.c_str() , vtkKWMessageDialog::ErrorIcon | vtkKWMessageDialog::InvokeAtPointer);
      return 1;
    }

  cout << "==========================================================================" << endl;
  cout << "== Completed loading task data " << endl;
  cout << "==========================================================================" << endl;

 this->GetGUI()->GetApplicationGUI()->SelectModule("EMSegmenter");

  return 0;
}

//-------------vtksys_stl::string ---------------------------------------------------------------
void vtkEMSegmentParametersSetStep::DefineDefaultTasksList()
{
  //  cout << "-------- DefineDefaultTasksList Start" << endl;
 // set define list of parameters 
  this->pssDefaultTasksName.clear();
  this->pssDefaultTasksFile.clear();

  vtkDirectory *dir = vtkDirectory::New();
  vtksys_stl::string FilePath =  this->GetGUI()->GetLogic()->GetTclTaskDirectory();
  if (!dir->Open(FilePath.c_str()))
      {
    vtkErrorMacro("Cannot open " << this->GetGUI()->GetLogic()->GetTclTaskDirectory());
    // No special files 
    dir->Delete();
    return;
      }
    
  for (int i = 0; i < dir->GetNumberOfFiles(); i++)
      {
      vtksys_stl::string filename = dir->GetFile(i);
      //skip ., ..,  and the default file  
      if (strcmp(filename.c_str(), ".") == 0)
        {
        continue;
        }
      if (strcmp(filename.c_str(), "..") == 0)
        {
        continue;
        }
      
      if (strcmp(filename.c_str(), vtkMRMLEMSNode::GetDefaultTclTaskFilename()) == 0)
        {
        continue;
        }
 
      vtksys_stl::string fullName = this->GetGUI()->GetLogic()->DefineTclTaskFullPathName(filename.c_str());
      
      if (strcmp(vtksys::SystemTools::
                 GetFilenameExtension(fullName.c_str()).c_str(), ".tcl") != 0)
        {
        continue;
        }

      if (!vtksys::SystemTools::FileIsDirectory(fullName.c_str()))
        {

      vtkstd::replace(filename.begin(), filename.end(), '-', ' ');
      // Get Rid of extension
      filename.resize(filename.size() - 4);
      this->pssDefaultTasksFile.push_back(fullName);
      this->pssDefaultTasksName.push_back(vtksys_stl::string(filename));
        } 
      }
  dir->Delete();
  if (!this->pssDefaultTasksFile.size()) 
    {
      vtkWarningMacro("No default tasks found in " << this->GetGUI()->GetLogic()->GetTclTaskDirectory());
    }
  // The last one is always "Create New" 
  this->pssDefaultTasksFile.push_back(vtksys_stl::string(""));
  this->pssDefaultTasksName.push_back("Create new task");
  // cout << "-------- DefineDefaultTasksList End" << endl;

}

void vtkEMSegmentParametersSetStep::_Validate(int flag)
{
  if (flag) {
    if (this->SettingSegmentationMode(0))
    {
      vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();
      wizard_workflow->PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      return;
    }
  } 
}
