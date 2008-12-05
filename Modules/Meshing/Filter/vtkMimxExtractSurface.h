/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractSurface.h,v $
Language:  C++
Date:      $Date: 2008/03/18 14:52:24 $
Version:   $Revision: 1.1 $

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

// .NAME vtkMimxExtractSurface - select portion of polygonal mesh; generate selection scalars
// .SECTION Description
// vtkMimxExtractSurface is a filter that selects polygonal data based on
// defining a "loop" and indicating the region inside of the loop. The
// function uses vtkSelectPolyData class to determine the list of elements
// lying inside the loop. Unlike vtkSelectPolyData the input could be an
// unstructuredgrid or a polydata and the original cell connectivity is maintained
// (vtkSelectPolyData triangulates the output). i.e. if an hexahedral mesh is input,
// the surface mesh is a quadrilateral and so on.

// To determine what portion of the mesh is inside and outside of the
// loop,  the elements lying within the loop, is the default choice and is
// the first output (this can be accessed using GetSurfaceIn) and the
// of the rest of the region can be accessed using GetSurfaceOut. List
// of points and cells with the respective regions can also be accessed.

// .SECTION Caveats
// Make sure that the points you pick are on a connected surface. If
// not, then the filter will generate an empty or partial result. Also,
// self-intersecting loops will generate unpredictable results.
//
// During processing of the data, non-triangular cells are converted to
// triangles if GenerateSelectionScalars is off.

// .SECTION See Also
// vtkImplicitSelectionLoop, vtkSelectPolyData

#ifndef __vtkMimxExtractSurface_h
#define __vtkMimxExtractSurface_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkIdList.h"
#include "vtkMimxFilterWin32Header.h"

class vtkIdList;
class vtkPoints;
class vtkPolyData;

class VTK_MIMXFILTER_EXPORT vtkMimxExtractSurface : public vtkPolyDataAlgorithm
{
public:
  // Description:
  // Instantiate object with InsideOut turned off, and 
  // GenerateSelectionScalars turned off. The unselected output
  // is not generated, and the inside mode is the smallest region.
  static vtkMimxExtractSurface *New();

  vtkTypeRevisionMacro(vtkMimxExtractSurface,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetObjectMacro(CellIdList, vtkIdList);
  vtkSetObjectMacro(FaceIdList, vtkIdList);
 
  unsigned long int GetMTime();

protected:
  vtkMimxExtractSurface();
  ~vtkMimxExtractSurface();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int FillInputPortInformation(int, vtkInformation *);

  vtkIdList *CellIdList;
  vtkIdList *FaceIdList;  
private:
  vtkMimxExtractSurface(const vtkMimxExtractSurface&);  // Not implemented.
  void operator=(const vtkMimxExtractSurface&);  // Not implemented.
};


#endif


