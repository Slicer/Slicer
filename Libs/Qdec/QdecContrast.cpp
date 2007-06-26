/**
 * @file  QdecContrast.cpp
 * @brief Stores a GLM contrast vector
 *
 * Stores a GLM contrast vector associated with a particular design (being
 * based on user selected factors, formulating a hypothesis to test).
 * Stores the name associated with it (to go in the .fsgd file), the
 * human-readable question (for use by GUI), and the vector itself (as an
 * int array).
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/20 23:15:23 $
 *    $Revision: 1.2 $
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

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <cassert>

#include "QdecContrast.h"


// Constructors/Destructors
//

QdecContrast::QdecContrast ( vector< double > iaVector,
                             string isName,
                             string isQuestion )
{
  assert( iaVector.size() );
  this->maVector = iaVector;
  this->msName = isName;
  this->msQuestion = isQuestion;
}

QdecContrast::~QdecContrast ( )
{ }

//
// Methods
//


/**
 * @return string
 */
string QdecContrast::GetName ( )
{
  return this->msName;
}


/**
 * @return string
 */
string QdecContrast::GetQuestion ( )
{
  return this->msQuestion;
}


/**
 * @return string
 */
string QdecContrast::GetContrastStr ( )
{
  string contrast = "";
  for (unsigned int i=0; i < this->maVector.size();)
  {
    char tmpstr[1000];
    sprintf(tmpstr,"% 2.3f",this->maVector[i]);
    contrast += strdup(tmpstr);
    if (++i < this->maVector.size()) contrast += "  ";
  }
  contrast += ";\n";

  return contrast;
}


/**
 * Writes the contrast vector to a .mat file, which is readable by matlab, and
 * mri_glmfit.
 * @return int
 * @param string ifnWorkingDir
 */
int QdecContrast::WriteDotMatFile ( string ifnWorkingDir )
{
  string dirName = ifnWorkingDir + "/contrasts/";
#ifndef _WIN32
  int err = mkdir( dirName.c_str(), 0777);
#else
  int err = mkdir (dirName.c_str());
#endif
  if( err != 0 && errno != EEXIST )
  {
    fprintf( stderr,
             "ERROR: QdecContrast::WriteDotMatFile: "
             "could not create directory %s\n",
             dirName.c_str());
    return(-1);
  }

  this->mfnDotMatFileName = dirName; 
  this->mfnDotMatFileName += this->GetName();
  this->mfnDotMatFileName += ".mat";

  FILE* fp = fopen( this->mfnDotMatFileName.c_str(), "w");
  if( ! fp )
  {
    fprintf( stderr,
             "ERROR: QdecContrast::WriteDotMatFile: "
             "could not create file %s\n",
             this->mfnDotMatFileName.c_str());
    return(-2);
  }

  for(unsigned int i=0; i < this->maVector.size(); i++)
  {
    fprintf( fp, "%+4.5f ", this->maVector[i] );
  }
  fprintf( fp, "\n" );
  fclose( fp );

  return 0;
}


/**
 * @return string
 */
string QdecContrast::GetDotMatFileName ( )
{
  return this->mfnDotMatFileName;
}

