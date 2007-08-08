/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxCheckTypeOfFile.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

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

#include "mimxCheckTypeOfFile.h"

#include <fstream>
using std::ifstream;
#include "string.h"
using std::string;

mimxCheckTypeOfFile::mimxCheckTypeOfFile()
{
  this->DataType = mimxCheckTypeOfFile::None;
}

mimxCheckTypeOfFile::~mimxCheckTypeOfFile()
{
}

int mimxCheckTypeOfFile::Check(const char* FileName)
{
  ifstream FileInput;
  FileInput.open(FileName,std::ios::in);
  if(FileInput)
  {
    // if a file is successfully opened
    // as of now only two file types are allowed .vtk and .stl
    char dummyStr[100];  
    string filename = FileName;
    int pos = filename.find(".stl");
    if(pos !=string::npos)
    {
      this->DataType = mimxCheckTypeOfFile::STL;
      return mimxCheckTypeOfFile::STL;
    }
      pos = filename.find(".vtk");
        if(pos != string::npos)
      {
        // read in the first three lines which are common
        // to all the vtk file formats
        FileInput.getline(dummyStr, 50, '\n');
        FileInput.getline(dummyStr, 50, '\n');
        FileInput.getline(dummyStr, 50, '\n');
        char dataset[100], datatype[100];
        char dimensions[100];
        int dim[3];
        FileInput >> dataset >> datatype;
        if(!strcmp(datatype, "POLYDATA")){
          this->DataType = mimxCheckTypeOfFile::SurfacePolyData;
          FileInput.close();
        return mimxCheckTypeOfFile::SurfacePolyData;}
        if(!strcmp(datatype, "UNSTRUCTURED_GRID")){  
          this->DataType = mimxCheckTypeOfFile::UnstructuredGrid;
          FileInput.close();
          return mimxCheckTypeOfFile::UnstructuredGrid;}
        if(!strcmp(datatype, "STRUCTURED_GRID"))
        {
          FileInput >> dimensions >> dim[0] >> dim[1] >> dim[2];
          if(dim[0] == 1 || dim[1] == 1 || dim[2] == 1)
          {
            this->DataType = mimxCheckTypeOfFile::StructuredPlanarGrid;
            FileInput.close();
            return mimxCheckTypeOfFile::StructuredPlanarGrid;
          }
          else
          {
            this->DataType = mimxCheckTypeOfFile::StructuredSolidGrid;
            FileInput.close();
            return mimxCheckTypeOfFile::StructuredSolidGrid;
          }  
        }
    }
      FileInput.close();
      return mimxCheckTypeOfFile::None;
  }
  else{
    FileInput.close();
    return mimxCheckTypeOfFile::None;
  }
}
