/**
 * @file  QdecGlmFit.cpp
 * @brief Wrapper for mri_glmfit.
 *
 * Run mri_glmfit, given its input data, and puts the output in the specified
 * working directory.
 */
/*
 * Original Authors: Nick Schmansky and Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/23 21:20:58 $
 *    $Revision: 1.6 $
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

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <fstream>

#include "QdecGlmFit.h"
#include "QdecUtilities.h"


// Constructors/Destructors
//

QdecGlmFit::QdecGlmFit ( )
{
  this->mGlmFitResults = NULL; 
}

QdecGlmFit::~QdecGlmFit ( )
{ 
  if( this->mGlmFitResults ) delete this->mGlmFitResults;
}

//
// Methods
//

/**
 *
 * Run mri_glmfit, using the design parameters specified in the given 
 * QdecGlmDesign object.  Results are written to disk, and the 
 * GetGlmFitResults method is called to get pointers to these results.
 *
 * @return int
 * @param  iGlmDesign
 */
int QdecGlmFit::Run ( QdecGlmDesign* iGlmDesign )
{
  if ( ! iGlmDesign->IsValid() )
  {
    fprintf(stderr,"\nERROR: QdecGlmFit::Run: QdecGlmDesign is invalid!\n");
    return -1;
  }

  if( iGlmDesign->GetProgressUpdateGUI() )
  {
    iGlmDesign->GetProgressUpdateGUI()->BeginActionWithProgress
      ( "Running GLM..." );
  }

  // We need to build a command line.
  stringstream ssCommand;
  ssCommand << "mri_glmfit --y " << iGlmDesign->GetYdataFileName()
            << " --fsgd " << iGlmDesign-> GetFsgdFileName()
            << " --glmdir " << iGlmDesign->GetWorkingDir()
            << " --surf " << iGlmDesign->GetAverageSubject()
            << " " << iGlmDesign->GetHemi();

  // Append the contrast option to the command for each contrast we
  // have.
  vector< string > contrastFileNames = iGlmDesign->GetContrastFileNames();
  for ( unsigned int i=0; i < iGlmDesign->GetContrastFileNames().size(); i++ )
  {
    ssCommand << " --C " << contrastFileNames[i];
  }

  // Now run the command.
  if( iGlmDesign->GetProgressUpdateGUI() )
  {
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage
      ( "Running GLM..." );
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent( 70 );
  }
  char* sCommand = strdup( ssCommand.str().c_str() );
  fflush(stdout);fflush(stderr);

  // Try running the command. 
  
  // =======================================================================
  // 6/27/07 RKT: This command will normally pass on Linux machines,
  // but will fail on Windows machines. For the NAMIC demo, when the
  // command fails, we'll create a fake result using data that we know
  // exists on disk called demo-analyzed-data. To remove this special
  // code, uncomment the commented code below that throws on an error,
  // remove if "if(rRun==)" statement, and remove the "else{...}"
  // portion.
  int rRun = system( sCommand );

  // Temporarily disabling the code to throw on failure.
//   if ( -1 == rRun )
//     throw runtime_error( "system call failed: " + ssCommand.str() );
//   if ( rRun > 0 )
//     throw runtime_error( "command failed: " + ssCommand.str() );
//   free( sCommand );

  if( rRun == 0 )
    {

    //
    // Concatenate the output.
    //
    // Now we have files in our working directory for each contrast we
    // have. We want to concatenate the sig.mgh file in those
    // directories into a final concatenated output file.  
    
    // First check if the output files exist. Add the filenames to a vector.
    if( iGlmDesign->GetProgressUpdateGUI() )
      {
      iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage
        ( "Finding output..." );
      iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent( 80 );
      }
    vector< string > contrastNames = iGlmDesign->GetContrastNames();
    vector< string > lfnSigFiles;
    for ( unsigned int i=0; i < iGlmDesign->GetContrastNames().size(); i++ )
      {
      string sigFile = iGlmDesign->GetWorkingDir();
      sigFile += "/";
      sigFile += contrastNames[i];
      sigFile += "/sig.mgh";
      
      // Check if it exists and is readable.
      QdecUtilities::AssertFileIsReadable( sigFile.c_str() );
      
      lfnSigFiles.push_back( sigFile );
      }
    
    // Go through and concatenate copy all the volumes.
    if( iGlmDesign->GetProgressUpdateGUI() )
      {
      iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage
        ( "Concatenating output scalars..." );
      iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent( 90 );
      }
    
    // form output filename
    string fnContrastsOutput;
    fnContrastsOutput = iGlmDesign->GetWorkingDir();
    fnContrastsOutput += "/contrasts.sig.mgh";
    
    stringstream ssCommand2;
    ssCommand2 << "mri_concat ";
    // subject inputs...
    for ( unsigned int i=0; i < lfnSigFiles.size(); i++ )
      {
      ssCommand2 << lfnSigFiles[i] << " ";
      }
    // and the output filename...
    ssCommand2 << "--o " << fnContrastsOutput;
    
    // Now run the command.
    sCommand = strdup( ssCommand2.str().c_str() );
    rRun = system( sCommand );
    if ( -1 == rRun )
      throw runtime_error( "system call failed: " + ssCommand2.str() );
    if ( rRun > 0 )
      throw runtime_error( "command failed: " + ssCommand2.str() );
    free( sCommand );
    
    string fnResidualErrorStdDevFile = iGlmDesign->GetWorkingDir();
    fnResidualErrorStdDevFile += "/rstd.mgh";
    string fnRegressionCoefficientsFile = iGlmDesign->GetWorkingDir();
    fnRegressionCoefficientsFile += "/beta.mgh";
    string fnFsgdFile = iGlmDesign->GetWorkingDir();
    fnFsgdFile += "/y.fsgd";
    
    // Now write the result information to a GlmFitResults object
    QdecGlmFitResults* glmFitResults = 
      new QdecGlmFitResults( iGlmDesign, 
                 lfnSigFiles, 
                 fnContrastsOutput,
                 fnResidualErrorStdDevFile,
                 fnRegressionCoefficientsFile,
                 fnFsgdFile );
    if (!glmFitResults )
      {
      fprintf(stderr,"\nERROR: QdecGlmFit::Run: glm fit results are invalid!");
      return -2;
      }
    this->mGlmFitResults = glmFitResults;
    
    if( iGlmDesign->GetProgressUpdateGUI() )
      {
      iGlmDesign->GetProgressUpdateGUI()->EndActionWithProgress();
      }
    }
  else 
    {
    // =======================================================================
    // 6/27/07 RKT: This is special code to fake the results of
    // running mri_glmfit on a machine that doesn't have that binary
    // installed. This is being done for the NAMIC Qdec demo which is
    // being run on a Windows machine. To remove this code, 

    // This is the known fake data dir.
    string fnSubjectsDir = iGlmDesign->GetSubjectsDir();
    string fnWorkingDir = fnSubjectsDir + "/qdec/demo-analyzed-data";
    iGlmDesign->SetWorkingDir( fnWorkingDir.c_str() );
    
    // Build a list of contrast files.
    vector< string > contrastNames = iGlmDesign->GetContrastNames();
    vector< string > lfnSigFiles;
    for ( unsigned int i=0; i < iGlmDesign->GetContrastNames().size(); i++ )
      {
      string sigFile = iGlmDesign->GetWorkingDir();
      sigFile += "/";
      sigFile += contrastNames[i];
      sigFile += "/sig.mgh";
      
      // Check if it exists and is readable.
      QdecUtilities::AssertFileIsReadable( sigFile.c_str() );
      
      lfnSigFiles.push_back( sigFile );
      }
    
    // form output filename
    string fnContrastsOutput;
    fnContrastsOutput = iGlmDesign->GetWorkingDir();
    fnContrastsOutput += "/contrasts.sig.mgh";
    
    // Other data names.
    string fnResidualErrorStdDevFile = iGlmDesign->GetWorkingDir();
    fnResidualErrorStdDevFile += "/rstd.mgh";
    string fnRegressionCoefficientsFile = iGlmDesign->GetWorkingDir();
    fnRegressionCoefficientsFile += "/beta.mgh";
    string fnFsgdFile = iGlmDesign->GetWorkingDir();
    fnFsgdFile += "/y.fsgd";
    
    // Make the results.
    QdecGlmFitResults* glmFitResults = 
      new QdecGlmFitResults( iGlmDesign, 
                             lfnSigFiles, 
                             fnContrastsOutput,
                             fnResidualErrorStdDevFile,
                             fnRegressionCoefficientsFile,
                             fnFsgdFile );
    
    // Check it.
    if (!glmFitResults)
      {
      fprintf(stderr,"\nERROR: QdecGlmFit::Run: Unable to load demo data");
      return -3;
      }

    // Make it ours.
    this->mGlmFitResults = glmFitResults;

    // return a code to advertise that loaded demo data
    return 1;
    // =======================================================================
    }
return 0;
}

int QdecGlmFit::CreateResultsFromCachedData ( QdecGlmDesign* iGlmDesign )
{

  // Build contrast filenames from our contrast names.
  vector<string> lContrastNames = iGlmDesign->GetContrastNames();
  vector<string> lfnSigFiles;
  vector<string>::iterator tContrastName;
  for( tContrastName = lContrastNames.begin();
       tContrastName != lContrastNames.end(); ++tContrastName )
    {

    // Build the name.
    string fnSigFile = iGlmDesign->GetWorkingDir();
    fnSigFile += "/";
    fnSigFile += *tContrastName;
    fnSigFile += "/sig.mgh";

    // Check if it exists and is readable.
    ifstream fInput( fnSigFile.c_str(), std::ios::in );
    if( !fInput || fInput.bad() )
      throw runtime_error( string("Couldn't open file " ) + fnSigFile );

    lfnSigFiles.push_back( fnSigFile );
    }

  // Make our other filenames.
  string fnContrastsOutput;
  fnContrastsOutput = iGlmDesign->GetWorkingDir();
  fnContrastsOutput += "/contrasts.sig.mgh";

  string fnResidualErrorStdDevFile = iGlmDesign->GetWorkingDir();
  fnResidualErrorStdDevFile += "/rstd.mgh";

  string fnRegressionCoefficientsFile = iGlmDesign->GetWorkingDir();
  fnRegressionCoefficientsFile += "/beta.mgh";

  string fnFsgdFile = iGlmDesign->GetWorkingDir();
  fnFsgdFile += "/y.fsgd";

  // Now write the result information to a GlmFitResults object
  QdecGlmFitResults* glmFitResults =
    new QdecGlmFitResults( iGlmDesign,
                           lfnSigFiles,
                           fnContrastsOutput,
                           fnResidualErrorStdDevFile,
                           fnRegressionCoefficientsFile,
                           fnFsgdFile );
  assert( glmFitResults );

  delete this->mGlmFitResults;
  this->mGlmFitResults = glmFitResults;

  return 0;
}



/**
 * @return QdecGlmFitResults
 */
QdecGlmFitResults* QdecGlmFit::GetResults ( )
{
  return this->mGlmFitResults;
}

