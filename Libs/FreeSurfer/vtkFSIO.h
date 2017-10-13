/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSIO_h
#define __vtkFSIO_h

#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtk_zlib.h>

// STD includes
#include <cstdio>

/// \brief Some IO functions for irregular FreeSurface files.
///
/// Some simple functions for doing silly things like reading three
/// byte ints, common in FreeSurfer file types.
namespace vtkFSIO {

  /// These use FILE types instead of file streams for no good reason,
  /// simply because the old code from which this is adapted is C-based.
  int VTK_FreeSurfer_EXPORT ReadShort (FILE* iFile, short& oShort);
  int VTK_FreeSurfer_EXPORT ReadInt (FILE* iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadInt3 (FILE* iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadInt2 (FILE* iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadFloat (FILE* iFile, float& oFloat);

  int VTK_FreeSurfer_EXPORT ReadShortZ (gzFile iFile, short& oShort);
  int VTK_FreeSurfer_EXPORT ReadIntZ (gzFile iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadInt3Z (gzFile iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadInt2Z (gzFile iFile, int& oInt);
  int VTK_FreeSurfer_EXPORT ReadFloatZ (gzFile iFile, float& oFloat);

  /// For testing purposes
  int VTK_FreeSurfer_EXPORT WriteInt (FILE* iFile, int iInt);
  int VTK_FreeSurfer_EXPORT WriteInt3 (FILE* iFile, int iInt);
  int VTK_FreeSurfer_EXPORT WriteInt2 (FILE* iFile, int iInt);
}

#endif
