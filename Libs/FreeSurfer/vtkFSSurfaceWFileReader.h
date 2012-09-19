/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceWFileReader.h,v $
  Date:      $Date: 2006/05/26 19:40:15 $
  Version:   $Revision: 1.5 $

=========================================================================auto=*/

#ifndef __vtkFSSurfaceWFileReader_h
#define __vtkFSSurfaceWFileReader_h

#include "FreeSurferConfigure.h"
#include "vtkFreeSurferWin32Header.h"

// VTK includes
#include <vtkDataReader.h>

class vtkFloatArray;

/// \brief Read a surface w file (*.w) file
/// from Freesurfer tools.
///
/// Reads a surface w file file from FreeSurfer and outputs a
/// vtkFloatArray. Use the SetFileName function to specify the file
/// name. The number of values in the array should be equal to the
/// number of vertices/points in the surface.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceWFileReader : public vtkDataReader
{
public:
  static vtkFSSurfaceWFileReader *New();
  vtkTypeMacro(vtkFSSurfaceWFileReader,vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkFloatArray *GetOutput();
  void SetOutput(vtkFloatArray *output);

  int ReadWFile();

  vtkGetMacro(NumberOfVertices,int);
  vtkSetMacro(NumberOfVertices,int);

  enum
  {
    /// error codes
    FS_ERROR_W_NONE = 0,
    FS_ERROR_W_OUTPUT_NULL = 1,
    FS_ERROR_W_NO_FILENAME = 2,
    FS_ERROR_W_OPEN = 3,
    FS_ERROR_W_NUM_VALUES = 4,
    FS_ERROR_W_ALLOC = 5,
    FS_ERROR_W_EOF = 6,
    /// file type magic numbers
    FS_NEW_SCALAR_MAGIC_NUMBER = 16777215,
  };

protected:
  vtkFSSurfaceWFileReader();
  ~vtkFSSurfaceWFileReader();

  vtkFloatArray * Scalars;

  /// this is the number of vertices in the associated model file,
  /// there may not be as many value in this scalar file as there
  /// are vertices
  int NumberOfVertices;

  int ReadInt3 (FILE* iFile, int& oInt);
  int ReadInt2 (FILE* iFile, int& oInt);
  int ReadFloat (FILE* iFile, float& oInt);

private:
  vtkFSSurfaceWFileReader(const vtkFSSurfaceWFileReader&);  /// Not implemented.
  void operator=(const vtkFSSurfaceWFileReader&);  /// Not implemented.
};

#endif
