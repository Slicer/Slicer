/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkFSSurfaceAnnotationReader_h
#define __vtkFSSurfaceAnnotationReader_h

// FreeSurfer includes
#include "FreeSurferConfigure.h"
#include "vtkFreeSurferExport.h"

// VTK includes
#include <vtkDataReader.h>

class vtkIntArray;
class vtkLookupTable;

/// \brief Read a surface annotation and
/// color table file from Freesurfer tools.
///
///
/// Reads a surface annotation file from FreeSurfer and outputs a
/// vtkIntArray and has an access function to get the vtkLookupTable
/// with the color values. Use the SetFileName function to specify the
/// file name. The number of values in the array should be equal to the
/// number of vertices/points in the surface.
class VTK_FreeSurfer_EXPORT vtkFSSurfaceAnnotationReader : public vtkDataReader
{
public:
  static vtkFSSurfaceAnnotationReader *New();
  vtkTypeMacro(vtkFSSurfaceAnnotationReader,vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkIntArray *GetOutput();
  void SetOutput(vtkIntArray *output);

  vtkLookupTable *GetColorTableOutput();
  void SetColorTableOutput(vtkLookupTable* colors);

  char* GetColorTableNames();

  int ReadFSAnnotation();

  /// write out the annotation file, using an internal color table
  int WriteFSAnnotation();

  vtkGetMacro(NumColorTableEntries, int);
  vtkSetStringMacro(ColorTableFileName);

  vtkGetMacro(UseExternalColorTableFile,int);
  vtkSetMacro(UseExternalColorTableFile,int);
  vtkBooleanMacro(UseExternalColorTableFile,int);

  ///
  /// previously defined as constants
  enum
  {
    /// tag
    FS_COLOR_TABLE_TAG = 1,

    FS_COLOR_TABLE_NAME_LENGTH = 1024,
    FS_COLOR_TABLE_ENTRY_NAME_LENGTH = 1024,

    FS_ERROR_LOADING_COLOR_TABLE = 1,
    FS_ERROR_LOADING_ANNOTATION = 2,
    FS_ERROR_PARSING_COLOR_TABLE = 3,
    FS_ERROR_PARSING_ANNOTATION = 4,
    FS_WARNING_UNASSIGNED_LABELS = 5,
    FS_NO_COLOR_TABLE = 6,
  };
protected:
  vtkFSSurfaceAnnotationReader();
  ~vtkFSSurfaceAnnotationReader() override;

  vtkIntArray    *Labels;
  vtkLookupTable *Colors;
  char           *NamesList;
  int            NumColorTableEntries;

  int UseExternalColorTableFile;
  char *ColorTableFileName;

  /// Read color table information from a source, allocate the arrays
  /// to hold rgb and name values, and return pointers to the
  /// arrays. The caller is responsible for disposing of the memory.
  int ReadEmbeddedColorTable (FILE* annotFile, int* numEntries,
                  int*** rgbValues, char*** names);
  int ReadExternalColorTable (char* fileName, int* numEntries,
                  int*** rgbValues, char*** names);


private:
  vtkFSSurfaceAnnotationReader(const vtkFSSurfaceAnnotationReader&) = delete;
  void operator=(const vtkFSSurfaceAnnotationReader&) = delete;
};

#endif
