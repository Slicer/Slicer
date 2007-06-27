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
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWLabel.h"
#include "vtkKWListBox.h"
#include "vtkKWListBoxWithScrollbars.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"

// for loading the outputs of the GLM processing
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerModelsLogic.h"

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
  this->SubjectsDirectoryButton = NULL;
  this->LoadTableButton = NULL;
  this->DesignEntry = NULL;
  this->ContinuousFactorsListBox = NULL;
  this->DiscreteFactorsListBox = NULL;
  this->ApplyButton = NULL;
  this->MultiColumnList = NULL;
  this->MeasureLabel = NULL;
  this->MeasureMenu = NULL;
  this->HemisphereLabel = NULL;
  this->HemisphereMenu = NULL;
  this->SmoothnessLabel = NULL;
  this->SmoothnessMenu = NULL;
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

  if ( this->SubjectsDirectoryButton )
    {
    this->SubjectsDirectoryButton->SetParent(NULL);
    this->SubjectsDirectoryButton->Delete();
    this->SubjectsDirectoryButton = NULL;
    }  

  if ( this->DesignEntry )
    {
    this->DesignEntry->SetParent(NULL);
    this->DesignEntry->Delete();
    this->DesignEntry = NULL;
    }
  
  if ( this->DiscreteFactorsListBox )
    {
    this->DiscreteFactorsListBox->SetParent ( NULL );
    this->DiscreteFactorsListBox->Delete();
    this->DiscreteFactorsListBox = NULL;
    }
 
   if ( this->ContinuousFactorsListBox )
    {
    this->ContinuousFactorsListBox->SetParent ( NULL );
    this->ContinuousFactorsListBox->Delete();
    this->ContinuousFactorsListBox = NULL;
    }

   if (this->MultiColumnList)
     {
     this->MultiColumnList->SetParent(NULL);
     this->MultiColumnList->Delete();
     this->MultiColumnList = NULL;
     }

   if ( this->MeasureLabel )
    {
    this->MeasureLabel->SetParent ( NULL );
    this->MeasureLabel->Delete();
    this->MeasureLabel = NULL;
    }

   if ( this->MeasureMenu )
    {
    this->MeasureMenu->SetParent ( NULL );
    this->MeasureMenu->Delete();
    this->MeasureMenu = NULL;
    }

    if ( this->HemisphereLabel )
    {
    this->HemisphereLabel->SetParent ( NULL );
    this->HemisphereLabel->Delete();
    this->HemisphereLabel = NULL;
    }

   if ( this->HemisphereMenu )
    {
    this->HemisphereMenu->SetParent ( NULL );
    this->HemisphereMenu->Delete();
    this->HemisphereMenu = NULL;
    }

    if ( this->SmoothnessLabel )
    {
    this->SmoothnessLabel->SetParent ( NULL );
    this->SmoothnessLabel->Delete();
    this->SmoothnessLabel = NULL;
    }

   if ( this->SmoothnessMenu )
    {
    this->SmoothnessMenu->SetParent ( NULL );
    this->SmoothnessMenu->Delete();
    this->SmoothnessMenu = NULL;
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
  this->SubjectsDirectoryButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->LoadTableButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkQdecModuleGUI::RemoveGUIObservers ( )
{
  if (this->SubjectsDirectoryButton)
    {
    this->SubjectsDirectoryButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  
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

  if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->DebugOn();
    vtkDebugMacro("Apply button pushed");
    
    std::string cont1 = "none";
    std::string cont2 = "none";
    std::string dis1 = "none";
    std::string dis2 = "none";
    int factorNum = 0;
    for (int i = 0; (i < this->ContinuousFactorsListBox->GetWidget()->GetWidget()->GetNumberOfItems()) && 
                    (factorNum < 2); i++)
      {
      if ( this->ContinuousFactorsListBox->GetWidget()->GetWidget()->GetSelectState(i))
        {
        vtkDebugMacro("\t" <<  this->ContinuousFactorsListBox->GetWidget()->GetWidget()->GetItem(i));
        if (factorNum == 0)
          {
          cont1 = this->ContinuousFactorsListBox->GetWidget()->GetWidget()->GetItem(i);
          }
        else if (factorNum == 1)
          {
          cont2 = this->ContinuousFactorsListBox->GetWidget()->GetWidget()->GetItem(i);
          }
        factorNum++;
        }
      }
    
    factorNum = 0;
    for (int i = 0; (i < this->DiscreteFactorsListBox->GetWidget()->GetWidget()->GetNumberOfItems()) &&
                    (factorNum < 2); i++)
      {
      if ( this->DiscreteFactorsListBox->GetWidget()->GetWidget()->GetSelectState(i))
        {
        vtkDebugMacro("\t" <<  this->DiscreteFactorsListBox->GetWidget()->GetWidget()->GetItem(i));
        if (factorNum == 0)
          {    
          dis1 = this->DiscreteFactorsListBox->GetWidget()->GetWidget()->GetItem(i);
      }
    else if (factorNum == 1)
      {
        dis2 = this->DiscreteFactorsListBox->GetWidget()->GetWidget()->GetItem(i);
      }
    factorNum++;
        }
      }
    vtkDebugMacro("Selected discrete factors = " << dis1.c_str() << " and " << dis2.c_str());
    vtkDebugMacro("Selected continuous factors = " << cont1.c_str() << " and " << cont2.c_str());
    vtkDebugMacro("Design name = " << this->DesignEntry->GetWidget()->GetValue());
    vtkDebugMacro("Measure = " << this->MeasureMenu->GetValue());
    vtkDebugMacro("Hemisphere = " << this->HemisphereMenu->GetValue());
    vtkDebugMacro("Smoothness = " << this->SmoothnessMenu->GetValue());

    // now pass it into the QDEC project to create a design
    int err = this->GetLogic()->QDECProject->CreateGlmDesign
      ( this->DesignEntry->GetWidget()->GetValue(),
    dis1.c_str(), dis2.c_str(), cont1.c_str(), cont2.c_str(),
    this->MeasureMenu->GetValue(),
    this->HemisphereMenu->GetValue(),
    atoi(this->SmoothnessMenu->GetValue()), 
    NULL ); // progress update GUI
    
    if (err == 0)
      {
    // success!
    vtkDebugMacro("Success in making the GLM design.");
    err = this->GetLogic()->QDECProject->RunGlmFit();
    if (err == 0)
      {
        vtkDebugMacro("Succeeded in running Glm Fit.");
      }
    else
      {
        vtkErrorMacro("Error running GLM Fit...");
      }
      }
    else
      {
    vtkErrorMacro("Error creating the GLM Design...");
      }


    
    // Make sure we got the results.
    QdecGlmFitResults* results =
      this->GetLogic()->QDECProject->GetGlmFitResults();
    assert( results );

    // fsaverage surface to load. This isn't returned in the results,
    // but we know where it is because it's just a normal subject in
    // the subjects dir.
    string fnSubjects = this->GetLogic()->QDECProject->GetSubjectsDir();
    string sHemi =  this->GetLogic()->QDECProject->GetHemi();
    string fnSurface = fnSubjects + "/fsaverage/surf/" + sHemi + ".inflated";
    vtkDebugMacro( "Surface: " << fnSurface.c_str() );
    
    // get the Models Logic and load the average surface file
    vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Models"))->GetLogic();
    vtkMRMLModelNode *modelNode = NULL;
    if (modelsLogic)
      {
    modelNode = modelsLogic->AddModel( fnSurface.c_str() );
    if (modelNode == NULL)
      {
        vtkErrorMacro("Unable to load average surface file " << fnSurface.c_str());
      }
    else
      {
        vtkDebugMacro("Loaded average surface file " << fnSurface.c_str());
      }
      }
    else
      {
    vtkErrorMacro("Unable to get at Models module to load average surface file.");
      }

    // load in the curvature overlay
    string curvFileName = fnSubjects + "/fsaverage/surf/" + sHemi + ".curv";
    vtkDebugMacro( "Surface: " << curvFileName.c_str() );
    string curvArrayName = "";
    if (modelsLogic && modelNode)
      {
    if (!modelsLogic->AddScalar(curvFileName.c_str(), modelNode))
        {
          vtkErrorMacro("Unable to add curvature to average model surface: " << curvFileName.c_str());
        }
    else
    {
        // grab the curvature array name
        curvArrayName = modelNode->GetActivePointScalarName("scalars");
        if (strcmp(curvArrayName.c_str(), "") == 0)
        {
            // hack it together
            curvArrayName = string("surf/") + sHemi + string(".curv");
            vtkDebugMacro("Failed to get the active point scalars name, so using curv array name = '" << curvArrayName.c_str() << "'");
        }
        vtkDebugMacro("Added the curvature file " << curvFileName.c_str() << ", got the curvature array name: '" << curvArrayName.c_str() << "'");
    }
    }

    // We should have the same number of questions as sig file. Each
    // sig file has a correpsponding question, and they are in the same
    // order in the vector.
    vector<string> lContrastQuestions = results->GetContrastQuestions();
    vector<string> lfnContrastSigs = results->GetContrastSigFiles();
    assert( lContrastQuestions.size() == lfnContrastSigs.size() );

    // Go through and get our sig files and questions.
    vector<string>::iterator fn;
    for( int nContrast = 0; 
     nContrast < results->GetContrastQuestions().size(); 
     nContrast++ ) {
     
      vtkDebugMacro( "Contrast " << nContrast << ": \""
             << lContrastQuestions[nContrast].c_str() << "\" in file " 
             << lfnContrastSigs[nContrast].c_str() );
      // load the sig file
      if (modelsLogic && modelNode)
    {
      if (!modelsLogic->AddScalar(lfnContrastSigs[nContrast].c_str(), modelNode))
        {
          vtkErrorMacro("Unable to add contrast to average model surface: " << lfnContrastSigs[nContrast].c_str());
        }
      else
    {
      if (strcmp(curvArrayName.c_str(), "") != 0)
        {
          // composite with the curv
          string sigArrayName = modelNode->GetActivePointScalarName("scalars");
          if (strcmp(sigArrayName.c_str(), "") == 0)
        {
          // hack it together
          std::string::size_type ptr = lfnContrastSigs[nContrast].find_last_of(std::string("/"));
          if (ptr != std::string::npos)
            {
              // find the dir name above
              std::string::size_type dirptr = lfnContrastSigs[nContrast].find_last_of(std::string("/"), ptr);
              if (dirptr != std::string::npos)
            {
              sigArrayName = lfnContrastSigs[nContrast].substr(++dirptr);
              vtkDebugMacro("created sig array name = '" << sigArrayName .c_str() << "'");
            }
              else
            {
              sigArrayName = lfnContrastSigs[nContrast].substr(++ptr);
            }
            }
          else
            {
              sigArrayName = lfnContrastSigs[nContrast];
            }
        }
          vtkDebugMacro("Compositing curv '" << curvArrayName.c_str() << "' with sig array '" << sigArrayName.c_str() << "'");
          modelNode->CompositeScalars(curvArrayName.c_str(), sigArrayName.c_str(), 2, 5, 1, 1, 0);
        }
    }
    }
    }
    
    // The regression coefficient and std dev files to load.
    string fnRegressionCoefficients = results->GetRegressionCoefficientsFile();
    string fnStdDev = results->GetResidualErrorStdDevFile();
    vtkDebugMacro( "Regressions coefficients: "
           << fnRegressionCoefficients.c_str() );
    vtkDebugMacro( "Std dev: " << fnStdDev.c_str() );
    
    // load the std dev file
    if (modelsLogic && modelNode)
      {
    if (!modelsLogic->AddScalar(fnStdDev.c_str(), modelNode))
      {
        vtkErrorMacro("Unable to add the residual errors std dev file " << fnStdDev.c_str() << " to the average surface model");
      }
      }
    // The fsgd file to plot.
    string fnFSGD = results->GetFsgdFile();
    vtkDebugMacro( "FSGD plot file: " << fnFSGD.c_str() );
    this->DebugOff();
    return;
    }

   vtkKWLoadSaveButton *dirbrowse = vtkKWLoadSaveButton::SafeDownCast(caller);

  if (dirbrowse == this->SubjectsDirectoryButton->GetWidget()  && event == vtkKWPushButton::InvokedEvent )
    {
    // If a table file has been selected for loading...
    const char *fileName = this->SubjectsDirectoryButton->GetWidget()->GetFileName();
    if (!fileName || strcmp(fileName,"") == 0)
      {
      vtkDebugMacro("Empty filename");
      this->SubjectsDirectoryButton->GetWidget()->SetText ("None");
      this->GetLogic()->QDECProject->SetSubjectsDir("None");
      return;
      }
    vtkDebugMacro("Setting the qdec projects subjects dir to " << fileName);
    this->GetLogic()->QDECProject->SetSubjectsDir(fileName);
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
    this->DiscreteFactorsListBox->GetWidget()->GetWidget()->DeleteAll();
    for (int i = 0; i < discreteFactors.size(); i++)
      {
      this->DiscreteFactorsListBox->GetWidget()->GetWidget()->Append(discreteFactors[i].c_str());
      }
      
    vector< string > continuousFactors = this->GetLogic()->QDECProject->GetContinousFactors();

    this->ContinuousFactorsListBox->GetWidget()->GetWidget()->DeleteAll();
    for (int i = 0; i < continuousFactors.size(); i++)
      {
      this->ContinuousFactorsListBox->GetWidget()->GetWidget()->Append(continuousFactors[i].c_str());
      }    
      
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

  this->SubjectsDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
  this->SubjectsDirectoryButton->SetParent ( subjectsFrame->GetFrame() );
  this->SubjectsDirectoryButton->Create ( );
  this->SubjectsDirectoryButton->SetLabelText ("Set SUBJECTS_DIR:");
  this->SubjectsDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Select the FreeSurfer Subjects directory");
  this->SubjectsDirectoryButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  if (getenv("SUBJECTS_DIR") == NULL)
    {
    this->SubjectsDirectoryButton->GetWidget()->SetText ("None");
    this->SubjectsDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( "." );
    }
  else
    {
    this->SubjectsDirectoryButton->GetWidget()->SetText (getenv("SUBJECTS_DIR"));
    this->SubjectsDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetInitialFileName(getenv("SUBJECTS_DIR"));
    }
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4", 
              this->SubjectsDirectoryButton->GetWidgetName());
  
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

  // ---
  // Design Frame
  vtkSlicerModuleCollapsibleFrame *designFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  designFrame->SetParent ( page );
  designFrame->Create ( );
  designFrame->SetLabelText ("Design");
  designFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                designFrame->GetWidgetName(),
                this->UIPanel->GetPageWidget("QdecModule")->GetWidgetName());

  this->DesignEntry = vtkKWEntryWithLabel::New();
  this->DesignEntry->SetParent( designFrame->GetFrame());
  this->DesignEntry->SetLabelText ("Design Name:");
  this->DesignEntry->Create();
  this->DesignEntry->GetWidget()->SetValue("Untitled");
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -in %s", 
              this->DesignEntry->GetWidgetName(),
              designFrame->GetFrame()->GetWidgetName());
  
  this->DiscreteFactorsListBox = vtkKWListBoxWithScrollbarsWithLabel::New();
  this->DiscreteFactorsListBox->SetParent( designFrame->GetFrame() );
  this->DiscreteFactorsListBox->SetLabelText("Discrete Factors (choose up to two):" );
  this->DiscreteFactorsListBox->Create();
  this->DiscreteFactorsListBox->GetWidget()->GetWidget()->SetSelectionModeToMultiple();
  this->DiscreteFactorsListBox->GetWidget()->GetWidget()->ExportSelectionOff();
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -in %s", 
              this->DiscreteFactorsListBox->GetWidgetName(),
              designFrame->GetFrame()->GetWidgetName());
  
  this->ContinuousFactorsListBox = vtkKWListBoxWithScrollbarsWithLabel::New();
  this->ContinuousFactorsListBox->SetParent( designFrame->GetFrame() );
  this->ContinuousFactorsListBox->SetLabelText("Continuous Factors (choose two):");
  this->ContinuousFactorsListBox->Create();
  this->ContinuousFactorsListBox->GetWidget()->GetWidget()->SetSelectionModeToMultiple();
  this->ContinuousFactorsListBox->GetWidget()->GetWidget()->ExportSelectionOff();
  app->Script("pack %s -side top -anchor nw -padx 2 -pady 4 -in %s", 
              this->ContinuousFactorsListBox->GetWidgetName(),
              designFrame->GetFrame()->GetWidgetName());

  vtkKWFrameWithLabel* measuresFrame = vtkKWFrameWithLabel::New();
  measuresFrame->SetParent( designFrame->GetFrame() );
  measuresFrame->Create();
  measuresFrame->SetLabelText( "Measures" );
  this->Script( "pack %s -fill x", measuresFrame->GetWidgetName() );

  
  int nRow = 0;
  this->MeasureLabel = vtkKWLabel::New();
  this->MeasureLabel->SetParent(measuresFrame->GetFrame() );
  this->MeasureLabel->Create();
  this->MeasureLabel->SetText("Measure: ");
  this->MeasureLabel->SetJustificationToRight();
  app->Script( "grid %s -column 0 -row %d -sticky ne -in %s",
               this->MeasureLabel->GetWidgetName(), nRow,
               measuresFrame->GetFrame()->GetWidgetName());

  this->MeasureMenu = vtkKWMenuButton::New();
  this->MeasureMenu->SetParent(measuresFrame->GetFrame() );
  this->MeasureMenu->Create();
  this->MeasureMenu->GetMenu()->AddRadioButton("thickness");
  this->MeasureMenu->GetMenu()->AddRadioButton("sulc");
  this->MeasureMenu->GetMenu()->AddRadioButton("curv");
  this->MeasureMenu->GetMenu()->AddRadioButton("jacobian");
  this->MeasureMenu->GetMenu()->AddRadioButton("area");
  this->MeasureMenu->SetValue("thickness");
  app->Script( "grid %s -column 1 -row %d -sticky nw -in %s",
               this->MeasureMenu->GetWidgetName(), nRow,
               measuresFrame->GetFrame()->GetWidgetName());
  
  nRow++;
  this->HemisphereLabel = vtkKWLabel::New();
  this->HemisphereLabel->SetParent(measuresFrame->GetFrame() );
  this->HemisphereLabel->Create();
  this->HemisphereLabel->SetText("Hemisphere: ");
  this->HemisphereLabel->SetJustificationToRight();
  app->Script( "grid %s -column 0 -row %d -sticky ne -in %s",
               this->HemisphereLabel->GetWidgetName(), nRow,
               measuresFrame->GetFrame()->GetWidgetName());

  this->HemisphereMenu = vtkKWMenuButton::New();
  this->HemisphereMenu->SetParent(measuresFrame->GetFrame() );
  this->HemisphereMenu->Create();
  this->HemisphereMenu->GetMenu()->AddRadioButton("lh");
  this->HemisphereMenu->GetMenu()->AddRadioButton("rh");
  this->HemisphereMenu->SetValue("lh");
  app->Script( "grid %s -column 1 -row %d -sticky nw -in %s",
               this->HemisphereMenu->GetWidgetName(), nRow,
               measuresFrame->GetFrame()->GetWidgetName());
  nRow++;

  this->SmoothnessLabel = vtkKWLabel::New();
  this->SmoothnessLabel->SetParent(measuresFrame->GetFrame());
  this->SmoothnessLabel->Create();
  this->SmoothnessLabel->SetText("Smoothness (FWHM, mm): ");
  this->SmoothnessLabel->SetJustificationToRight();
  app->Script( "grid %s -column 0 -row %d -sticky ne -in %s",
               this->SmoothnessLabel->GetWidgetName(), nRow,
               measuresFrame->GetFrame()->GetWidgetName());

  this->SmoothnessMenu = vtkKWMenuButton::New();
  this->SmoothnessMenu->SetParent(measuresFrame->GetFrame());
  this->SmoothnessMenu->Create();
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "0" );
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "5" );
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "10" );
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "15" );
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "20" );
  this->SmoothnessMenu->GetMenu()->AddRadioButton( "25" );
  this->SmoothnessMenu->SetValue( "10" );
  this->Script( "grid %s -column 1 -row %d -sticky nw -in %s",
                this->SmoothnessMenu->GetWidgetName(), nRow,
                measuresFrame->GetFrame()->GetWidgetName());

  // weight the grid
  for( int nRowConfigure = 0; nRowConfigure <= nRow; nRowConfigure++ )
    {
    this->Script( "grid rowconfigure %s %d -pad 4",
                  measuresFrame->GetFrame()->GetWidgetName(),
                  nRowConfigure );
    }
  
  this->ApplyButton = vtkKWPushButton::New();
  this->ApplyButton->SetParent( designFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10 -in %s", 
              this->ApplyButton->GetWidgetName(),
              designFrame->GetFrame()->GetWidgetName());
  

  measuresFrame->Delete();
  designFrame->Delete();
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
