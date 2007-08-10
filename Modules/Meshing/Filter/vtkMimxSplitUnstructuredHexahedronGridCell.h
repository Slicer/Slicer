/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSplitUnstructuredHexahedronGridCell.h,v $
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

// .NAME vtkMimxSplitUnstructuredHexahedronGridCell - 
// .SECTION Caveats
// Filter takes in an unstructuredgrid as primary input. PointsList input
// should contain two points denoting an edge in the unstructured grid.
// the output contains unstructured grid with the cell containing the edge
// split in half.

#ifndef __vtkMimxSplitUnstructuredHexahedronGridCell_h
#define __vtkMimxSplitUnstructuredHexahedronGridCell_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"
#include "mimxMatrixTemplate.h"
#include "mimxLinkedList.h"

class vtkIdList;


class VTK_MIMXFILTER_EXPORT vtkMimxSplitUnstructuredHexahedronGridCell : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxSplitUnstructuredHexahedronGridCell *New();
  vtkTypeRevisionMacro(vtkMimxSplitUnstructuredHexahedronGridCell,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(IdList, vtkIdList*);
protected:
  vtkMimxSplitUnstructuredHexahedronGridCell();
  ~vtkMimxSplitUnstructuredHexahedronGridCell();
  vtkIdList *IdList;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void BuildMeshSeedLinks(int CellNum, int EdgeNum);
  void GetCellNeighbors(vtkIdType, vtkIdList*);        // get cell neighbors that have not
  // been traversed
  int CheckIfCellsShareEdgeX(int , int , int);
  int CheckIfCellsShareEdgeY(int , int , int);
  int CheckIfCellsShareEdgeZ(int , int , int);
  void EstimateMeshSeedsBasedOnAverageElementLength(double);
  //        the common edge could be x, y, z on the cell being compared
  int WhichEdgeOfCellBeingCompared(vtkIdType, vtkIdType);
  // mesh seed links
  // size is determined by numberofcells in the unstructuredgrid*3
  // matrix to store cell connectivity information
  CMatrix<int> MeshSeedLinks;
  // matrix for book keeping on cell neighbors visited
  CMatrix<int> MeshSeedCheck;

  private:
  vtkMimxSplitUnstructuredHexahedronGridCell(const vtkMimxSplitUnstructuredHexahedronGridCell&);  // Not implemented.
  void operator=(const vtkMimxSplitUnstructuredHexahedronGridCell&);  // Not implemented.
};

#endif
