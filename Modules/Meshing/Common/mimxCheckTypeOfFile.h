/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxCheckTypeOfFile.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.3 $

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

// To check the type of file being opened
// different types of files have different enumerated values.
// files concidered are STL, VTK (polydata, unstructuredgrid, structuredgrid)

#ifndef __mimxCheckTypeOfFile_h
#define __mimxCheckTypeOfFile_h

#include "vtkCommon.h"

class vtkUnstructuredGrid;
class vtkIntArray;

class VTK_MIMXCOMMON_EXPORT mimxCheckTypeOfFile
{
public:
  mimxCheckTypeOfFile();
  ~mimxCheckTypeOfFile();
  // function to check the file type given the file name
  int Check(const char*);
  int DataType;
private:
  enum FileType
  {
    None=0,
    STL=1,
    SurfacePolyData=2,
    StructuredPlanarGrid=3,
    StructuredSolidGrid=4,
    UnstructuredGrid=5
  };
};

#endif
