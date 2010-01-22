/**
 * @file  QdecUtilities.h
 * @brief Misc utilities
 *
 * A bunch of misc utilities for Qdec.
 */
/*
 * Original Author: Kevin Teich
 */

#ifndef QdecUtilities_h
#define QdecUtilities_h

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

#include <stdio.h>
#ifndef MAXPATHLEN
#ifndef _WIN32
#include <sys/param.h>
#else
#include <direct.h>
#define MAXPATHLEN 1024
#endif
#endif

//BTX
#include <string>


using namespace std;

class VTK_Qdec_EXPORT QdecUtilities
{

public:

  /// Calls IsFileReadable and throws an error if it fails.
  static void AssertFileIsReadable ( string const& ifn );

  /// Returns true if a file exists and is openable with read
  /// permissions.
  static bool IsFileReadable ( string const& ifn );

  /// extract the path name from a file name and return a pointer to it
  static const char *FileNamePath(const char *fname, char *pathName);

};
//ETX
#endif
