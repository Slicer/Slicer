/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAttachBoundingBoxMesh.h,v $
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

// .NAME vtkMimxAttachBoundingBoxMesh - 
// .SECTION Description
// Currently this filter is implemented specifically for attaching posterior elements
// of vertebra with the vertebral body mesh. The filter takes in 4 inputs. The primary
// input is an all hexahedron unstructuredgrid of the vertebral body. The second input
// is set through SetBoundingBox. The boundingbox is an unstructuredgrid which forms the
// skeleton of the posterior element mesh. By specifying the mesh seeding, all hexahedron
// mesh can be generated. The third input (SetCompleteSource)is the complete surface 
// definition onto which the bounding box is morphed on using closest point projection. 
// The fourth input (SetCutSource) is the same surface definition as in the third input 
// but with the posterior elements cut-out such that two holes exist where the posterior 
// elements attach.
// .SECTION caveats
// The bounding box points which attach to the vertebral body should lie on the vertebral 
// body mesh. Since the vtkPointLocator is used to check for the proximity of the two
// nodes and to determine which face of the boundingbox needs to be attached. The filter
// works well if the bounding box faces attaching to the vertebral body approximate the
// hole as closely as possible. The points on the vertebral body mesh corresponding to
// the points on the bounding box when connected with each other should form a structured
// planar mesh.

#ifndef __vtkMimxAttachBoundingBoxMesh_h
#define __vtkMimxAttachBoundingBoxMesh_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkCollection;
class vtkIdList;
class vtkMimxPatchSurfaceMesh;
class vtkPolyData;

class VTK_MIMXFILTER_EXPORT vtkMimxAttachBoundingBoxMesh : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxAttachBoundingBoxMesh *New();
  void SetBoundingBox(vtkUnstructuredGrid*);
  void SetCompleteSource(vtkPolyData*);
  void SetCutSource(vtkPolyData*);
  vtkTypeRevisionMacro(vtkMimxAttachBoundingBoxMesh,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkCollection *SGridCollection;
  void SetUnstructuredMesh(vtkUnstructuredGrid*);
  vtkGetMacro(ModifiedInput, vtkUnstructuredGrid*);
protected:
  vtkMimxAttachBoundingBoxMesh();
  ~vtkMimxAttachBoundingBoxMesh();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation*);
  int GetClosestContour(vtkMimxPatchSurfaceMesh*, vtkUnstructuredGrid*, vtkIdList*);
  int CheckBoundaryFace(vtkIdList* , vtkPolyData *);
  int CheckInteriorEdge(vtkIdList* , vtkPolyData *);
  vtkUnstructuredGrid *ModifiedInput;
  private:
  vtkMimxAttachBoundingBoxMesh(const vtkMimxAttachBoundingBoxMesh&);  // Not implemented.
  void operator=(const vtkMimxAttachBoundingBoxMesh&);  // Not implemented.
};

#endif
