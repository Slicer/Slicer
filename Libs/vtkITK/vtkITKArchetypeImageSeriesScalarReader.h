/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKArchetypeImageSeriesScalarReader_h
#define __vtkITKArchetypeImageSeriesScalarReader_h

#include "vtkITKArchetypeImageSeriesReader.h"

#include "itkImageFileReader.h"

class VTK_ITK_EXPORT vtkITKArchetypeImageSeriesScalarReader : public vtkITKArchetypeImageSeriesReader
{
 public:
  
  static vtkITKArchetypeImageSeriesScalarReader *New();
  vtkTypeMacro(vtkITKArchetypeImageSeriesScalarReader,vtkITKArchetypeImageSeriesReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

 protected:
  vtkITKArchetypeImageSeriesScalarReader();
  ~vtkITKArchetypeImageSeriesScalarReader() override;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
  static void ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&, void* data);
  /// private:

private:
  vtkITKArchetypeImageSeriesScalarReader(const vtkITKArchetypeImageSeriesScalarReader&) = delete;
  void operator=(const vtkITKArchetypeImageSeriesScalarReader&) = delete;
};

#endif
