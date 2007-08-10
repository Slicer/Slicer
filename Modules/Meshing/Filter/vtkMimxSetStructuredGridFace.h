/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSetStructuredGridFace.h,v $
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

// .NAME vtkMimxSetStructuredGridFace - Set elements forming a face of a structured
// grid. The numbering follows regular convention of vtkHexhedron

// .SECTION Description
// vtkMimxSetStructuredGridFace requires a structured grid as an input and outputs a 
// structuredgrid as an output. the dimension of the output in Z will always be 1.

// .SECTION See Also
// vtkStructuredGrid

#ifndef __vtkMimxSetStructuredGridFace_h
#define __vtkMimxSetStructuredGridFace_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"



class VTK_MIMXFILTER_EXPORT vtkMimxSetStructuredGridFace : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxSetStructuredGridFace *New();
  vtkTypeRevisionMacro(vtkMimxSetStructuredGridFace,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(FaceNum, int);
  void SetFace(vtkStructuredGrid *);

  protected:
  vtkMimxSetStructuredGridFace();
  ~vtkMimxSetStructuredGridFace();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int , vtkInformation *);
private:
        int FaceNum;
        void SetFaceCase(int , vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);

        void SetFace0(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);
        void SetFace1(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);
        void SetFace2(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);
        void SetFace3(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);
        void SetFace4(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);
        void SetFace5(vtkStructuredGrid* , vtkStructuredGrid*, vtkStructuredGrid*);

  vtkMimxSetStructuredGridFace(const vtkMimxSetStructuredGridFace&);  // Not implemented.
  void operator=(const vtkMimxSetStructuredGridFace&);  // Not implemented.
};

#endif
