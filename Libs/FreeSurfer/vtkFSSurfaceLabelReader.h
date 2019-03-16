/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSSurfaceLabelReader_h
#define __vtkFSSurfaceLabelReader_h

#include "FreeSurferConfigure.h"
#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtkDataReader.h>

class vtkFloatArray;

/// \brief Read a label surface overlay file (*.label)
/// from Freesurfer tools.
///
/// Reads a surface overlay file file from FreeSurfer and outputs a
/// vtkFloatArray. Use the SetFileName function to specify the file
/// name. The number of values in the array should be equal to the
/// number of vertices/points in the surface, but not all elements may be found
/// in the label file.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceLabelReader : public vtkDataReader
{
public:
  static vtkFSSurfaceLabelReader *New();
  vtkTypeMacro(vtkFSSurfaceLabelReader,vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkFloatArray *GetOutput()
      {return this->Scalars; };
  void SetOutput(vtkFloatArray *output)
      {this->Scalars = output; };

  int ReadLabel();

  ///
  /// Number of vertices on the surface
  vtkGetMacro(NumberOfVertices,int);
  vtkSetMacro(NumberOfVertices,int);

  ///
  /// Number of values read from file, may not be the same as number of
  /// vertices
  vtkGetMacro(NumberOfValues, int);

  ///
  /// Scalar value to use in the array at points in the file
  /// Defaults to 1.0
  vtkGetMacro(LabelOn, float);
  vtkSetMacro(LabelOn, float);

  ///
  /// Scalar value to use in the array at points that don't appear in the file.
  /// defaults to 0.0
  vtkGetMacro(LabelOff, float);
  vtkSetMacro(LabelOff, float);

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
  vtkFSSurfaceLabelReader();
  ~vtkFSSurfaceLabelReader() override;

  vtkFloatArray *Scalars;


  ///
  /// this is the number of vertices in the associated model file,
  /// there may not be as many value in this scalar file as there
  /// are vertices
  int NumberOfVertices;
  ///
  /// number of values read from file
  int NumberOfValues;

  float LabelOff;
  float LabelOn;
/*
  int ReadInt3 (FILE* iFile, int& oInt);
  int ReadInt2 (FILE* iFile, int& oInt);
  int ReadFloat (FILE* iFile, float& oInt);
*/
private:
  vtkFSSurfaceLabelReader(const vtkFSSurfaceLabelReader&) = delete;
  void operator=(const vtkFSSurfaceLabelReader&) = delete;
};

#endif
