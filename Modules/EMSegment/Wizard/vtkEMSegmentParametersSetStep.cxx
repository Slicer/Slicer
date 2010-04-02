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
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentParametersSetStep);
vtkCxxRevisionMacro(vtkEMSegmentParametersSetStep, "$Revision: 1.2 $");


const int pssNumDefaultTasks = 3;
const char *pssDefaultTasksName[pssNumDefaultTasks] = 
{ 
  "MRI Human Brain",
  "CT Torso",
  "Create new task"
};

const char *pssDefaultTasksFile[pssNumDefaultTasks-1] = 
{ 
  "/share/data/EMSegmentTraining/MRIHumanBrain.mrml",
  "/share/data/EMSegmentTraining/CTTorso.mrml",
};


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


  vtkKWMenu *menu =
    this->ParameterSetMenuButton->GetWidget()->GetMenu();
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

  for (int i = 0 ; i <  pssNumDefaultTasks; i++)
    {
      int index = 0; 
      while ((index < numSets) && strcmp(mrmlManager->GetNthParameterSetName(index),pssDefaultTasksName[i] ))
    {
      index++;
    }
      
      if (index == numSets)
    {
      sprintf(buffer, "SelectedDefaultTaskChangedCallback %d", i);
      menu->AddRadioButton(pssDefaultTasksName[i], this, buffer);
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
    this->SelectedParameterSetChangedCallback(0);
    }
}

//----------------------------------------------------------------------------
int vtkEMSegmentParametersSetStep::LoadDefaultData(const char *dataLink, bool warningFlag)
{
  if (warningFlag)
    {
      if (!vtkKWMessageDialog::PopupYesNo( 
                      this->GetApplication(), 
                      NULL, 
                      "Load Task Specific Data?",
                      "It might take some time to download the default setting. Do you want to proceed ?", 
                      vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
    {
      return 1;
    }
    }


  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLScene *scene = mrmlManager->GetMRMLScene();
  int res;
  scene->SetURL(dataLink);
  res = scene->Connect();
  if(scene->GetErrorCode())
    {
      vtkErrorMacro("ERROR: Failed to connect to the data. Error code: " << scene->GetErrorCode() 
      << " Error message: " << scene->GetErrorMessage());
      return 1;
    }
  return 0;
}


//----------------------------------------------------------------------------
void vtkEMSegmentParametersSetStep::
SelectedDefaultTaskChangedCallback(int index, bool warningFlag)
{

  if (index < 0 || index >  pssNumDefaultTasks -1) 
    {
      vtkErrorMacro("Index is not defined");
      return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  // Create New task 
  if (index ==  pssNumDefaultTasks -1)
    {   
      vtkWarningMacro("\n===========\n====Ignore message: vtkEMSegmentMRMLManager: Output volume is NULL\n===========");
      mrmlManager->CreateAndObserveNewParameterSet();
      vtkWarningMacro("\n===End of Error=======");
      this->PopUpRenameEntry(mrmlManager->GetNumberOfParameterSets() - 1);
      return;
    }

  // Load Task 
  // if (!this->LoadDefaultData("http://xnd.slicer.org:8000/data/20090803T130148Z/ChangetrackerTutorial2009.mrml"))
  if (!this->LoadDefaultData(pssDefaultTasksFile[index],warningFlag))
    {
      // Remove the default selection entry from the menue, 
      this->PopulateLoadedParameterSets();

      // Figure out the index number
      int numSets = mrmlManager->GetNumberOfParameterSets();
      for(int index = 0; index < numSets; index++)
    {
      const char *name = mrmlManager->GetNthParameterSetName(index);
      if (name && !strcmp(name,pssDefaultTasksName[index]))
        {
          // Select the Node 
          this->SelectedParameterSetChangedCallback(index);
          index = numSets;
        }
    }
      // Go to next step 
      // Add that we transition to next stage 
      vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
      wizard_widget->GetWizardWorkflow()->AttemptToGoToNextStep();
    }
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


void vtkEMSegmentParametersSetStep::SelectedParameterSetChangedCallback(int index)
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

