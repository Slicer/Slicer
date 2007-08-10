/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridFromBoundingBox.h,v $
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

// .NAME vtkMimxUnstructuredGridFromBoundingBox - 
// .SECTION Caveats
// Filter takes in a bounding box and a surface definition and generates
// a solid unstructured grid based on the mesh seeding in the bounding box

#ifndef __vtkMimxUnstructuredGridFromBoundingBox_h
#define __vtkMimxUnstructuredGridFromBoundingBox_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"
class vtkPolyData;
class vtkUnstructuredGrid;

class VTK_MIMXFILTER_EXPORT vtkMimxUnstructuredGridFromBoundingBox : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxUnstructuredGridFromBoundingBox *New();
  vtkTypeRevisionMacro(vtkMimxUnstructuredGridFromBoundingBox,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetBoundingBox(vtkUnstructuredGrid*);
  void SetSurface(vtkPolyData*);
protected:
  vtkMimxUnstructuredGridFromBoundingBox();
  ~vtkMimxUnstructuredGridFromBoundingBox();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation*);
  
  private:
  vtkMimxUnstructuredGridFromBoundingBox(const vtkMimxUnstructuredGridFromBoundingBox&);  // Not implemented.
  void operator=(const vtkMimxUnstructuredGridFromBoundingBox&);  // Not implemented.
};

#endif
