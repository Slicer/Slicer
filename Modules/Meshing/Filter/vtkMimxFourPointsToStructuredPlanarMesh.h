/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxFourPointsToStructuredPlanarMesh.h,v $
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

// .NAME vtkMimxFourPointsToStructuredPlanarMesh - Generate a structured planar mesh from a 
//       list of quadrilateral polygons.

// .SECTION Description
// vtkMimxFourPointsToStructuredPlanarMesh is a filter that takes in a polygonal quadrilateral mesh
// and list of four points ids. the four points represent the corners of a structured planar mesh
// that would be generated. The ordering of point ids should be in either clockwise or 
// counter clock-wise direction. It is the user's responsibility to make sure the four corners
// form a structured planar mesh.

// .SECTION See Also
// vtkGeometryFilter vtkExtractGeometry vtkExtractVOI 
// vtkStructuredGridGeometryFilter

#ifndef __vtkMimxFourPointsToStructuredPlanarMesh_h
#define __vtkMimxFourPointsToStructuredPlanarMesh_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxFourPointsToStructuredPlanarMesh : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxFourPointsToStructuredPlanarMesh *New();
  vtkTypeRevisionMacro(vtkMimxFourPointsToStructuredPlanarMesh,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(PointList, vtkIdList*);
  vtkGetMacro(CorrespondingPointList, vtkIdList*);
protected:
  vtkMimxFourPointsToStructuredPlanarMesh();
  ~vtkMimxFourPointsToStructuredPlanarMesh();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);
  
   vtkIdList* PointList;
   vtkIdList *CorrespondingPointList;

private:
  vtkMimxFourPointsToStructuredPlanarMesh(const vtkMimxFourPointsToStructuredPlanarMesh&);  // Not implemented.
  void operator=(const vtkMimxFourPointsToStructuredPlanarMesh&);  // Not implemented.
};

#endif


