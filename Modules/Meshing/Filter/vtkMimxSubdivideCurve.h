/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSubdivideCurve.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.9 $

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

// .NAME vtkMimxSubdivideCurve - resample a given curve based on number of 
// number of divisions / user defined element length.
// .SECTION Description
// vtkMimxSubdivideCurve is a filter that takes in a set of ordered points which
// define a curve and output a curve (polydata). The curve is resampled based
// on number of user-defined subdivisions or segment length 
// .SECTION Caveats
// This filter is specialized to curves. Only Polydata in the form of ordered points
// should be input.

#ifndef __vtkMimxSubdivideCurve_h
#define __vtkMimxSubdivideCurve_h

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxSubdivideCurve : public vtkPolyDataAlgorithm
{
public:

  static vtkMimxSubdivideCurve *New();
  vtkTypeRevisionMacro(vtkMimxSubdivideCurve,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(NumberOfDivisions,int);
  vtkGetMacro(NumberOfDivisions,int);
  double MeasureCurveLength(vtkPoints *Points);
  int SubDivideCurveBasedOnNumberOfDivisions(vtkPoints *Pts, vtkPoints *NewPts);
  void GenerateCurveConnectivity(vtkPolyData* Polydata);
protected:
  vtkMimxSubdivideCurve();
  ~vtkMimxSubdivideCurve();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int NumberOfDivisions;
  private:
  vtkMimxSubdivideCurve(const vtkMimxSubdivideCurve&);  // Not implemented.
  void operator=(const vtkMimxSubdivideCurve&);  // Not implemented.
};

#endif
