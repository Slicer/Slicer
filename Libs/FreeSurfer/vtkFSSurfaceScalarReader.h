/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSSurfaceScalarReader_h
#define __vtkFSSurfaceScalarReader_h

#include "FreeSurferConfigure.h"
#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtkDataReader.h>

class vtkFloatArray;

/// \brief Read a surface scalar file from
/// Freesurfer tools.
///
/// Reads a surface scalar file from FreeSurfer and outputs a
/// vtkFloatArray. Use the SetFileName function to specify the file
/// name. The number of values in the array should be equal to the
/// number of vertices/points in the surface.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceScalarReader : public vtkDataReader
{
public:
  static vtkFSSurfaceScalarReader *New();
  vtkTypeMacro(vtkFSSurfaceScalarReader,vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkFloatArray *GetOutput();
  void SetOutput(vtkFloatArray *output);

  /// Read the scalars from a file. Return 1 on success, 0 on failure
  int ReadFSScalars();

  /// file type magic numbers
  /// const int FS_NEW_SCALAR_MAGIC_NUMBER = 16777215;
  enum
    {
      FS_NEW_SCALAR_MAGIC_NUMBER = 16777215,
    };
protected:
  vtkFSSurfaceScalarReader();
  ~vtkFSSurfaceScalarReader() override;

  vtkFloatArray * Scalars;

  int ReadInt3 (FILE* iFile, int& oInt);
  int ReadInt2 (FILE* iFile, int& oInt);
  int ReadFloat (FILE* iFile, float& oInt);

private:
  vtkFSSurfaceScalarReader(const vtkFSSurfaceScalarReader&);  /// Not implemented.
  void operator=(const vtkFSSurfaceScalarReader&);  /// Not implemented.
};

#endif
