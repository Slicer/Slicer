/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractStructuredGridFace.h,v $
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

// .NAME vtkMimxExtractStructuredGridFace - Extract elements forming a face of a structured
// grid. The numbering follows regular convention of vtkHexhedron

// .SECTION Description
// vtkMimxExtractStructuredGridFace requires a structured grid as an input and outputs a 
// structuredgrid as an output. the dimension of the output in Z will always be 1.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxExtractStructuredGridFace_h
#define __vtkMimxExtractStructuredGridFace_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"



class VTK_MIMXFILTER_EXPORT vtkMimxExtractStructuredGridFace : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxExtractStructuredGridFace *New();
  vtkTypeRevisionMacro(vtkMimxExtractStructuredGridFace,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(FaceNum, int);
  protected:
  vtkMimxExtractStructuredGridFace();
  ~vtkMimxExtractStructuredGridFace();
  
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
private:
        int FaceNum;
        void GetFace(int , vtkPoints* , vtkStructuredGrid*);

        void GetFace0(vtkPoints* , vtkStructuredGrid*);
        void GetFace1(vtkPoints* , vtkStructuredGrid*);
        void GetFace2(vtkPoints* , vtkStructuredGrid*);
        void GetFace3(vtkPoints* , vtkStructuredGrid*);
        void GetFace4(vtkPoints* , vtkStructuredGrid*);
        void GetFace5(vtkPoints* , vtkStructuredGrid*);

  vtkMimxExtractStructuredGridFace(const vtkMimxExtractStructuredGridFace&);  // Not implemented.
  void operator=(const vtkMimxExtractStructuredGridFace&);  // Not implemented.
};

#endif
