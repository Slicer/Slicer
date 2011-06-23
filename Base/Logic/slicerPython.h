/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

//
/// workarounds to incorporate python into slicer's build without warnings
//

#ifdef Slicer_USE_PYTHON

/// Python will define _LARGEFILE_SOURCE to 1, while kwsys simply defines it - undef it here
/// to avoid getting a warning during the build

#  ifdef _LARGEFILE_SOURCE
#  undef _LARGEFILE_SOURCE
#  endif 

/* Undefine macros that Python.h defines to avoid redefinition warning.  */
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

/// If debug, Python wants pythonxx_d.lib, so fake it out

#  ifdef _DEBUG
#    undef _DEBUG
#    include <Python.h>
#    define _DEBUG
#  else
#    include <Python.h>
#  endif

#endif
