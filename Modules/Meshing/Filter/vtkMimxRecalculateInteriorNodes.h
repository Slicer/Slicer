/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRecalculateInteriorNodes.h,v $
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

// .NAME vtkMimxRecalculateInteriorNodes - Set elements forming a edge of a structured
// grid. The output is a polydata line.

// .SECTION Description
// vtkMimxRecalculateInteriorNodes requires two inputs (both unstructured grids).
// This filter is used to recalculate the position of interior nodes using 
// various interpolation algorithms. Currently elliptic interpolation has been
// used.

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxRecalculateInteriorNodes_h
#define __vtkMimxRecalculateInteriorNodes_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkIdList;
class vtkPoints;
class vtkPolyData;
class vtkUnstructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxRecalculateInteriorNodes : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxRecalculateInteriorNodes *New();
  vtkTypeRevisionMacro(vtkMimxRecalculateInteriorNodes,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(NumberOfIterations, int);

  void SetBoundingBox(vtkUnstructuredGrid*);

  protected:
  vtkMimxRecalculateInteriorNodes();
  ~vtkMimxRecalculateInteriorNodes();

  // get points from an unstructured grid corresponding to a bounding
  // box (cell num)in the bounding box structure. vtkIdList contains 
  // the node numbering in the unstructured grid.
  void GetCellPoints(int, vtkUnstructuredGrid*, vtkUnstructuredGrid *,
          vtkPoints*, vtkIdList*);

  void SetCellPoints(vtkPoints*, vtkIdList*, vtkUnstructuredGrid*);

  void GetFaceUGrid(int, int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);

  void GetFace0UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void GetFace1UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void GetFace2UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void GetFace3UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void GetFace4UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void GetFace5UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);

  void SetFaceUGrid(int , int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);

  void SetFace0UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void SetFace1UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void SetFace2UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void SetFace3UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void SetFace4UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);
  void SetFace5UGrid(int, vtkUnstructuredGrid*, vtkPoints*, vtkPoints*);

  int CheckBoundaryFace(vtkIdList*, vtkPolyData*);
  int CheckBoundaryEdge(vtkIdList*, vtkPolyData*);
  void RecalculateEdge(int CellNum, int EdgeNum, vtkIdList *IdList, 
          vtkUnstructuredGrid *BBox, vtkPoints *CellPoints);

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

        int NumberOfIterations;
        void GetEdge(int CellNum, int EdgeNum, vtkUnstructuredGrid* BBox,  
                vtkPoints* PointListCell, vtkPoints* PointListEdge);

        void GetEdge0(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge1(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge2(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge3(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge4(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge5(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge6(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge7(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge8(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge9(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge10(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void GetEdge11(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);

        void SetEdge(int CellNum, int EdgeNum, vtkUnstructuredGrid* BBox,  
                vtkPoints* PointListCell, vtkPoints* PointListEdge);

        void SetEdge0(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge1(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge2(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge3(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge4(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge5(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge6(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge7(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge8(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge9(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge10(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        void SetEdge11(int CellNum,  vtkPoints *PointListCell, vtkUnstructuredGrid *BBox, vtkPoints* PointList);
        int CheckIfEdgeNodesRecalculated(int EdgeNum, vtkIdList* IdList);

  vtkMimxRecalculateInteriorNodes(const vtkMimxRecalculateInteriorNodes&);  // Not implemented.
  void operator=(const vtkMimxRecalculateInteriorNodes&);  // Not implemented.
};

#endif
