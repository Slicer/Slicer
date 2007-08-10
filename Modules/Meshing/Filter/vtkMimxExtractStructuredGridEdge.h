/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractStructuredGridEdge.h,v $
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

// .NAME vtkMimxExtractStructuredGridEdge - Extract elements forming a edge of a structured
// grid. The output is a polydata line.

// .SECTION Description
// vtkMimxExtractStructuredGridEdge requires a structured grid as an input and outputs a 
// polydata as an output. The edge number to be extracted should be input. The edge 
// numbering follows vtkHexaheron convention.

// .SECTION See Also
// vtkHexahedron
#ifndef __vtkMimxExtractStructuredGridEdge_h
#define __vtkMimxExtractStructuredGridEdge_h

#include "vtkFilter.h"

#include "vtkPolyDataAlgorithm.h"

class vtkStructuredGrid;


class VTK_MIMXFILTER_EXPORT vtkMimxExtractStructuredGridEdge : public vtkPolyDataAlgorithm
{
public:
  static vtkMimxExtractStructuredGridEdge *New();
  vtkTypeRevisionMacro(vtkMimxExtractStructuredGridEdge,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(EdgeNum, int);
  protected:
  vtkMimxExtractStructuredGridEdge();
  ~vtkMimxExtractStructuredGridEdge();

private:

        virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
        virtual int FillInputPortInformation(int , vtkInformation *);

        int EdgeNum;
        void GetEdge(int , vtkPoints* , vtkStructuredGrid*);

        void GetEdge0(vtkPoints* , vtkStructuredGrid*);
        void GetEdge1(vtkPoints* , vtkStructuredGrid*);
        void GetEdge2(vtkPoints* , vtkStructuredGrid*);
        void GetEdge3(vtkPoints* , vtkStructuredGrid*);
        void GetEdge4(vtkPoints* , vtkStructuredGrid*);
        void GetEdge5(vtkPoints* , vtkStructuredGrid*);
        void GetEdge6(vtkPoints* , vtkStructuredGrid*);
        void GetEdge7(vtkPoints* , vtkStructuredGrid*);
        void GetEdge8(vtkPoints* , vtkStructuredGrid*);
        void GetEdge9(vtkPoints* , vtkStructuredGrid*);
        void GetEdge10(vtkPoints* , vtkStructuredGrid*);
        void GetEdge11(vtkPoints* , vtkStructuredGrid*);

  vtkMimxExtractStructuredGridEdge(const vtkMimxExtractStructuredGridEdge&);  // Not implemented.
  void operator=(const vtkMimxExtractStructuredGridEdge&);  // Not implemented.
};

#endif
