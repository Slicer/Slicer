/**
 * @file  QdecUtilities.cxx
 * @brief Misc utilities
 *
 * A bunch of misc utilities for Qdec.
 */
/*
 * Original Author: Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/22 18:25:11 $
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

#include "QdecUtilities.h"

#include <fstream>
#include <cstring>
#include <stdexcept>

using namespace std;

void
QdecUtilities::AssertFileIsReadable ( string const& ifn ) {
  
  if( !QdecUtilities::IsFileReadable( ifn ) )
    throw runtime_error( string("Couldn't find file ") + ifn );

}

bool
QdecUtilities::IsFileReadable ( string const& ifn ) {

  ifstream f( ifn.c_str(), ios::in );
  if( !f || f.bad() )
    return false;

  return true;
}

const char *
QdecUtilities::FileNamePath(const char *fname, char *pathName)
{
  char *slash ;
  strcpy(pathName, fname) ;
  slash = strrchr(pathName, '/') ;
  if (slash) *slash = 0 ; /* remove file name */
  else
#ifndef Linux
    getcwd((char*)pathName, MAXPATHLEN-1) ;  /* no path at all, must be cwd */
#else
#if 0
  getcwd(pathName, MAXPATHLEN-1) ;
#else
  sprintf((char*)pathName, ".") ;
#endif
#endif

  return(pathName) ;
}
