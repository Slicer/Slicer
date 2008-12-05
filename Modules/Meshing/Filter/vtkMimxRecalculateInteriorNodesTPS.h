/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRecalculateInteriorNodesTPS.h,v $
Language:  C++
Date:      $Date: 2007/10/18 21:38:00 $
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

// .NAME vtkMimxRecalculateInteriorNodesTPS - recalculate the interior nodes of 
// an unstructured grid. based on the deformation field of the source

// .SECTION Description
// vtkMimxRecalculateInteriorNodesTPS requires two inputs (both unstructured grids).
// the source is the undeformed grid and the target is the deformed grid. TPS is used to apply
// the transform. this transform works fast with number of points/vertices < 2000.

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxRecalculateInteriorNodesTPS_h
#define __vtkMimxRecalculateInteriorNodesTPS_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMimxFilterWin32Header.h"

class vtkUnstructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxRecalculateInteriorNodesTPS : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxRecalculateInteriorNodesTPS *New();
  vtkTypeRevisionMacro(vtkMimxRecalculateInteriorNodesTPS,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetSource(vtkUnstructuredGrid*);
  void SetTarget(vtkUnstructuredGrid*);
  vtkSetMacro(SamplingNumber, vtkIdType);
  protected:
  vtkMimxRecalculateInteriorNodesTPS();
  ~vtkMimxRecalculateInteriorNodesTPS();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        int SamplingNumber;     // every samplingnumber th point is used in calculating TPS
  vtkMimxRecalculateInteriorNodesTPS(const vtkMimxRecalculateInteriorNodesTPS&);  // Not implemented.
  void operator=(const vtkMimxRecalculateInteriorNodesTPS&);  // Not implemented.
};

#endif
