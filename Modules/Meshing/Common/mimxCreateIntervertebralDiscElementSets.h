/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxCreateIntervertebralDiscElementSets.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
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

// To associate starting node and element numbers for  output
// The node and element numbers are stored in vtkPointData and vtkCellData
// respectively. The array containing  nodes numbers will be 
// "_Nodes_Numbers" and that of elements is "_Element_Numbers".

#ifndef __CreateIntervertebralDiscElementSets_h
#define __CreateIntervertebralDiscElementSets_h

#include "vtkCommon.h"

class vtkUnstructuredGrid;
class vtkIntArray;

class VTK_MIMXCOMMON_EXPORT CreateIntervertebralDiscElementSets
{
public:
  CreateIntervertebralDiscElementSets();
  ~CreateIntervertebralDiscElementSets();
  void SetUnstructuredGrid(vtkUnstructuredGrid*);
  void SetBoundingBox(vtkUnstructuredGrid*);
  void SetStartingElementSetNumber(int StartElementSetNumber);
  void CreateElementSetArrays();
  void SetElementSetsName(const char *Name);
private:
  vtkUnstructuredGrid *UGrid;
  vtkUnstructuredGrid *BoundingBox;
  int StartingElementSetNumber;
  const char *ElementSetsName;
};

#endif
