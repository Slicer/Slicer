/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSolidEllipticalInterpolation.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.6 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkMimxSolidEllipticalInterpolation - To compute interior points of 
// of a structured solid mesh
// .SECTION Description
// vtkMimxSolidEllipticalInterpolation is a filter that calculates the interior
// nodes of a solid structured dataset using elliptic grid generation. The
// initial estimation of the position is done using either trilinear 
// interpolation or transfinite interpolation. Gauss-seidel iteration
// is used for the calculation of the unknowns. The error level or the 
// number of iterations can be set. The equation solved is a laplacian equation.
// A poisson equation with control functions on the boundary could also
// be incorperated.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxSolidEllipticalInterpolation_h
#define __vtkMimxSolidEllipticalInterpolation_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxSolidEllipticalInterpolation : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxSolidEllipticalInterpolation *New();
  vtkTypeRevisionMacro(vtkMimxSolidEllipticalInterpolation,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(NumberOfIterations, int);
  vtkSetMacro(ErrorLevel, double);
         
protected:
  vtkMimxSolidEllipticalInterpolation();
  ~vtkMimxSolidEllipticalInterpolation();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int NumberOfIterations;
  double ErrorLevel;
private:
  vtkMimxSolidEllipticalInterpolation(const vtkMimxSolidEllipticalInterpolation&);  // Not implemented.
  void operator=(const vtkMimxSolidEllipticalInterpolation&);  // Not implemented.
};

#endif
