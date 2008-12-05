/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGeometryFilter.h,v $
Language:  C++
Date:      $Date: 2008/08/14 05:01:51 $
Version:   $Revision: 1.2 $

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

// .NAME vtkMimxGeometryFilter - Set elements forming a edge of a structured
// grid. The output is a polydata line.

// .SECTION Description
// vtkMimxGeometryFilter requires two inputs (both unstructured grids).
// This filter is used to decompose the merged unstructured grids to individual 
// grids (structured). the first input will be the merged unstructured grids.
// the second input will be the underlying bounding box structure from which the 
// first input was generated. The second input's vectors will contain the 
// mesh seeding definition for individual cells in the grid

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxGeometryFilter_h
#define __vtkMimxGeometryFilter_h

#include "vtkPolyDataAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

class vtkUnstructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxGeometryFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkMimxGeometryFilter *New();
  vtkTypeRevisionMacro(vtkMimxGeometryFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  protected:
  vtkMimxGeometryFilter();
  ~vtkMimxGeometryFilter();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        virtual int FillInputPortInformation(int , vtkInformation *);

  vtkMimxGeometryFilter(const vtkMimxGeometryFilter&);  // Not implemented.
  void operator=(const vtkMimxGeometryFilter&);  // Not implemented.
};

#endif
