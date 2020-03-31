/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkITKArchetypeImageSeriesVectorReaderSeries_h
#define __vtkITKArchetypeImageSeriesVectorReaderSeries_h

#include "vtkITKArchetypeImageSeriesReader.h"
#include <vtkVersion.h>
namespace itk
{
  class ProcessObject;
  class ProgressEvent;
};

class VTK_ITK_EXPORT vtkITKArchetypeImageSeriesVectorReaderSeries : public vtkITKArchetypeImageSeriesReader
{
public:
  static vtkITKArchetypeImageSeriesVectorReaderSeries *New();
  vtkTypeMacro(vtkITKArchetypeImageSeriesVectorReaderSeries,vtkITKArchetypeImageSeriesReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static void ReadProgressCallback(itk::ProcessObject* obj,
                                   const itk::ProgressEvent&,
                                   void* data);
protected:
  vtkITKArchetypeImageSeriesVectorReaderSeries();
  ~vtkITKArchetypeImageSeriesVectorReaderSeries() override;

  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

private:
  vtkITKArchetypeImageSeriesVectorReaderSeries(const vtkITKArchetypeImageSeriesVectorReaderSeries&) = delete;
  void operator=(const vtkITKArchetypeImageSeriesVectorReaderSeries&) = delete;
};

#endif
