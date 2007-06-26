/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkQdecModuleGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "QdecSubject.h"

#include "vtkQdecModuleGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWLabel.h"

//------------------------------------------------------------------------------
vtkQdecModuleGUI* vtkQdecModuleGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkQdecModuleGUI");
  if(ret)
    {
      return (vtkQdecModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkQdecModuleGUI;
}

//----------------------------------------------------------------------------
vtkQdecModuleGUI::vtkQdecModuleGUI()
{
  this->NAMICLabel = NULL;
  this->LoadTableButton = NULL;
  this->ContinuousFactorsLabel = NULL;
  this->DiscreteFactorsLabel = NULL;
  this->ApplyButton = NULL;
  this->MultiColumnList = NULL;
  
  this->Logic = NULL;
}

//----------------------------------------------------------------------------
vtkQdecModuleGUI::~vtkQdecModuleGUI()
{
  this->RemoveMRMLNodeObservers ( );
  this->RemoveLogicObservers ( );

  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  
  if ( this->LoadTableButton )
    {
    this->LoadTableButton->SetParent(NULL);
    this->LoadTableButton->Delete();
    this->LoadTableButton = NULL;
    }

  if ( this->DiscreteFactorsLabel )
    {
    this->DiscreteFactorsLabel->SetParent ( NULL );
    this->DiscreteFactorsLabel->Delete();
    this->DiscreteFactorsLabel = NULL;
    }

   if ( this->ContinuousFactorsLabel )
    {
    this->ContinuousFactorsLabel->SetParent ( NULL );
    this->ContinuousFactorsLabel->Delete();
    this->ContinuousFactorsLabel = NULL;
    }

   if (this->MultiColumnList)
     {
     this->MultiColumnList->SetParent(NULL);
     this->MultiColumnList->Delete();
     this->MultiColumnList = NULL;
     }
  
  if ( this->ApplyButton )
    {
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }

  this->SetLogic (NULL);
}

//----------------------------------------------------------------------------
void vtkQdecModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "vtkQdecModuleGUI" << endl;
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::AddGUIObservers ( ) 
{
  this->LoadTableButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkQdecModuleGUI::RemoveGUIObservers ( )
{
  if (this->LoadTableButton)
    {
    this->LoadTableButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::RemoveLogicObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkQdecModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  
  //this->DebugOn();

  if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    vtkDebugMacro("Apply button pushed");
    return;
    }
  
  vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);

  if (filebrowse == this->LoadTableButton->GetWidget()  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a table file has been selected for loading...
    const char *fileName = this->LoadTableButton->GetWidget()->GetFileName();
    if (!fileName || strcmp(fileName,"") == 0)
      {
      vtkDebugMacro("Empty filename");
      this->LoadTableButton->GetWidget()->SetText ("None");
      return;
      }
    vtkDebugMacro("Trying to load file " << fileName);
    if (this->GetLogic()->LoadDataTable(fileName) == 0)
      {
      // failure
      vtkErrorMacro("Error loading table file " << fileName);
      this->LoadTableButton->GetWidget()->SetText ("None");
      }
    else
      {
      filebrowse->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      vtkDebugMacro("ProcessGUIEvents: was able to load file " << fileName);
      if (this->GetDebug())
        {
        this->GetLogic()->QDECProject->DumpDataTable ( stdout );
        }
      this->UpdateGUI();
      
      }
    return;
    }
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::UpdateMRML ()
{

}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::UpdateGUI ()
{
  if (this->GetLogic()->QDECProject)
    {
    // get the discrete and continuous factors
    vector< string > discreteFactors = this->GetLogic()->QDECProject->GetDiscreteFactors();
    string newFactors = "Discrete factors: ";      
    for (int i = 0; i < discreteFactors.size(); i++)
      {
      newFactors = newFactors + string("\n") + discreteFactors[i];
      }
    this->DiscreteFactorsLabel->SetText(newFactors.c_str());
      
    vector< string > continuousFactors = this->GetLogic()->QDECProject->GetContinousFactors();
    
    newFactors = "Continuous factors: ";
    for (int i = 0; i < continuousFactors.size(); i++)
      {
      newFactors = newFactors + string("\n") + continuousFactors[i];
      }
    this->ContinuousFactorsLabel->SetText(newFactors.c_str());
    
      
    // now populate the table    
    if (this->MultiColumnList)
      {
      if (this->MultiColumnList->GetWidget()->GetNumberOfRows() != 0)
        {
        //clear out the list box
        this->MultiColumnList->GetWidget()->DeleteAllRows();
        }
      // set up the correct number of columns
      if ((1 + discreteFactors.size() + continuousFactors.size()) !=
          this->MultiColumnList->GetWidget()->GetNumberOfColumns())
        {
        this->MultiColumnList->GetWidget()->DeleteAllColumns();
        this->MultiColumnList->GetWidget()->AddColumn("Subject");
        for (int i = 0; i < discreteFactors.size(); i++)
          {
          vtkDebugMacro("Adding a column for discrete factor: " << discreteFactors[i].c_str());
          this->MultiColumnList->GetWidget()->AddColumn(discreteFactors[i].c_str());
          }
        for (int i = 0; i < continuousFactors.size(); i++)
          {
          vtkDebugMacro("Adding a column for cont factor: " << continuousFactors[i].c_str());
          this->MultiColumnList->GetWidget()->AddColumn(continuousFactors[i].c_str());
          }
        }
      else
        {
        // just make sure the column names are correct
        int col = 1;
        for (int i = 0; i < discreteFactors.size(); i++)
          {
          vtkDebugMacro("Setting a column name at " << col << " for discrete factor: " << discreteFactors[i].c_str());
          this->MultiColumnList->GetWidget()->SetColumnName(col, discreteFactors[i].c_str());
          col++;
          }
        for (int i = 0; i < continuousFactors.size(); i++)
          {
          vtkDebugMacro("Setting a column name at " << col << " for continuous factor: " << continuousFactors[i].c_str());
          this->MultiColumnList->GetWidget()->SetColumnName(col, continuousFactors[i].c_str());
          col++;
          }
        }
      
      vector< string > subjectIDs = this->GetLogic()->QDECProject->GetDataTable()->GetSubjectIDs();
      vector< QdecSubject* > subjects = this->GetLogic()->QDECProject->GetDataTable()->GetSubjects();
      for (int row = 0; row < subjectIDs.size(); row++)
        {
        this->MultiColumnList->GetWidget()->AddRow();
        int col = 0;
        vtkDebugMacro("Setting cell at " << row << " " << col << " to " << subjectIDs[row].c_str());
        this->MultiColumnList->GetWidget()->SetCellText(row, col, subjectIDs[row].c_str());
        col++;
        // these are strings
        for (int i = 0; i < discreteFactors.size(); i++)
          {
          vtkDebugMacro("Setting cell at " << row << " " << col << " to factor " << discreteFactors[i].c_str() << " = " << subjects[row]->GetDiscreteFactor(discreteFactors[i].c_str()).c_str());
          this->MultiColumnList->GetWidget()->SetCellText(row, col, subjects[row]->GetDiscreteFactor(discreteFactors[i].c_str()).c_str());
          col++;
          }
        // these are numbers
        for (int i = 0; i < continuousFactors.size(); i++)
          {
          vtkDebugMacro("Setting cell at " << row << " " << col << " to factor " << continuousFactors[i].c_str() << " = " << subjects[row]->GetContinuousFactor(continuousFactors[i].c_str()));
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row, col, subjects[row]->GetContinuousFactor(continuousFactors[i].c_str()));
          col++;
          }
        }
      }
    } // end of valid QdecProject
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values

}




//---------------------------------------------------------------------------
void vtkQdecModuleGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();


  this->UIPanel->AddPage ( "QdecModule", "QdecModule", NULL );
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
    
  // HELP FRAME
  const char *help = "The Qdec Module provides support for the QDEC program from MGH: Query, Design, Estimate, Contrast.\nBefore you start: Set up a subjects directory and set your SUBJECTS_DIR environment variable (you'll need to restart Slicer3). In that directory, put the set of subjects you wish to analyse, along with the fsaverage subject. Then make a subdirectory qdec, and in it place your qdec.table.dat file along with any .level files describing the discrete factors (ie gender.levels will have 'male' and 'female' on two separate lines).";
  const char *about = "This work was supported by NA-MIC. See http://na-mic.org/Wiki/index.php/Projects/Slicer3/2007_Project_Week_QDEC_Slicer3_Integration for details on the integration project.";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QdecModule" );
  this->BuildHelpAndAboutFrame ( page, help, about );
  
  this->NAMICLabel = vtkKWLabel::New();
  this->NAMICLabel->SetParent ( this->GetLogoFrame() );
  this->NAMICLabel->Create();
  this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    
  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());

  // ---
  // Subjects Frame
  vtkSlicerModuleCollapsibleFrame *subjectsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  subjectsFrame->SetParent ( page );
  subjectsFrame->Create ( );
  subjectsFrame->SetLabelText ("Subjects");
  subjectsFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                subjectsFrame->GetWidgetName(),
                this->UIPanel->GetPageWidget("QdecModule")->GetWidgetName());

  this->LoadTableButton = vtkKWLoadSaveButtonWithLabel::New();
  this->LoadTableButton->SetParent ( subjectsFrame->GetFrame() );
  this->LoadTableButton->Create ( );
  this->LoadTableButton->SetLabelText ("Load Table Data File:");
  this->LoadTableButton->GetWidget()->SetText ("None");
  this->LoadTableButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Open QDEC table data file");
  this->LoadTableButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  this->LoadTableButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes("{ {All} {.*} } { {Data} {.dat} }");
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4", 
              this->LoadTableButton->GetWidgetName());

   // add the multicolumn list to show the data table
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( subjectsFrame->GetFrame() );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
  this->MultiColumnList->GetWidget()->MovableRowsOff();
  this->MultiColumnList->GetWidget()->MovableColumnsOff();

  this->MultiColumnList->GetWidget()->AddColumn("Subject");
  //not setting up all columns just yet
  app->Script ( "pack %s -fill both -expand true",
                this->MultiColumnList->GetWidgetName());
  this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
  
  
  this->DiscreteFactorsLabel = vtkKWLabel::New();
  this->DiscreteFactorsLabel->SetParent( subjectsFrame->GetFrame() );
  this->DiscreteFactorsLabel->Create();
  this->DiscreteFactorsLabel->SetText("Discrete factors: ");
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -in %s", 
              this->DiscreteFactorsLabel->GetWidgetName(),
              subjectsFrame->GetFrame()->GetWidgetName());

  this->ContinuousFactorsLabel = vtkKWLabel::New();
  this->ContinuousFactorsLabel->SetParent( subjectsFrame->GetFrame() );
  this->ContinuousFactorsLabel->Create();
  this->ContinuousFactorsLabel->SetText("Continuous factors: ");
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -in %s", 
              this->ContinuousFactorsLabel->GetWidgetName(),
              subjectsFrame->GetFrame()->GetWidgetName());

  this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent( subjectsFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10 -in %s", 
              this->ApplyButton->GetWidgetName(),
              subjectsFrame->GetFrame()->GetWidgetName());
  
  subjectsFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQdecModuleGUI::UpdateElement(int row, int col, char * str)
{
  vtkDebugMacro("UpdateElement: row = " << row << ", col = " << col << ", str = " << str << "\n");
  
  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
    {
    vtkDebugMacro("Valid row " << row << ", column " << col <<  ", valid columns are 0-" << this->MultiColumnList->GetWidget()->GetNumberOfColumns() << "\n");
    } 
  else
    {
    vtkErrorMacro ("Invalid row " << row << " or column " << col <<  ", valid columns are 0-" << this->MultiColumnList->GetWidget()->GetNumberOfColumns() << "\n");
    }
}
