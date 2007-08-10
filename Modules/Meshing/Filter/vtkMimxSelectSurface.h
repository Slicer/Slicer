/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectSurface.h,v $
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

// .NAME vtkMimxSelectSurface - select portion of polygonal mesh; generate selection scalars
// .SECTION Description
// vtkMimxSelectSurface is a filter that selects polygonal data based on
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

#ifndef __vtkMimxSelectSurface_h
#define __vtkMimxSelectSurface_h

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class vtkIdList;
class vtkPoints;
class vtkPolyData;

class VTK_MIMXFILTER_EXPORT vtkMimxSelectSurface : public vtkPolyDataAlgorithm
{
public:
  // Description:
  // Instantiate object with InsideOut turned off, and 
  // GenerateSelectionScalars turned off. The unselected output
  // is not generated, and the inside mode is the smallest region.
  static vtkMimxSelectSurface *New();

  vtkTypeRevisionMacro(vtkMimxSelectSurface,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the array of point coordinates defining the loop. There must
  // be at least three points used to define a loop.
  virtual void SetLoop(vtkIdList*);
  vtkGetObjectMacro(Loop,vtkIdList);

  // Description:
  // Get Cell and Point ID lists of the two sets of data
  // 1 refers to cells lying within the loop and 2 the other set of elements
  // lying outside the loop.
  vtkGetObjectMacro(CellIdListIn, vtkIdList);
  vtkGetObjectMacro(PointIdListIn, vtkIdList);
  vtkGetObjectMacro(CellIdListOut, vtkIdList);
  vtkGetObjectMacro(PointIdListOut, vtkIdList);

  // Description:
  // Get surface depicted by polydata inside and outside the loop
  vtkPolyData* GetSurfaceIn();
  vtkPolyData* GetSurfaceOut();

  unsigned long int GetMTime();

protected:
  vtkMimxSelectSurface();
  ~vtkMimxSelectSurface();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int FillInputPortInformation(int, vtkInformation *);

  vtkIdList *Loop;
  vtkIdList *CellIdListIn;
  vtkIdList *PointIdListIn;
  vtkIdList *CellIdListOut;
  vtkIdList *PointIdListOut;
  
private:
  vtkMimxSelectSurface(const vtkMimxSelectSurface&);  // Not implemented.
  void operator=(const vtkMimxSelectSurface&);  // Not implemented.
};


#endif


