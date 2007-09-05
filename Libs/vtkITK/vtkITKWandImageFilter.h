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
// .NAME vtkITKWandImageFilter - Wand tool implemented using connected threshold
// .SECTION Description
// itk::WandImageFilter
// WandImageFilter selects all pixels connected to a seed point that
// are within a specified intensity difference of the seed point

#ifndef __vtkITKWandImageFilter_h
#define __vtkITKWandImageFilter_h
#include "vtkITK.h"
#include "vtkImageAlgorithm.h"
#include "vtkObjectFactory.h"

class VTK_ITK_EXPORT vtkITKWandImageFilter : public vtkImageAlgorithm
{
public:
  static vtkITKWandImageFilter *New();
  vtkTypeRevisionMacro(vtkITKWandImageFilter, vtkImageAlgorithm);
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

  // Set/Get the intensity difference to connect as a function of the
  // dynamic range
  vtkSetClampMacro(DynamicRangePercentage, double, 0.0, 1.0);
  vtkGetMacro(DynamicRangePercentage, double);
  
protected:
  vtkITKWandImageFilter();
  ~vtkITKWandImageFilter();

  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  int Seed[3];
  int Plane;
  double DynamicRangePercentage;
  
private:
  vtkITKWandImageFilter(const vtkITKWandImageFilter&);  // Not implemented.
  void operator=(const vtkITKWandImageFilter&);  // Not implemented.
};


#endif
