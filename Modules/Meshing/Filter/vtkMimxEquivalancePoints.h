/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEquivalancePoints.h,v $
Language:  C++
Date:      $Date: 2008/02/05 18:26:00 $
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

// .NAME vtkMimxEquivalancePoints - Introduces two arrays, Original_Cell_Ids
// belonging to CellData and Original_Point_Ids belonging to PointData to the PartialMesh
// input. The two arrays should be integer arrays.Right now the input itself is being 
// modified but should be modified later on
// .SECTION Description
// vtkMimxEquivalancePoints has two inputs. A CompleteMesh and a PartialMesh.
// PartialMesh should be a subset of CompleteMesh else the filter will not work.
// The CompleteMesh may or may not contain Arrays Original_Point_Ids and Original_Cell_Ids.
// If the arrays are not present the filter will assign the arrays to the CompleteMesh and
// carry out mapping accordingly. Here mapping is finding corresponding point and cell
// in PartialMesh with respect to CompleteMesh.

#ifndef __vtkMimxEquivalancePoints_h
#define __vtkMimxEquivalancePoints_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"


class VTK_MIMXFILTER_EXPORT vtkMimxEquivalancePoints : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxEquivalancePoints *New();
  vtkTypeRevisionMacro(vtkMimxEquivalancePoints,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

 
protected:
  vtkMimxEquivalancePoints();
  ~vtkMimxEquivalancePoints();

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkMimxEquivalancePoints(const vtkMimxEquivalancePoints&);  // Not implemented.
  void operator=(const vtkMimxEquivalancePoints&);  // Not implemented.
};

#endif
