/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarTrilinearInterpolation.h,v $
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

// .NAME vtkMimxPlanarTrilinearInterpolation - Create a solid structured grid 
//      given the number of nodes along each face and the position of these nodes.
// .SECTION Description
// vtkMimxPlanarTrilinearInterpolation is a filter that takes in list of points and 
// number of divisions along parametric co-ordinate axes I and J. the output contains 
// the co-ordinates of the interior nodes which could be used to create a structured grid.

#ifndef __vtkMimxPlanarTrilinearInterpolation_h
#define __vtkMimxPlanarTrilinearInterpolation_h

#include "vtkFilter.h"

#include "vtkPointSetAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxPlanarTrilinearInterpolation : public vtkPointSetAlgorithm
{
public:
  static vtkMimxPlanarTrilinearInterpolation *New();
  vtkTypeRevisionMacro(vtkMimxPlanarTrilinearInterpolation,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Number of nodes along the parametric axes I, J, and K.
  vtkSetMacro(IDiv,int);
  vtkGetMacro(IDiv,int);
  vtkSetMacro(JDiv,int);
  vtkGetMacro(JDiv,int);

protected:
  vtkMimxPlanarTrilinearInterpolation();
  ~vtkMimxPlanarTrilinearInterpolation();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int IDiv, JDiv;
private:
  vtkMimxPlanarTrilinearInterpolation(const vtkMimxPlanarTrilinearInterpolation&);  // Not implemented.
  void operator=(const vtkMimxPlanarTrilinearInterpolation&);  // Not implemented.
};

#endif
