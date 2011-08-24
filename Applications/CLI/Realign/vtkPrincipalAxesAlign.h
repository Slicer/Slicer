/*=========================================================================

  Program:   Realign Volumes
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __vtk_principal_axes_align_h
#define __vtk_principal_axes_align_h
#include <vtkPolyDataToPolyDataFilter.h>
#include <vtkSetGet.h>
// ---------------------------------------------------------
// Author: Axel Krauth
//
// This class computes the principal axes of the input.
// The direction of the eigenvector for the largest eigenvalue is the XAxis,
// the direction of the eigenvector for the smallest eigenvalue is the ZAxis,
// and the YAxis the the eigenvector for the remaining eigenvalue.
class vtkPrincipalAxesAlign : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkPrincipalAxesAlign * New();

  vtkTypeMacro(vtkPrincipalAxesAlign, vtkPolyDataToPolyDataFilter);

  vtkGetVector3Macro(Center, vtkFloatingPointType);
  vtkGetVector3Macro(XAxis, vtkFloatingPointType);
  vtkGetVector3Macro(YAxis, vtkFloatingPointType);
  vtkGetVector3Macro(ZAxis, vtkFloatingPointType);
  void Execute();

  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkPrincipalAxesAlign();
  ~vtkPrincipalAxesAlign();
private:
  vtkPrincipalAxesAlign(vtkPrincipalAxesAlign &);
  void operator=(const vtkPrincipalAxesAlign &);

  vtkFloatingPointType* Center;
  vtkFloatingPointType* XAxis;
  vtkFloatingPointType* YAxis;
  vtkFloatingPointType* ZAxis;

  // a matrix of the eigenvalue problem
  double* * eigenvalueProblem;
  // for efficiency reasons parts of the eigenvalue problem are computed separately
  double* * eigenvalueProblemDiag;
  double* * eigenvectors;
  double*   eigenvalues;
};

#endif
