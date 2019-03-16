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
#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>
#include <vtkVersion.h>
// ---------------------------------------------------------
// Author: Axel Krauth
//
// This class computes the principal axes of the input.
// The direction of the eigenvector for the largest eigenvalue is the XAxis,
// the direction of the eigenvector for the smallest eigenvalue is the ZAxis,
// and the YAxis the the eigenvector for the remaining eigenvalue.
class vtkPrincipalAxesAlign : public vtkPolyDataAlgorithm
{
public:
  static vtkPrincipalAxesAlign * New();

  vtkTypeMacro(vtkPrincipalAxesAlign, vtkPolyDataAlgorithm);

  vtkGetVector3Macro(Center, double);
  vtkGetVector3Macro(XAxis, double);
  vtkGetVector3Macro(YAxis, double);
  vtkGetVector3Macro(ZAxis, double);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkPrincipalAxesAlign();
  ~vtkPrincipalAxesAlign() override;
private:
  vtkPrincipalAxesAlign(vtkPrincipalAxesAlign &) = delete;
  void operator=(const vtkPrincipalAxesAlign &) = delete;

  double* Center;
  double* XAxis;
  double* YAxis;
  double* ZAxis;

  // a matrix of the eigenvalue problem
  double* * eigenvalueProblem;
  // for efficiency reasons parts of the eigenvalue problem are computed separately
  double* * eigenvalueProblemDiag;
  double* * eigenvectors;
  double*   eigenvalues;
};

#endif
