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

#include <sstream>
#include <stdexcept>

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
  int rRun = system( sCommand );
  if ( -1 == rRun )
    throw runtime_error( "system call failed: " + ssCommand.str() );
  if ( rRun > 0 )
    throw runtime_error( "command failed: " + ssCommand.str() );
  free( sCommand );

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
  assert( glmFitResults );
  this->mGlmFitResults = glmFitResults;

  if( iGlmDesign->GetProgressUpdateGUI() )
  {
    iGlmDesign->GetProgressUpdateGUI()->EndActionWithProgress();
  }

  return 0;
}


/**
 * @return QdecGlmFitResults
 */
QdecGlmFitResults* QdecGlmFit::GetResults ( )
{
  return this->mGlmFitResults;
}

