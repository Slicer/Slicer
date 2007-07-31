/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/
// .NAME vtkImageErode -  Performs erosion
// .SECTION Description
// Erodes pixels of specified Foreground value by setting them 
// to the Background value. Variable 3D connectivity (4- or 8-neighbor).

#ifndef __vtkImageErode_h
#define __vtkImageErode_h

#include "vtkImageData.h"
#include "vtkImageNeighborhoodFilter.h"
#include "vtkSlicerBaseLogic.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageErode : public vtkImageNeighborhoodFilter
{
public:
  static vtkImageErode *New();
  vtkTypeMacro(vtkImageErode,vtkImageNeighborhoodFilter);
    
  // Description: 
  // Background and foreground pixel values in the image.
  // Usually 0 and some label value, respectively.
  vtkSetMacro(Background, float);
  vtkGetMacro(Background, float);
  vtkSetMacro(Foreground, float);
  vtkGetMacro(Foreground, float);

protected:
  vtkImageErode();
  ~vtkImageErode();

  float Background;
  float Foreground;

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
    int extent[6], int id);
};

#endif

