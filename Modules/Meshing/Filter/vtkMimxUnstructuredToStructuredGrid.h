/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredToStructuredGrid.h,v $
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

// .NAME vtkMimxUnstructuredToStructuredGrid - Set elements forming a edge of a structured
// grid. The output is a polydata line.

// .SECTION Description
// vtkMimxUnstructuredToStructuredGrid requires two inputs (both unstructured grids).
// This filter is used to decompose the merged unstructured grids to individual 
// grids (structured). the first input will be the merged unstructured grids.
// the second input will be the underlying bounding box structure from which the 
// first input was generated. The second input's vectors will contain the 
// mesh seeding definition for individual cells in the grid

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxUnstructuredToStructuredGrid_h
#define __vtkMimxUnstructuredToStructuredGrid_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkUnstructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxUnstructuredToStructuredGrid : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxUnstructuredToStructuredGrid *New();
  vtkTypeRevisionMacro(vtkMimxUnstructuredToStructuredGrid,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(StructuredGridNum, int);

  void SetBoundingBox(vtkUnstructuredGrid*);
  protected:
  vtkMimxUnstructuredToStructuredGrid();
  ~vtkMimxUnstructuredToStructuredGrid();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        virtual int FillInputPortInformation(int , vtkInformation *);

        int StructuredGridNum;

  vtkMimxUnstructuredToStructuredGrid(const vtkMimxUnstructuredToStructuredGrid&);  // Not implemented.
  void operator=(const vtkMimxUnstructuredToStructuredGrid&);  // Not implemented.
};

#endif
