/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSolidTransfiniteInterpolation.h,v $
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
// .NAME vtkMimxSolidTransfiniteInterpolation - deform geometry with vector data
// .SECTION Description
// vtkMimxSolidTransfiniteInterpolation is a filter that takes in list of points and 
// number of divisions along parametric co-ordinate axes I, J and K. the output contains 
// the co-ordinates of the interior nodes which could be used to create a structured grid.

#ifndef __vtkMimxSolidTransfiniteInterpolation_h
#define __vtkMimxSolidTransfiniteInterpolation_h

#include "vtkFilter.h"

#include "vtkPointSetAlgorithm.h"

class VTK_MIMXFILTER_EXPORT vtkMimxSolidTransfiniteInterpolation : public vtkPointSetAlgorithm
{
public:
  static vtkMimxSolidTransfiniteInterpolation *New();
  vtkTypeRevisionMacro(vtkMimxSolidTransfiniteInterpolation,vtkPointSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Number of nodes along the parametric axes I and J.
  vtkSetMacro(IDiv,int);
  vtkGetMacro(IDiv,int);
  vtkSetMacro(JDiv,int);
  vtkGetMacro(JDiv,int);
  vtkSetMacro(KDiv,int);
  vtkGetMacro(KDiv,int);

protected:
  vtkMimxSolidTransfiniteInterpolation();
  ~vtkMimxSolidTransfiniteInterpolation();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int IDiv, JDiv, KDiv;
private:
  vtkMimxSolidTransfiniteInterpolation(const vtkMimxSolidTransfiniteInterpolation&);  // Not implemented.
  void operator=(const vtkMimxSolidTransfiniteInterpolation&);  // Not implemented.
};

#endif
