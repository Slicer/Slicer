/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtrudePolyData.h,v $
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

// .NAME vtkMimxExtrudePolyData - 
// .SECTION Caveats
// Filter takes in a surface mesh(polydata) no lines, polydata only closed surface mesh
// and extrudes that with the given thickness and generates an unstructured grid.
// the implementation is for quad and this inturn generates Hexahedrons.

#ifndef __vtkMimxExtrudePolyData_h
#define __vtkMimxExtrudePolyData_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkPoints;
class vtkPolyData;

class VTK_MIMXFILTER_EXPORT vtkMimxExtrudePolyData : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxExtrudePolyData *New();
  vtkTypeRevisionMacro(vtkMimxExtrudePolyData,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(ExtrusionLength, double);

  vtkSetMacro(ReverseExtrusionDirection, int);

protected:
  vtkMimxExtrudePolyData();
  ~vtkMimxExtrudePolyData();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int FillInputPortInformation(int, vtkInformation *);

  double ExtrusionLength;
  int ReverseExtrusionDirection;

  private:
  vtkMimxExtrudePolyData(const vtkMimxExtrudePolyData&);  // Not implemented.
  void operator=(const vtkMimxExtrudePolyData&);  // Not implemented.
};

#endif
