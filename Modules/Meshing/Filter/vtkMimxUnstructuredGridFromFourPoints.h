/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridFromFourPoints.h,v $
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

// .NAME vtkMimxUnstructuredGridFromFourPoints - 
// .SECTION Caveats
// Filter takes in a list of four point co-ordinates. The output will be an 
// unstructuredgrid containing a hexahedron cell. The other 4 points are added
// by extruding the four initial points in the direction of normal of the average
// plane containing the initial input points.

#ifndef __vtkMimxUnstructuredGridFromFourPoints_h
#define __vtkMimxUnstructuredGridFromFourPoints_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkPoints;

class VTK_MIMXFILTER_EXPORT vtkMimxUnstructuredGridFromFourPoints : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxUnstructuredGridFromFourPoints *New();
  vtkTypeRevisionMacro(vtkMimxUnstructuredGridFromFourPoints,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(Points, vtkPoints*);
  vtkSetMacro(ExtrusionLength, double);

protected:
  vtkMimxUnstructuredGridFromFourPoints();
  ~vtkMimxUnstructuredGridFromFourPoints();
  vtkPoints *Points;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double ExtrusionLength;

  private:
  vtkMimxUnstructuredGridFromFourPoints(const vtkMimxUnstructuredGridFromFourPoints&);  // Not implemented.
  void operator=(const vtkMimxUnstructuredGridFromFourPoints&);  // Not implemented.
};

#endif
