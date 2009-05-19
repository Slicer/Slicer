/**
 * @file  QdecUtilities.cxx
 * @brief Misc utilities
 *
 * A bunch of misc utilities for Qdec.
 */
/*
 * Original Author: Kevin Teich
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
