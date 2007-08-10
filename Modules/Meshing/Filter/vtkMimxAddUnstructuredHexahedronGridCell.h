/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAddUnstructuredHexahedronGridCell.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.9 $

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

// .NAME vtkMimxAddUnstructuredHexahedronGridCell - 
// .SECTION Caveats
// Filter takes in an unstructuredgrid as primary input. PointsList input
// should contain 4 points denoting an face in the unstructured grid.
// the output contains unstructured grid with the cell containing the
// face will have a newly extruded neighboring cell sharing an edge with the
// face originally picked.

#ifndef __vtkMimxAddUnstructuredHexahedronGridCell_h
#define __vtkMimxAddUnstructuredHexahedronGridCell_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkIdList;

class VTK_MIMXFILTER_EXPORT vtkMimxAddUnstructuredHexahedronGridCell : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxAddUnstructuredHexahedronGridCell *New();
  vtkTypeRevisionMacro(vtkMimxAddUnstructuredHexahedronGridCell,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(IdList, vtkIdList*);
  vtkSetMacro(ExtrusionLength, double);
protected:
  vtkMimxAddUnstructuredHexahedronGridCell();
  ~vtkMimxAddUnstructuredHexahedronGridCell();
  vtkIdList *IdList;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double ExtrusionLength;
  private:
  vtkMimxAddUnstructuredHexahedronGridCell(const vtkMimxAddUnstructuredHexahedronGridCell&);  // Not implemented.
  void operator=(const vtkMimxAddUnstructuredHexahedronGridCell&);  // Not implemented.
};

#endif
