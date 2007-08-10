/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEdgeToPlanarStructuredGrid.h,v $
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

// .NAME vtkMimxEdgeToPlanarStructuredGrid - translate point attribute data into a blanking field

// .SECTION Description
// vtkMimxEdgeToPlanarStructuredGrid is a filter that sets the blanking field in a 
// vtkStructuredGrid dataset. The blanking field is set by examining a
// specified point attribute data array (e.g., scalars) and converting
// values in the data array to either a "1" (visible) or "0" (blanked) value
// in the blanking array. The values to be blanked are specified by giving
// a min/max range. All data values in the data array indicated and laying
// within the range specified (inclusive on both ends) are translated to 
// a "off" blanking value.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxEdgeToPlanarStructuredGrid_h
#define __vtkMimxEdgeToPlanarStructuredGrid_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkPolyData;

#include "vtkFilter.h"

class VTK_MIMXFILTER_EXPORT vtkMimxEdgeToPlanarStructuredGrid : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxEdgeToPlanarStructuredGrid *New();
  vtkTypeRevisionMacro(vtkMimxEdgeToPlanarStructuredGrid,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetDimensions(int dim[2]);
  void SetEdge(int EdgeNum, vtkPolyData *PolyData);
 
protected:
  vtkMimxEdgeToPlanarStructuredGrid();
  ~vtkMimxEdgeToPlanarStructuredGrid();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int FillInputPortInformation(int port, vtkInformation *info);

  int Dimensions[2];
  int SetEdge(int EdgeNum);
private:
  vtkMimxEdgeToPlanarStructuredGrid(const vtkMimxEdgeToPlanarStructuredGrid&);  // Not implemented.
  void operator=(const vtkMimxEdgeToPlanarStructuredGrid&);  // Not implemented.
};

#endif
