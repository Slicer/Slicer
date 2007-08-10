/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxToStructuredGrids.h,v $
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

// .NAME vtkMimxBoundingBoxToStructuredGrids - Generate a list of structured grids from
// an unstructured grid. each cell in the unstructured grid will correspond to the 
// a structured grid.

// .SECTION Description
// vtkMimxBoundingBoxToStructuredGrids is a filter that takes an unstructured grid as an 
// input and generate structured grids as output. the dimensions of the 
// structured grids is set through an IntArray of SetCellData method.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxBoundingBoxToStructuredGrids_h
#define __vtkMimxBoundingBoxToStructuredGrids_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkIntArray;
class vtkCollection;

class VTK_MIMXFILTER_EXPORT vtkMimxBoundingBoxToStructuredGrids : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxBoundingBoxToStructuredGrids *New();
  vtkTypeRevisionMacro(vtkMimxBoundingBoxToStructuredGrids,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  vtkStructuredGrid *GetStructuredGrid(vtkIdType );
 
protected:
  vtkMimxBoundingBoxToStructuredGrids();
  ~vtkMimxBoundingBoxToStructuredGrids();
  
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int , vtkInformation *);
  vtkCollection *StructuredGrid;
private:
  vtkMimxBoundingBoxToStructuredGrids(const vtkMimxBoundingBoxToStructuredGrids&);  // Not implemented.
  void operator=(const vtkMimxBoundingBoxToStructuredGrids&);  // Not implemented.
};

#endif
