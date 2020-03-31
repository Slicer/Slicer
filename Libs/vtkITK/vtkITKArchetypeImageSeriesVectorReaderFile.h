/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkITKArchetypeImageSeriesVectorReaderFile_h
#define __vtkITKArchetypeImageSeriesVectorReaderFile_h

#include "vtkITKArchetypeImageSeriesReader.h"

#include "itkImageFileReader.h"
#include <vtkVersion.h>

class VTK_ITK_EXPORT vtkITKArchetypeImageSeriesVectorReaderFile : public vtkITKArchetypeImageSeriesReader
{
 public:
  static vtkITKArchetypeImageSeriesVectorReaderFile *New();
  vtkTypeMacro(vtkITKArchetypeImageSeriesVectorReaderFile,vtkITKArchetypeImageSeriesReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

 protected:
  vtkITKArchetypeImageSeriesVectorReaderFile();
  ~vtkITKArchetypeImageSeriesVectorReaderFile() override;

  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;
  static void ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&, void* data);

private:
  vtkITKArchetypeImageSeriesVectorReaderFile(const vtkITKArchetypeImageSeriesVectorReaderFile&) = delete;
  void operator=(const vtkITKArchetypeImageSeriesVectorReaderFile&) = delete;
};

#endif
