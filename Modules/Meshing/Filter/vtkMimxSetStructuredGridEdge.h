/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSetStructuredGridEdge.h,v $
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

// .NAME vtkMimxSetStructuredGridEdge - Set elements forming a edge of a structured
// grid. The output is a polydata line.

// .SECTION Description
// vtkMimxSetStructuredGridEdge requires a polydata as an input and outputs a 
// polydata as an output. The edge number to be extracted should be input. The edge 
// numbering follows vtkHexaheron convention.

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxSetStructuredGridEdge_h
#define __vtkMimxSetStructuredGridEdge_h

#include "vtkFilter.h"

#include "vtkStructuredGridAlgorithm.h"

class vtkPolyData;


class VTK_MIMXFILTER_EXPORT vtkMimxSetStructuredGridEdge : public vtkStructuredGridAlgorithm
{
public:
  static vtkMimxSetStructuredGridEdge *New();
  vtkTypeRevisionMacro(vtkMimxSetStructuredGridEdge,vtkStructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(EdgeNum, int);

  void SetEdge(vtkPolyData*);
  protected:
  vtkMimxSetStructuredGridEdge();
  ~vtkMimxSetStructuredGridEdge();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        virtual int FillInputPortInformation(int , vtkInformation *);

        int EdgeNum;
        void SetEdge(int , vtkPoints* , vtkStructuredGrid*);

        void SetEdge0(vtkPoints* , vtkStructuredGrid*);
        void SetEdge1(vtkPoints* , vtkStructuredGrid*);
        void SetEdge2(vtkPoints* , vtkStructuredGrid*);
        void SetEdge3(vtkPoints* , vtkStructuredGrid*);
        void SetEdge4(vtkPoints* , vtkStructuredGrid*);
        void SetEdge5(vtkPoints* , vtkStructuredGrid*);
        void SetEdge6(vtkPoints* , vtkStructuredGrid*);
        void SetEdge7(vtkPoints* , vtkStructuredGrid*);
        void SetEdge8(vtkPoints* , vtkStructuredGrid*);
        void SetEdge9(vtkPoints* , vtkStructuredGrid*);
        void SetEdge10(vtkPoints* , vtkStructuredGrid*);
        void SetEdge11(vtkPoints* , vtkStructuredGrid*);

  vtkMimxSetStructuredGridEdge(const vtkMimxSetStructuredGridEdge&);  // Not implemented.
  void operator=(const vtkMimxSetStructuredGridEdge&);  // Not implemented.
};

#endif
