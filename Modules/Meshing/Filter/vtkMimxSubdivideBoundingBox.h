/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSubdivideBoundingBox.h,v $
Language:  C++
Date:      $Date: 2007/10/11 19:54:47 $
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

// .NAME vtkMimxSubdivideBoundingbox - 
// .SECTION Caveats
// Filter takes in an unstructuredgrid as primary input. PointsList input
// should contain 4 points denoting an face in the unstructured grid.
// the output contains unstructured grid with the cell containing the
// face will have a newly extruded neighboring cell sharing an edge with the
// face originally picked.

#ifndef __vtkMimxSubdivideBoundingbox_h
#define __vtkMimxSubdivideBoundingbox_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

class vtkIdList;

class VTK_MIMXFILTER_EXPORT vtkMimxSubdivideBoundingbox : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxSubdivideBoundingbox *New();
  vtkTypeRevisionMacro(vtkMimxSubdivideBoundingbox,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(CellNum, vtkIdType);
protected:
  vtkMimxSubdivideBoundingbox();
  ~vtkMimxSubdivideBoundingbox();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  vtkIdType CellNum;
  private:
  vtkMimxSubdivideBoundingbox(const vtkMimxSubdivideBoundingbox&);  // Not implemented.
  void operator=(const vtkMimxSubdivideBoundingbox&);  // Not implemented.
};

#endif
