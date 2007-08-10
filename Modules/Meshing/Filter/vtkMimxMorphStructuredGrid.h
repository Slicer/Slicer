/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMorphStructuredGrid.h,v $
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

// .NAME vtkMimxMorphStructuredGrid - Morph a structured grid onto a surface

// .SECTION Description
// vtkMimxMorphStructuredGrid is a filter requires 2 input datasets and an optional input
// dataset. Required data sets are the structured grid that is being morphed which is
// in port number 0, the surface onto which the structured grid is being morphed is input
// through setsource. The optional input would be unstructuredgrid structure which will
// be used to determine the faces that need to be projected. The optional input could be
// set through SetGridStructure. In the absence of optional
// input all the 6 faces of the structured grid are projected. ProjectionType can also be
// set. The current projection techniques implemented are ClosestPoint, Spherical and
// Cylindrical. Default projection technique used is ClosestPoint.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxMorphStructuredGrid_h
#define __vtkMimxMorphStructuredGrid_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkIntArray;
class vtkCollection;
class vtkPolyData;
class vtkUnstructuredGrid;
class vtkCellLocator;

class VTK_MIMXFILTER_EXPORT vtkMimxMorphStructuredGrid : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxMorphStructuredGrid *New();
  vtkTypeRevisionMacro(vtkMimxMorphStructuredGrid,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // to set the surface onto which projection needs to be carried out
  void SetSource(vtkPolyData* );
  // Underlying grid structure which is used to determine the faces that
  // need to be projected
  void SetGridStructure(vtkUnstructuredGrid* );
  vtkSetMacro(ProjectionType, int);
  vtkSetMacro(CellNum, int);
  enum Projection
  {
          ClosestPoint = 0,
          Spherical,
          Cylindrical
  };
protected:
  vtkMimxMorphStructuredGrid();
  ~vtkMimxMorphStructuredGrid();
  
  int ProjectionType;
  int CellNum;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int , vtkInformation *);
  int CheckBoundaryFace(vtkIdList* , vtkPolyData *);
  void ClosestPointProjection(vtkCellLocator *, vtkPoints *);
  int CheckIfEdgeNodesRecalculated(int , vtkIdList*);
  void RecalculateEdge(int , vtkIdList*, vtkPoints*, int[] );
  int CheckInteriorEdge(vtkIdList* IdList, vtkPolyData *BoundaryData);
private:
  vtkMimxMorphStructuredGrid(const vtkMimxMorphStructuredGrid&);  // Not implemented.
  void operator=(const vtkMimxMorphStructuredGrid&);  // Not implemented.
};

#endif
