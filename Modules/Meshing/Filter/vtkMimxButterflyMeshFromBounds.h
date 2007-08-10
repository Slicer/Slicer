/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxButterflyMeshFromBounds.h,v $
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

// .NAME vtkMimxButterflyMeshFromBounds - create a rectangular hexahedron
// .SECTION Description
// vtkMimxButterflyMeshFromBounds creates a rectangular hexahedron which acts as a 
// bounding box. The inputs could be a polydata whose bounds are calculated
// or user defined bounds could be input. If no input is given, the xmin,xmax,
// ymin,ymax,zmin,zmax are 0,1,0,1,0,1 respectively. Bounds calculated by the
// polydata takes precedence over other inputs. Number of layers designates the
// number rings in the mesh structure. As of now only two rings have been
// incorperated.

#ifndef __vtkMimxButterflyMeshFromBounds_h
#define __vtkMimxButterflyMeshFromBounds_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxButterflyMeshFromBounds : public vtkUnstructuredGridAlgorithm 
{
public:
  vtkTypeRevisionMacro(vtkMimxButterflyMeshFromBounds,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct a bounding box
  static vtkMimxButterflyMeshFromBounds *New();
  void SetBounds(double bounds[6]);
  vtkSetMacro(NumberOfLayers, int);
protected:
  vtkMimxButterflyMeshFromBounds();
  ~vtkMimxButterflyMeshFromBounds();
  double Bounds[6];
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int NumberOfLayers;
private:
  vtkMimxButterflyMeshFromBounds(const vtkMimxButterflyMeshFromBounds&);  // Not implemented.
  void operator=(const vtkMimxButterflyMeshFromBounds&);  // Not implemented.
};

#endif
