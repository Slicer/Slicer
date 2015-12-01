/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderSeries.h $
  Date:      $Date: 2007-01-19 13:21:56 -0500 (Fri, 19 Jan 2007) $
  Version:   $Revision: 2267 $

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
  void PrintSelf(ostream& os, vtkIndent indent);

  static void ReadProgressCallback(itk::ProcessObject* obj,
                                   const itk::ProgressEvent&,
                                   void* data);
protected:
  vtkITKArchetypeImageSeriesVectorReaderSeries();
  ~vtkITKArchetypeImageSeriesVectorReaderSeries();

  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo);
};

#endif
