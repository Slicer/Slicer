/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUGrid.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
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

#ifndef _vtkMimxUGrid_h
#define _vtkMimxUGrid_h

#include "vtkCommon.h"

#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkMimxSTLSource.h"

#include "mimxMatrixTemplate.h"

//  class for storing polydata generated from Tracing
class VTK_MIMXCOMMON_EXPORT vtkMimxUGrid 
{
public:
  static vtkMimxUGrid *New();
  vtkMimxUGrid();
  ~vtkMimxUGrid();
  vtkActor* GetActor();
  vtkDataSetMapper* GetDataSetMapper();
  vtkUnstructuredGrid* UGrid;
  void SetPrevColor(double color[3]);
  void GetPrevColor(double color[3]);
  void CreateBoundingBox(vtkMimxSTLSource* STL);
  vtkActor* Actor;
  void ComputeMeshSeed(double AvElLength);
  int MeshSeedFromAverageElementLength(double);
  void SetUnstructuredGrid(vtkUnstructuredGrid*);
  void SetMeshSeedValues(vtkIntArray*);
  void ChangeMeshSeed(int, int, int);  // to change mesh seed and propagate the change
  // through all the cells whose mesh seed is changed
private:
  vtkDataSetMapper* UGridMapper;
  char *FileName;
  double PrevColor[3];
  vtkIntArray *MeshSeedValues;
  int Links; // to check if links need to be recomputed
  void BuildMeshSeedLinks(); // generates a matrix of values signifying the connection
  // between different cells. this helps in calculation of
  // mesh seeds of cells that are not immediate neighbors.
  void GetCellNeighbors(vtkIdType, vtkIdList*);  // get cell neighbors that have not
  // been traversed
  int CheckIfCellsShareEdgeX(int , int , int, int);
  int CheckIfCellsShareEdgeY(int , int , int, int);
  int CheckIfCellsShareEdgeZ(int , int , int, int);
  void EstimateMeshSeedsBasedOnAverageElementLength(double);
  //  the common edge could be x, y, z on the cell being compared
  int WhichEdgeOfCellBeingCompared(vtkIdType, vtkIdType);
  // mesh seed links
  // size is determined by numberofcells in the unstructuredgrid*3
  // matrix to store cell connectivity information
  CMatrix<int> MeshSeedLinks;
  // matrix for book keeping on cell neighbors visited
  CMatrix<int> MeshSeedCheck;
  // to be used when mesh seeding on average element length is concidered
  void SetHigherNumberedBBoxMeshSeed(int , int );
protected:
};

#endif
