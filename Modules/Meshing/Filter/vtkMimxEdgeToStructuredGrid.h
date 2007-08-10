/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEdgeToStructuredGrid.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.7 $

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

// .NAME vtkMimxEdgeToStructuredGrid - translate point attribute data into a blanking field

// .SECTION Description
// vtkEdgeToStructuredGrid is a filter that sets the blanking field in a 
// vtkStructuredGrid dataset. The blanking field is set by examining a
// specified point attribute data array (e.g., scalars) and converting
// values in the data array to either a "1" (visible) or "0" (blanked) value
// in the blanking array. The values to be blanked are specified by giving
// a min/max range. All data values in the data array indicated and laying
// within the range specified (inclusive on both ends) are translated to 
// a "off" blanking value.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxEdgeToStructuredGrid_h
#define __vtkMimxEdgeToStructuredGrid_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkPolyData;

#define VTKIS_PLANE_NONE   0
#define VTKIS_PLANE_IJ   1
#define VTKIS_PLANE_JK   2
#define VTKIS_PLANE_KI   3



class VTK_MIMXFILTER_EXPORT vtkMimxEdgeToStructuredGrid : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxEdgeToStructuredGrid *New();
  vtkTypeRevisionMacro(vtkMimxEdgeToStructuredGrid,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetEdge(int EdgeNum, vtkPolyData* Polydata);
  void SetDimensions(int dim[3]);
  vtkSetMacro(Plane,int);
protected:
  vtkMimxEdgeToStructuredGrid();
  ~vtkMimxEdgeToStructuredGrid();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int SetEdge(int EdgeNum);
  int FillInputPortInformation(int port, vtkInformation *info);

  vtkPoints* GetFace(int FaceNum);
  int SetFace(int FaceNum, vtkPoints* PointsList);
  int Plane;
  int Dimensions[3];
private:
  vtkMimxEdgeToStructuredGrid(const vtkMimxEdgeToStructuredGrid&);  // Not implemented.
  void operator=(const vtkMimxEdgeToStructuredGrid&);  // Not implemented.
};

#endif
