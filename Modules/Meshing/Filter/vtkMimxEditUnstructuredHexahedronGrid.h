/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEditUnstructuredHexahedronGrid.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.8 $

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

// .NAME vtkMimxEditUnstructuredHexahedronGrid- to edit unstructured grid. Delete, add, merge cells with auto renumbering of the nodes and cells.
// .SECTION Description
// vtkMimxEditUnstructuredHexahedronGrid is used to delete, split, add cell in the unstructured hexahedron grid.


#ifndef __vtkMimxEditUnstructuredHexahedronGrid_h
#define __vtkMimxEditUnstructuredHexahedronGrid_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridToUnstructuredGridFilter.h"

class vtkIdList;

#define VTKPROJECTION_SPHERE   0
#define VTKPROJECTION_CYLINDER 1
#define VTKPROJECTION_CLOSESTPOINT 2
#define VTKPROJECTION_PLANAR 3

class  VTK_MIMXFILTER_EXPORT vtkMimxEditUnstructuredHexahedronGrid : public vtkUnstructuredGridToUnstructuredGridFilter
{
public:

  vtkTypeRevisionMacro(vtkMimxEditUnstructuredHexahedronGrid, vtkUnstructuredGridToUnstructuredGridFilter);
//  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkMimxEditUnstructuredHexahedronGrid *New();
  
   // Description:
  // Mark a cell to be removed later.  
        void DeleteCell(vtkIdType i);
        // projection method associated with each cell
        vtkIdList* ProjectionMethodCell;
        void AddCell(vtkIdType* ptlist);
        void SplitCell(vtkIdType* ptlist);
        void MergeNodes(double Tol);
        void Mirror(int axis, int end);
        // input unstructuredgrid
        vtkSetMacro(UGrid,vtkUnstructuredGrid*);
        vtkGetMacro(UGrid,vtkUnstructuredGrid*);

protected:
  vtkMimxEditUnstructuredHexahedronGrid();
  ~vtkMimxEditUnstructuredHexahedronGrid();
  void Execute();
  vtkUnstructuredGrid* UGrid;
private:
  vtkMimxEditUnstructuredHexahedronGrid(const vtkMimxEditUnstructuredHexahedronGrid&);  // Not implemented.
  void operator=(const vtkMimxEditUnstructuredHexahedronGrid&);  // Not implemented.
  int IsInitialized;
  vtkIdType CellToBeDeleted;
};

#endif


