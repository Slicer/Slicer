/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageSequenceWriter_h
#define __vtkITKImageSequenceWriter_h

#include "vtkImageAlgorithm.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"

#include "vtkITK.h"
#include "itkImageIOBase.h"

class vtkStringArray;

class VTK_ITK_EXPORT vtkITKImageSequenceWriter : public vtkImageAlgorithm
{
public:
  static vtkITKImageSequenceWriter *New();
  vtkTypeMacro(vtkITKImageSequenceWriter,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
    {
    VoxelVectorTypeUndefined,
    VoxelVectorTypeSpatial,
    VoxelVectorTypeColorRGB,
    VoxelVectorTypeColorRGBA,
    VoxelVectorType_Last // must be last
    };

  ///
  /// Specify file name for the image file. You should specify either
  /// a FileName or a FilePrefix. Use FilePrefix if the data is stored
  /// in multiple files.
  void SetFileName(const char *);

  char* GetFileName()
    {
    return FileName;
    }

  ///
  /// Use compression if possible
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);
  vtkBooleanMacro(UseCompression, int);

  ///
  /// Set/Get the ImageIO class name.
  vtkGetStringMacro(ImageIOClassName);
  vtkSetStringMacro(ImageIOClassName);

  ///
  /// Set/Get the number of data frames in the sequence to write.
  vtkGetMacro(NumberOfSequenceFrames, int);
  vtkSetMacro(NumberOfSequenceFrames, int);

  ///
  /// The main interface which triggers the writer to start.
  void Write();

  /// Set orientation matrix
  void SetRasToIJKMatrix(vtkMatrix4x4* mat)
    {
    RasToIJKMatrix = mat;
    }

  /// Set orientation matrix
  void SetMeasurementFrameMatrix(vtkMatrix4x4* mat)
    {
    MeasurementFrameMatrix = mat;
    }

  /// Defines how to interpret voxel components
  vtkSetMacro(VoxelVectorType, int);
  vtkGetMacro(VoxelVectorType, int);

protected:
  vtkITKImageSequenceWriter();
  ~vtkITKImageSequenceWriter() override;

  char *FileName;
  vtkMatrix4x4* RasToIJKMatrix;
  vtkMatrix4x4* MeasurementFrameMatrix;
  int UseCompression;
  char* ImageIOClassName;
  int NumberOfSequenceFrames;
  int VoxelVectorType;

private:
  vtkITKImageSequenceWriter(const vtkITKImageSequenceWriter&) = delete;
  void operator=(const vtkITKImageSequenceWriter&) = delete;
};

#endif
