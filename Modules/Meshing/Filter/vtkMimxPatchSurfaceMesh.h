/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPatchSurfaceMesh.h,v $
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

// .NAME vtkMimxPatchSurfaceMesh - patch-up holes in a surface data using 
// delaunay triangulation
// .SECTION Description
// vtkMimxPatchSurfaceMesh is a filter to patch holes in a surface data. It
// handles multiple holes and uses delaunay2D to triangulate the hole.
// input can be any dataset and output is a polydata. The filter also outputs
// the points forming the boundary as a polydata.
// .SECTION Caveats
// the hole being patched should be convex. the concave hole might not be
// patched accurately.

#ifndef __vtkMimxPatchSurfaceMesh_h
#define __vtkMimxPatchSurfaceMesh_h

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class vtkPointLocator;
class vtkPolyData;
class vtkPolyDataCollection;

class VTK_MIMXFILTER_EXPORT vtkMimxPatchSurfaceMesh : public vtkPolyDataAlgorithm
{

public:
  static vtkMimxPatchSurfaceMesh *New();
  vtkTypeRevisionMacro(vtkMimxPatchSurfaceMesh,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  // to access the contours generated
  vtkPolyData* GetContour(int );
  vtkGetMacro(NumberOfContours, int);
protected:
  vtkMimxPatchSurfaceMesh();
  ~vtkMimxPatchSurfaceMesh();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  //computation function
  void PolyDataExecute(vtkPolyData *, vtkPolyData *, vtkInformation *);
  // to check and insert a boundary edge
  int IsEdge(vtkIdList *, vtkIdList *, vtkIdType , vtkIdType );
  int GetCellContainingPoint(vtkIdType , vtkIdList *, vtkIdType );
  // to check if all the boundary edges been included
  int HaveAllEdgesUtilized(vtkIntArray *);
        
  vtkPolyDataCollection *ContourCollection;
  int NumberOfContours;
private:
  vtkMimxPatchSurfaceMesh(const vtkMimxPatchSurfaceMesh&);  // Not implemented.
  void operator=(const vtkMimxPatchSurfaceMesh&);  // Not implemented.
};

#endif


