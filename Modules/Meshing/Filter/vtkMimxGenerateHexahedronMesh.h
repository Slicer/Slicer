/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGenerateHexahedronMesh.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.15 $

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

// .NAME vtkMimxGenerateHexahedronMesh- 
// .SECTION Description
// vtkMimxGenerateHexahedronMesh takes in an unstructured hexahedron grid and generates 
// a finer unstructured hexahedron grid based on specified number of divisions of the
// original coarse grid.


#ifndef __vtkMimxGenerateHexahedronMesh_h
#define __vtkMimxGenerateHexahedronMesh_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridToUnstructuredGridFilter.h"
#include "mimxMatrixTemplate.h"
#include "mimxLinkedList.h"

class vtkActor;
class vtkCellLocator;
class vtkCollection;
class vtkDataSetMapper;
class vtkIdList;
class vtkPolyData;
class vtkCell;
class vtkPoints;
class vtkQuad;
class vtkIntArray;

struct Node
{
        int CellNum;        // to st
        int EdgeNum;
        Node* Parent;
        Node* PeerPrev;
        Node* PeerNext;
        Node* Child;
};


class  VTK_MIMXFILTER_EXPORT vtkMimxGenerateHexahedronMesh : public vtkUnstructuredGridToUnstructuredGridFilter
{
public:

  vtkTypeRevisionMacro(vtkMimxGenerateHexahedronMesh, vtkUnstructuredGridToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkMimxGenerateHexahedronMesh *New();
  // list of structured grids derived from the input hexahedron grid
  vtkCollection* StructuredGridCollection;
  vtkCollection* StructuredGridMapperCollection;
  vtkCollection* StructuredGridActorCollection;
  // Function to generate initial rectilinear mesh
  void GenerateRectilinearHexahedronMesh();
  void MorphRectilinearHexahedronMesh(vtkPolyData* polydata);
  void CreateOutput();
  vtkUnstructuredGrid* UGrid;
  vtkActor* Actor;
  void LaplacianSmoothing(vtkPolyData* polydata);
  // Functions to store and set mesh seeds
  // read mesh seed values from a file
  void SetMeshSeedsFileName(const char* filename);
  void SetMeshSeed(int cellnum, int meshseedX, int meshseedY, int meshseedZ);
  void GetMeshSeed(int cellnum, int meshseedX, int meshseedY, int meshseedZ);
  void CheckMeshSeedValidity();
  void BuildMeshSeedLinks();
  int CheckIfCellsShareEdgeX(int cellnum1, int cellnum2, int parcell);
  int CheckIfCellsShareEdgeY(int cellnum1, int cellnum2, int parcell);
  int CheckIfCellsShareEdgeZ(int cellnum1, int cellnum2, int parcell);
  // for automated mesh seeding. mesh seeding starts with bounding box 1
  // and lower numbered box mesh seeds take precedence over upper numbered
  // mesh seed.
  void EstimateMeshSeedsBasedOnAverageElementLength(double length);
  // set mesh seed values based on mesh seed of the lower numbered element
  // axis, 1 for X , 2 for Y and 3 for Z
  void SetHigherNumberedBBoxMeshSeed(int boxnum, int axis);

  void ReCalculateInteriorNodes();

  // lists storing 3 mesh seeds
  vtkIntArray *MeshSeedX;
  vtkIntArray *MeshSeedY;
  vtkIntArray *MeshSeedZ;

  // mesh seed links
  // size is determined by numberofcells in the bounding box*3
  CMatrix<int> MeshSeedLinks;
  // matrix for book keeping on cell neighbors visited
  CMatrix<int> MeshSeedCheck;
  void ProjectVertices(vtkPolyData *polydata);
protected:
  vtkMimxGenerateHexahedronMesh();
  ~vtkMimxGenerateHexahedronMesh();
  int CheckBoundaryFace(vtkIdList*, vtkPolyData*);
  int CheckBoundaryEdge(vtkIdList*, vtkPolyData*);
  int CheckNumberOfCellsSharingEdge(vtkCell* edge);
  vtkDataSetMapper* Mapper;

  void GetFace(int CellNum, int FaceNum, vtkPoints* PointList);

  void GetFace0(int CellNum, vtkPoints* PointList);
  void GetFace1(int CellNum, vtkPoints* PointList);
  void GetFace2(int CellNum, vtkPoints* PointList);
  void GetFace3(int CellNum, vtkPoints* PointList);
  void GetFace4(int CellNum, vtkPoints* PointList);
  void GetFace5(int CellNum, vtkPoints* PointList);

  void SetFace(int CellNum, int FaceNum, vtkPoints* PointList);

  void SetFace0(int CellNum, vtkPoints* PointList);
  void SetFace1(int CellNum, vtkPoints* PointList);
  void SetFace2(int CellNum, vtkPoints* PointList);
  void SetFace3(int CellNum, vtkPoints* PointList);
  void SetFace4(int CellNum, vtkPoints* PointList);
  void SetFace5(int CellNum, vtkPoints* PointList);

  void GetEdge(int CellNum, int EdgeNum, vtkPoints* PointList);

  void GetEdge0(int CellNum, vtkPoints* PointList);
  void GetEdge1(int CellNum, vtkPoints* PointList);
  void GetEdge2(int CellNum, vtkPoints* PointList);
  void GetEdge3(int CellNum, vtkPoints* PointList);
  void GetEdge4(int CellNum, vtkPoints* PointList);
  void GetEdge5(int CellNum, vtkPoints* PointList);
  void GetEdge6(int CellNum, vtkPoints* PointList);
  void GetEdge7(int CellNum, vtkPoints* PointList);
  void GetEdge8(int CellNum, vtkPoints* PointList);
  void GetEdge9(int CellNum, vtkPoints* PointList);
  void GetEdge10(int CellNum, vtkPoints* PointList);
  void GetEdge11(int CellNum, vtkPoints* PointList);

  void SetEdge(int CellNum, int EdgeNum, vtkPoints* PointList);

  void SetEdge0(int CellNum, vtkPoints* PointList);
  void SetEdge1(int CellNum, vtkPoints* PointList);
  void SetEdge2(int CellNum, vtkPoints* PointList);
  void SetEdge3(int CellNum, vtkPoints* PointList);
  void SetEdge4(int CellNum, vtkPoints* PointList);
  void SetEdge5(int CellNum, vtkPoints* PointList);
  void SetEdge6(int CellNum, vtkPoints* PointList);
  void SetEdge7(int CellNum, vtkPoints* PointList);
  void SetEdge8(int CellNum, vtkPoints* PointList);
  void SetEdge9(int CellNum, vtkPoints* PointList);
  void SetEdge10(int CellNum, vtkPoints* PointList);
  void SetEdge11(int CellNum, vtkPoints* PointList);

  void RecalculateEdge(int CellNum, int EdgeNum, vtkIdList *IdList);

  void ClosestPointProjection(vtkCellLocator* Locator, vtkPoints *Points);
  
  void MergeStructuredGridsToUnstructuredGrid(vtkPolyData*);
  bool CheckWhichSeedDiretion(vtkCell *cell, vtkIdType pt1,
          vtkIdType pt2, int i, int j);
  // function to check if edge X is shared. cell and cell_comp 
  // are the two cells being compared. value returned is 1 for 
  // X , 2 for Y and 3 for Z, if they share no edge 0 is returned
  int CheckIfEdgeSharedX(vtkCell *cell, vtkCell *cell_comp);
  int CheckIfEdgeSharedY(vtkCell *cell, vtkCell *cell_comp);
  int CheckIfEdgeSharedZ(vtkCell *cell, vtkCell *cell_comp);
  int WhichEdgeOfCellBeingCompared(vtkIdType vertex1, vtkIdType vertex2);
  int CheckIfEdgeNodesRecalculated(int EdgeNum, vtkIdList* IdList);
  // function to return cell neighbors including neighbors sharing 
  // a single vertex
  void GetCellNeighbors(vtkIdType cellnum, vtkIdList *cellids);
  int Links;

  // get points from an unstructured grid corresponding to a bounding
  // box (cell num)in the bounding box structure. vtkIdList contains 
  // the node numbering in the unstructured grid.
  void GetCellPoints(int, vtkPoints*, vtkIdList*);
  void SetCellPoints(vtkPoints*, vtkIdList*);


  void GetFaceUGrid(int, int, vtkPoints*, vtkPoints*);

  void GetFace0UGrid(int, vtkPoints*, vtkPoints*);
  void GetFace1UGrid(int, vtkPoints*, vtkPoints*);
  void GetFace2UGrid(int, vtkPoints*, vtkPoints*);
  void GetFace3UGrid(int, vtkPoints*, vtkPoints*);
  void GetFace4UGrid(int, vtkPoints*, vtkPoints*);
  void GetFace5UGrid(int, vtkPoints*, vtkPoints*);

  void SetFaceUGrid(int , int, vtkPoints*, vtkPoints*);

  void SetFace0UGrid(int, vtkPoints*, vtkPoints*);
  void SetFace1UGrid(int, vtkPoints*, vtkPoints*);
  void SetFace2UGrid(int, vtkPoints*, vtkPoints*);
  void SetFace3UGrid(int, vtkPoints*, vtkPoints*);
  void SetFace4UGrid(int, vtkPoints*, vtkPoints*);
  void SetFace5UGrid(int, vtkPoints*, vtkPoints*);

private:
  vtkMimxGenerateHexahedronMesh(const vtkMimxGenerateHexahedronMesh&);  // Not implemented.
  void operator=(const vtkMimxGenerateHexahedronMesh&);  // Not implemented.
};

#endif


