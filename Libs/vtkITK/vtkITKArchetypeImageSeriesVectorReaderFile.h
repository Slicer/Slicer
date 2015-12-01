/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderFile.h $
  Date:      $Date: 2007-01-19 13:21:56 -0500 (Fri, 19 Jan 2007) $
  Version:   $Revision: 2267 $

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
  void PrintSelf(ostream& os, vtkIndent indent);

 protected:
  vtkITKArchetypeImageSeriesVectorReaderFile();
  ~vtkITKArchetypeImageSeriesVectorReaderFile();

  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo);
  static void ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&, void* data);
  /// private:
};

#endif
