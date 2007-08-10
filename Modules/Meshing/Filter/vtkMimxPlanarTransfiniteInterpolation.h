/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxPlanarTransfiniteInterpolation.h,v $
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

// .NAME vtkMimxPlanarTransfiniteInterpolation - deform geometry with vector data
// .SECTION Description
// vtkMimxPlanarTransfiniteInterpolation is a filter that takes in list of points and 
// number of divisions along parametric co-ordinate axes I and J. the output contains 
// calculated interior nodes which could be used to create a surface

#ifndef __vtkMimxPlanarTransfiniteInterpolation_h
#define __vtkMimxPlanarTransfiniteInterpolation_h

#include "vtkPointSetAlgorithm.h"

#include "vtkFilter.h"

class  VTK_MIMXFILTER_EXPORT vtkMimxPlanarTransfiniteInterpolation : public vtkPointSetAlgorithm
{
public:
  static vtkMimxPlanarTransfiniteInterpolation *New();
  vtkTypeRevisionMacro(vtkMimxPlanarTransfiniteInterpolation,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Number of nodes along the parametric axes I and J and K.
  vtkSetMacro(IDiv,int);
  vtkGetMacro(IDiv,int);
  vtkSetMacro(JDiv,int);
  vtkGetMacro(JDiv,int);

protected:
  vtkMimxPlanarTransfiniteInterpolation();
  ~vtkMimxPlanarTransfiniteInterpolation();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int IDiv, JDiv;
private:
  vtkMimxPlanarTransfiniteInterpolation(const vtkMimxPlanarTransfiniteInterpolation&);  // Not implemented.
  void operator=(const vtkMimxPlanarTransfiniteInterpolation&);  // Not implemented.
};

#endif
