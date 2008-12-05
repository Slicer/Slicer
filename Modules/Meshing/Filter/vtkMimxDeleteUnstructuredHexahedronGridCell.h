/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxDeleteUnstructuredHexahedronGridCell.h,v $
Language:  C++
Date:      $Date: 2008/06/23 23:03:56 $
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

// .NAME vtkMimxDeleteUnstructuredHexahedronGridCell - 
// .SECTION Caveats
// Filter takes in an unstructuredgrid as primary input. PointsList input
// should contain 4 points denoting an face in the unstructured grid.
// the output contains unstructured grid with the cell containing the
// face will have a newly extruded neighboring cell sharing an edge with the
// face originally picked.

#ifndef __vtkMimxDeleteUnstructuredHexahedronGridCell_h
#define __vtkMimxDeleteUnstructuredHexahedronGridCell_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkIdList.h"
#include "vtkMimxFilterWin32Header.h"

class vtkIdList;

class VTK_MIMXFILTER_EXPORT vtkMimxDeleteUnstructuredHexahedronGridCell : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxDeleteUnstructuredHexahedronGridCell *New();
  vtkTypeRevisionMacro(vtkMimxDeleteUnstructuredHexahedronGridCell,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetObjectMacro(CellList, vtkIdList);
  void CopyConstraintValues(vtkUnstructuredGrid *input, vtkUnstructuredGrid *output);
protected:
  vtkMimxDeleteUnstructuredHexahedronGridCell();
  ~vtkMimxDeleteUnstructuredHexahedronGridCell();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  vtkIdList *CellList;
  private:
  vtkMimxDeleteUnstructuredHexahedronGridCell(const vtkMimxDeleteUnstructuredHexahedronGridCell&);  // Not implemented.
  void operator=(const vtkMimxDeleteUnstructuredHexahedronGridCell&);  // Not implemented.
};

#endif
