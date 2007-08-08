/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxSource.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
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
// .NAME vtkMimxBoundingBoxSource - create a rectangular hexahedron
// .SECTION Description
// vtkMimxBoundingBoxSource creates a rectangular hexahedron which acts as a 
// bounding box. The inputs could be a polydata whose bounds are calculated
// or user defined bounds could be input. If no input is given, the xmin,xmax,
// ymin,ymax,zmin,zmax are 0,1,0,1,0,1 respectively. Bounds calculated by the
// polydata takes precedence over other inputs.

#ifndef __vtkMimxBoundingBoxSource_h
#define __vtkMimxBoundingBoxSource_h

#include "vtkCommon.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkActor;
class vtkDataSetMapper;
class vtkPolyData;

class VTK_MIMXCOMMON_EXPORT vtkMimxBoundingBoxSource : public vtkUnstructuredGridAlgorithm 
{
public:
  vtkTypeRevisionMacro(vtkMimxBoundingBoxSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct a bounding box
  static vtkMimxBoundingBoxSource *New();
  void SetBounds(double bounds[6]);
  vtkSetMacro(Source, vtkPolyData*);
  vtkGetMacro(Source,vtkPolyData*);
 
protected:
  vtkMimxBoundingBoxSource();
  ~vtkMimxBoundingBoxSource();
  double Bounds[6];
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  vtkPolyData* Source;
private:
  vtkMimxBoundingBoxSource(const vtkMimxBoundingBoxSource&);  // Not implemented.
  void operator=(const vtkMimxBoundingBoxSource&);  // Not implemented.
};

#endif
