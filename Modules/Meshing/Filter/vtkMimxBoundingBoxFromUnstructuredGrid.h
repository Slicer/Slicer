/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxFromUnstructuredGrid.h,v $
Language:  C++
Date:      $Date: 2007/10/18 21:38:00 $
Version:   $Revision: 1.2 $

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

// .NAME vtkMimxBoundingBoxFromUnstructuredGrid - Extract the Building block structure
// from the final unstructured mesh.

// .SECTION Description
// vtkMimxBoundingBoxFromUnstructuredGrid requires two inputs (both unstructured grids).
// This filter is used to extract the building block structure from a given unstructured mesh
// the input is a bounding box and the unstructured grid. bounding box structure is used for
// the mesh seeds.

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxBoundingBoxFromUnstructuredGrid_h
#define __vtkMimxBoundingBoxFromUnstructuredGrid_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

class vtkUnstructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxBoundingBoxFromUnstructuredGrid : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxBoundingBoxFromUnstructuredGrid *New();
  vtkTypeRevisionMacro(vtkMimxBoundingBoxFromUnstructuredGrid,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetBoundingBox(vtkUnstructuredGrid*);

  protected:
  vtkMimxBoundingBoxFromUnstructuredGrid();
  ~vtkMimxBoundingBoxFromUnstructuredGrid();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        void GetCellPoint(vtkIdType CellNum, vtkIdType PointNum, 
                vtkUnstructuredGrid *BoundingBox, vtkUnstructuredGrid *FEMesh, double x[]);

  vtkMimxBoundingBoxFromUnstructuredGrid(const vtkMimxBoundingBoxFromUnstructuredGrid&);  // Not implemented.
  void operator=(const vtkMimxBoundingBoxFromUnstructuredGrid&);  // Not implemented.
};

#endif
