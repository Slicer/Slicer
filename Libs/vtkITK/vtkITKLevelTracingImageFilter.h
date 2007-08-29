/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKLevelTracingImageFilter.h $

  Copyright (c) ???
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkITKLevelTracingImageFilter - Wrapper class around itk::LevelTracingImageFilterImageFilter
// .SECTION Description
// itk::LevelTracingImageFilter
// LevelTracingImageFilter traces a level curve (or surface) from a
// seed point.  The pixels on this level curve "boundary" are labeled
// as 1. Does nothing if seed is in uniform area.

// .SECTION Caveats
// This filter is specialized to volumes. If you are interested in 
// contouring other types of data, use the general vtkContourFilter. If you
// want to contour an image (i.e., a volume slice), use vtkMarchingSquares.

#ifndef __vtkITKLevelTracingImageFilter_h
#define __vtkITKLevelTracingImageFilter_h
#include "vtkITK.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkObjectFactory.h"

class VTK_ITK_EXPORT vtkITKLevelTracingImageFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkITKLevelTracingImageFilter *New();
  vtkTypeRevisionMacro(vtkITKLevelTracingImageFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Methods to set/get seeds.  Seeds are specified in IJK (not XYZ).
  vtkSetVector3Macro(Seed, int);
  vtkGetVector3Macro(Seed, int);

  // Method to set the plane (IJ=2, IK=1, JK=0)
  vtkSetMacro(Plane, int);
  vtkGetMacro(Plane, int);

  void SetPlaneToIJ() {this->SetPlane(2);}
  void SetPlaneToIK() {this->SetPlane(1);}
  void SetPlaneToJK() {this->SetPlane(0);}

protected:
  vtkITKLevelTracingImageFilter();
  ~vtkITKLevelTracingImageFilter();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  int Seed[3];
  int Plane;

private:
  vtkITKLevelTracingImageFilter(const vtkITKLevelTracingImageFilter&);  // Not implemented.
  void operator=(const vtkITKLevelTracingImageFilter&);  // Not implemented.
};


#endif
