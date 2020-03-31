/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkITKArchetypeDiffusionTensorImageReaderFile_h
#define __vtkITKArchetypeDiffusionTensorImageReaderFile_h

#include "vtkITKArchetypeImageSeriesReader.h"
#include <vtkVersion.h>

#include "itkImageFileReader.h"

class VTK_ITK_EXPORT vtkITKArchetypeDiffusionTensorImageReaderFile
  : public vtkITKArchetypeImageSeriesReader
{
 public:
  static vtkITKArchetypeDiffusionTensorImageReaderFile *New();
  vtkTypeMacro(vtkITKArchetypeDiffusionTensorImageReaderFile,vtkITKArchetypeImageSeriesReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

 protected:
  vtkITKArchetypeDiffusionTensorImageReaderFile();
  ~vtkITKArchetypeDiffusionTensorImageReaderFile() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  static void ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&, void* data);
  /// private:

private:
  vtkITKArchetypeDiffusionTensorImageReaderFile(const vtkITKArchetypeDiffusionTensorImageReaderFile&) = delete;
  void operator=(const vtkITKArchetypeDiffusionTensorImageReaderFile&) = delete;
};

#endif
