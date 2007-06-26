/**
 * @file  QdecProject.cpp
 * @brief API class containing all qdec subject data and methods
 *
 * Top-level interface class containing all data associated with a users
 * subject group, and potentially mri_glmfit processed data associated with
 * that group.
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/23 21:20:58 $
 *    $Revision: 1.5 $
 *
 * Copyright (C) 2007,
 * The General Hospital Corporation (Boston, MA).
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */

#include <stdexcept>
#include <sstream>

#include "QdecProject.h"


// Constructors/Destructors
//

QdecProject::QdecProject ( )
{
  this->mfnProjectFile = "qdec.xml";
  this->mDataTable = new QdecDataTable();
  this->mGlmDesign = new QdecGlmDesign( this->mDataTable );
  this->mGlmFitter = new QdecGlmFit();
}

QdecProject::~QdecProject ( )
{
  delete this->mDataTable;
  delete this->mGlmDesign;
  delete this->mGlmFitter;
}

//
// Methods
//

/**
 * Load a project file (containing all necessary info to begin working
 * either on a new project, or to continue working using results from a
 * prior saved work session).
 * @return int
 * @param  isFileName
 */
int QdecProject::LoadProjectFile ( const char* isFileName )
{
  return 0;
}


/**
 * Save all necessary information pertaining to this project (all subject
 * data, any results, any user preferences).
 * @return int
 * @param  isFileName
 */
int QdecProject::SaveProjectFile ( const char* isFileName )
{
  return 0;
}


/**
 * @return int
 * @param  isFileName
 */
int QdecProject::LoadDataTable ( const char* isFileName )
{
  return this->mDataTable->Load ( isFileName );
}


/**
 * @return void
 * @param  iFilePointer
 */
void QdecProject::DumpDataTable ( FILE* iFilePointer )
{
  return this->mDataTable->Dump ( iFilePointer );
}

/**
 * @return int
 * @param  isFileName
 */
int QdecProject::SaveDataTable ( const char* isFileName )
{
  return this->mDataTable->Save ( isFileName );
}


/**
 * @return QdecDataTable*
 */
QdecDataTable* QdecProject::GetDataTable ( )
{
  return this->mDataTable;
}


/**
 * @return string
 */
string QdecProject::GetSubjectsDir ( )
{
  return this->mGlmDesign->GetSubjectsDir();
}


/**
 * @param  ifnSubjectsDir
 */
void QdecProject::SetSubjectsDir ( const char* ifnSubjectsDir )
{
  this->mGlmDesign->SetSubjectsDir( ifnSubjectsDir );
}


/**
 * @return string
 */
string QdecProject::GetAverageSubject ( )
{
  return this->mGlmDesign->GetAverageSubject();
}


/**
 * @param  isSubjectName
 */
void QdecProject::SetAverageSubject ( const char* isSubjectName )
{
  this->mGlmDesign->SetAverageSubject( isSubjectName );
}


/**
 * @return string
 */
string QdecProject::GetWorkingDir ( )
{
  return this->mGlmDesign->GetWorkingDir();
}


/**
 * @return 0 if ok, 1 on error
 * @param  isWorkingDir
 */
int QdecProject::SetWorkingDir ( const char* isWorkingDir )
{
  return this->mGlmDesign->SetWorkingDir( isWorkingDir );
}


/**
 * @return vector< string >
 */
vector< string > QdecProject::GetSubjectIDs ( )
{
  return this->mDataTable->GetSubjectIDs();
}


/**
 * @return vector< string >
 */
vector< string > QdecProject::GetDiscreteFactors ( )
{
  return this->mDataTable->GetDiscreteFactors();
}


/**
 * @return vector< string >
 */
vector< string > QdecProject::GetContinousFactors ( )
{
  return this->mDataTable->GetContinuousFactors();
}


/**
 * @return string
 */
string QdecProject::GetHemi ( )
{
  return this->mGlmDesign->GetHemi();
}


/**
 * From the given design parameters, this creates the input data required by
 * mri_glmfit:
 *  - the 'y' data (concatenated subject volumes)
 *  - the FSGD file
 *  - the contrast vectors, as .mat files
 * and writes this data to the specified working directory.
 * @return int
 * @param  isName
 * @param  isFirstDiscreteFactor
 * @param  isSecondDiscreteFactor
 * @param  isFirstContinuousFactor
 * @param  isSecondContinuousFactor
 * @param  isMeasure
 * @param  isHemi
 * @param  iSmoothnessLevel
 * @param  iProgressUpdateGUI
 */
int QdecProject::CreateGlmDesign ( const char* isName,
                                   const char* isFirstDiscreteFactor,
                                   const char* isSecondDiscreteFactor,
                                   const char* isFirstContinuousFactor,
                                   const char* isSecondContinuousFactor,
                                   const char* isMeasure,
                                   const char* isHemi,
                                   int iSmoothnessLevel,
                                   ProgressUpdateGUI* iProgressUpdateGUI )
{
  return this->mGlmDesign->Create ( this->mDataTable,
                                    isName,
                                    isFirstDiscreteFactor,
                                    isSecondDiscreteFactor,
                                    isFirstContinuousFactor,
                                    isSecondContinuousFactor,
                                    isMeasure,
                                    isHemi,
                                    iSmoothnessLevel,
                                    iProgressUpdateGUI );
}


/**
 * @return int
 */
int QdecProject::RunGlmFit ( )
{
  return this->mGlmFitter->Run( mGlmDesign );
}


/**
 * @return QdecGlmFitResults
 */
QdecGlmFitResults* QdecProject::GetGlmFitResults ( )
{
  return this->mGlmFitter->GetResults();
}


/**
 * Run mri_label2label on each subject, mapping the label that was drawn on 
 * the average surface onto each subject. Optionally supply a GUI manager
 * to allow posting progress info.
 * @return int
 * @param  ifnLabel
 * @param  iProgressUpdateGUI
 */
int QdecProject::GenerateMappedLabelForAllSubjects 
( const char* ifnLabel,
  ProgressUpdateGUI* iProgressUpdateGUI )
{
  vector< string > subjects = this->GetSubjectIDs();
  int numSubjects = this->GetSubjectIDs().size();
  float stepIncrement = 100.0 / numSubjects-1;
  int nStep = 1;

  if ( 0 == numSubjects )
    throw runtime_error( "Zero subjects! Cannot run mri_label2label\n" );

  if( iProgressUpdateGUI )
  {
    iProgressUpdateGUI->BeginActionWithProgress
      ( "Running mri_label2label..." );
  }
      
  for( int i=0; i < numSubjects; i++ )
  {
    // build a command line for this subject
    stringstream ssCommand;
    ssCommand << "mri_label2label"
              << " --srclabel " << ifnLabel
              << " --srcsubject " << this->GetAverageSubject()
              << " --trgsubject " << subjects[i]
              << " --trglabel " << ifnLabel
              << " --regmethod surface"
              << " --hemi " << this->GetHemi();

     // Now run the command.
    if( iProgressUpdateGUI )
    {
      string status = "Running mri_label2label on subject '";
      status += subjects[i];
      status += "'...";
      iProgressUpdateGUI->UpdateProgressMessage( status.c_str() );
      iProgressUpdateGUI->UpdateProgressPercent
        ( (float)nStep++ * stepIncrement );
    }
    char* sCommand = strdup( ssCommand.str().c_str() );
    printf( "\n----------------------------------------------------------\n" );
    printf( "%s\n", sCommand );
    fflush(stdout);fflush(stderr);
    int rRun = system( sCommand );
    if ( -1 == rRun )
      throw runtime_error( "system call failed: " + ssCommand.str() );
    if ( rRun > 0 )
      throw runtime_error( "command failed: " + ssCommand.str() );
    free( sCommand );
  }

  if( iProgressUpdateGUI )
  {
    iProgressUpdateGUI->UpdateProgressMessage( "Completed mri_label2label." );
    iProgressUpdateGUI->UpdateProgressPercent( 100 );
    iProgressUpdateGUI->EndActionWithProgress();
  }

  return 0;
}


