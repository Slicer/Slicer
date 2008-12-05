/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMirrorUnstructuredHexahedronGridCell.h,v $
Language:  C++
Date:      $Date: 2007/11/09 21:09:25 $
Version:   $Revision: 1.8 $

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

// .NAME vtkMimxMirrorUnstructuredHexahedronGridCell - 
// .SECTION Caveats
// Filter takes in an unstructuredgrid as primary input. The axis perpendicular to
// and the point about which the grid has to be mirrored. 

#ifndef __vtkMimxMirrorUnstructuredHexahedronGridCell_h
#define __vtkMimxMirrorUnstructuredHexahedronGridCell_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPlane.h"
#include "vtkMimxFilterWin32Header.h"

class vtkIdList;

class VTK_MIMXFILTER_EXPORT vtkMimxMirrorUnstructuredHexahedronGridCell : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxMirrorUnstructuredHexahedronGridCell *New();
  vtkTypeRevisionMacro(vtkMimxMirrorUnstructuredHexahedronGridCell,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetObjectMacro(MirrorPlane, vtkPlane);
  void CopyConstraintValues(vtkUnstructuredGrid *input, vtkUnstructuredGrid *output);
protected:
  vtkMimxMirrorUnstructuredHexahedronGridCell();
  ~vtkMimxMirrorUnstructuredHexahedronGridCell();
  vtkIdList *IdList;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  // 0 for X, 1 for Y and 2 for Z
  vtkPlane *MirrorPlane;
  private:
  vtkMimxMirrorUnstructuredHexahedronGridCell(const vtkMimxMirrorUnstructuredHexahedronGridCell&);  // Not implemented.
  void operator=(const vtkMimxMirrorUnstructuredHexahedronGridCell&);  // Not implemented.
};

#endif
