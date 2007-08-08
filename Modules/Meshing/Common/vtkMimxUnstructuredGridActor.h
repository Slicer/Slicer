/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridActor.h,v $
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

// .NAME vtkUnstructuredGridActor - 
// .SECTION Description
// vtkUnstructuredGridActor is the cla

#ifndef __vtkMimxUnstructuredGridActor_h
#define __vtkMimxUnstructuredGridActor_h

#include "vtkCommon.h"

#include "vtkMimxActorBase.h"

#include "mimxMatrixTemplate.h"

class vtkActor;
class vtkDataSetMapper;
class vtkIdList;
class vtkUnstructuredGrid;

class VTK_MIMXCOMMON_EXPORT vtkMimxUnstructuredGridActor : public vtkMimxActorBase
{
public:
  static vtkMimxUnstructuredGridActor *New();
  vtkTypeRevisionMacro(vtkMimxUnstructuredGridActor,vtkMimxActorBase);
  void PrintSelf(ostream& os, vtkIndent indent);
 //vtkDataSet* GetDataSet();
  vtkUnstructuredGrid* GetDataSet();
 // functions related to mesh seeding
 void ComputeMeshSeed(double AvElLength);
 int MeshSeedFromAverageElementLength(double);
 void ChangeMeshSeed(int, int, int);  // to change mesh seed and propagate the change
 // through all the cells whose mesh seed is changed
 void SetDataType(int );
 vtkSetMacro(ElementSetName, char*);
 vtkGetMacro(ElementSetName, char*);
protected:
  vtkMimxUnstructuredGridActor();
  ~vtkMimxUnstructuredGridActor();
  vtkUnstructuredGrid *UnstructuredGrid;
  vtkDataSetMapper *UnstructuredGridMapper;
private:
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
  char *ElementSetName;
  vtkMimxUnstructuredGridActor(const vtkMimxUnstructuredGridActor&);  // Not implemented.
  void operator=(const vtkMimxUnstructuredGridActor&);  // Not implemented.
};

#endif

