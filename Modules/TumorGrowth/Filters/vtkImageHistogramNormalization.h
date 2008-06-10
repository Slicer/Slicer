/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageHistogramNormalization.h,v $
  Date:      $Date: 2006/01/31 17:02:27 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkImageHistogramNormalization - 
// .SECTION Description

#ifndef __vtkImageHistogramNormalization_h
#define __vtkImageHistogramNormalization_h

#include "vtkTumorGrowth.h"
#include "vtkImageShiftScale.h"

class VTK_TUMORGROWTH_EXPORT  vtkImageHistogramNormalization : public vtkImageShiftScale
{
public:
  static vtkImageHistogramNormalization *New();
  vtkTypeMacro(vtkImageHistogramNormalization,vtkImageShiftScale);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkImageHistogramNormalization();
  ~vtkImageHistogramNormalization();
  vtkImageHistogramNormalization(const vtkImageHistogramNormalization&);
  void operator=(const vtkImageHistogramNormalization&);
  
  void ExecuteData(vtkDataObject *output);
};

#endif
