/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSSurfaceReader_h
#define __vtkFSSurfaceReader_h

#include "FreeSurferConfigure.h"
#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtkAbstractPolyDataReader.h>

/// Prints debugging info.
#define FS_DEBUG 0

/// This code can calc normals but it doesn't seem to do so very well
/// (bug?) and vtkPolyDataNormals does it anyway.
#define FS_CALC_NORMALS 0

class vtkInformation;
class vtkInformationVector;
class vtkPolyData;

/// \brief Read a surface file from Freesurfer tools
///
/// Reads a surface file from FreeSurfer and output PolyData. Use the
/// SetFileName function to specify the file name.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceReader : public vtkAbstractPolyDataReader
{
public:
  static vtkFSSurfaceReader *New();
  vtkTypeMacro(vtkFSSurfaceReader, vtkAbstractPolyDataReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// old previous versions constants
  enum
  {
      FS_QUAD_FILE_MAGIC_NUMBER = (-1 & 0x00ffffff),
      FS_NEW_QUAD_FILE_MAGIC_NUMBER = (-3 & 0x00ffffff),
      FS_TRIANGLE_FILE_MAGIC_NUMBER = (-2 & 0x00ffffff),
      FS_NUM_VERTS_IN_QUAD_FACE = 4, /// dealing with quads
      FS_NUM_VERTS_IN_TRI_FACE = 3, /// dealing with tris
      FS_MAX_NUM_FACES_PER_VERTEX = 10, /// kinda arbitrary
  };

protected:
  vtkFSSurfaceReader();
  ~vtkFSSurfaceReader() override;

  int RequestData(
    vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *outputVector) override;

private:
  vtkFSSurfaceReader(const vtkFSSurfaceReader&) = delete;
  void operator=(const vtkFSSurfaceReader&) = delete;
};


/// If we're going to try to do normals, we need to keep some
/// connectivity information for the verts and faces. Use these
/// structures.
#if FS_CALC_NORMALS
struct VertexStruct {
  int numFaces;
  int faces[FS_MAX_NUM_FACES_PER_VERTEX];
  int indicesInFace[FS_MAX_NUM_FACES_PER_VERTEX];
  float x, y, z;
  float nx, ny, nz;
};
using Vertex = struct VertexStruct;

struct FaceStruct {
  int vertices[FS_NUM_SIDES_IN_FACE];
};
using Face = struct FaceStruct;
#endif

#endif
