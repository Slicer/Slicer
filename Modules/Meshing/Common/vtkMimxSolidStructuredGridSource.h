/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSolidStructuredGridSource.h,v $
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

// .NAME vtkMimxSolidStructuredGridSource - orthogonal hexahedron 3D widget
// .SECTION Description
// vtkMimxSolidStructuredGridSource creates an unstructured grid of one hexahedron.
// The co-ordinates are determined by the bounds of the input polydata.

#ifndef _vtkMimxSolidStructuredGridSource_h
#define _vtkMimxSolidStructuredGridSource_h

#include "vtkCommon.h"

#include "vtkPolyDataSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkDataSetMapper.h"
#include "vtkStructuredGrid.h"

class VTK_MIMXCOMMON_EXPORT vtkMimxSolidStructuredGridSource : public vtkPolyDataSource
{
public:
  vtkTypeRevisionMacro(vtkMimxSolidStructuredGridSource,vtkPolyDataSource);
  static vtkMimxSolidStructuredGridSource *New();
  vtkMimxSolidStructuredGridSource();
  ~vtkMimxSolidStructuredGridSource();
  vtkStructuredGrid* StructuredGrid;
  vtkDataSetMapper* Mapper;
  vtkActor* Actor;
  void SetStructuredGrid(vtkStructuredGrid *);
private:
  
protected:

};

#endif
