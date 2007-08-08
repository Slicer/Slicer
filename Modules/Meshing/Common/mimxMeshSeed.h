/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxMeshSeed.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

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

// To calculate mesh seeds based on initial average element length and
// to modify the mesh seeds of the entire bounding box structure with 
// modification of one single mesh seeds.
// The bounding box structure will be in the form of vtkUnstructuredGrid
// and mesh seed array is set using vtkIntArray.

#ifndef __MeshSeed_h
#define __MeshSeed_h

#include "vtkCommon.h"



#include "MatrixTemplate.h"
#include "vtkSystemIncludes.h"

class vtkUnstructuredGrid;
class vtkIntArray;
class vtkIdList;

class VTK_MIMXCOMMON_EXPORT MeshSeed
{
public:
  MeshSeed();
  ~MeshSeed();
  int MeshSeedFromAverageElementLength(double);
  void SetUnstructuredGrid(vtkUnstructuredGrid*);
  void SetMeshSeedValues(vtkIntArray*);
  void ChangeMeshSeed(int, int, int);  // to change mesh seed and propagate the change
                    // through all the cells whose mesh seed is changed
private:
  vtkUnstructuredGrid *UGrid;
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
};

#endif
