/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxIntervertebralDiscAbaqusWriter.cxx,v $
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

#include "vtkMimxIntervertebralDiscAbaqusWriter.h"

#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include <string.h>

vtkCxxRevisionMacro(vtkMimxIntervertebralDiscAbaqusWriter, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxIntervertebralDiscAbaqusWriter);

vtkMimxIntervertebralDiscAbaqusWriter::vtkMimxIntervertebralDiscAbaqusWriter()
{
  this->SetNumberOfInputPorts(2);
  Modulus = 200000;
  PoissonsRatio = 0.35;
  this->StartingElementNumber = 1;
  this->StartingNodeNumber = 1;
}

vtkMimxIntervertebralDiscAbaqusWriter::~vtkMimxIntervertebralDiscAbaqusWriter()
{
}


//----------------------------------------------------------------------------
int vtkMimxIntervertebralDiscAbaqusWriter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
  {
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *bboxInfo = inputVector[1]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkDataSet *input = vtkDataSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));
        vtkDataSet *bbox = vtkDataSet::SafeDownCast(
                bboxInfo->Get(vtkDataObject::DATA_OBJECT()));

    if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID && 
                bbox->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {   
      this->UnstGridExecute( vtkUnstructuredGrid::SafeDownCast( input ),
                  vtkUnstructuredGrid::SafeDownCast( bbox ));
    }
        else    
        {
                vtkErrorMacro("Only Unstructured grid data types allowed");
                return 0;
        }
    return 1;
  }

  void vtkMimxIntervertebralDiscAbaqusWriter::UnstGridExecute( vtkUnstructuredGrid *input, vtkUnstructuredGrid *bbox )
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int numOfCells = input->GetNumberOfCells( );
    
    std::ofstream out;  
    out.open(this->FileName, std::ios::out );
    
    /* Writing out Nodal Definitions */
    out << "*NODE" << endl;
    double point[3];
    for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    {
      input->GetPoint( pointIndex, point );
      out << ( pointIndex + this->StartingNodeNumber ) << ", " << point[0] << ", " << point[1] << ", " << point[2] <<std::endl;
    }

    /* Writing out Element Definitions */
    out << "** \n** \n*ELEMENT, TYPE=, ELSET=" << endl;
    vtkIdList *idlist;
    for( int elementId = 0; elementId < numOfCells; elementId++ )
    {
      int cellType = input->GetCellType( elementId );
      if ( cellType == VTK_HEXAHEDRON || cellType == VTK_TETRA )
      {
        idlist = input->GetCell( elementId )->GetPointIds( );
        int numOfIds = idlist->GetNumberOfIds( );
        for ( int count = 0; count < numOfIds; count ++ )
        {
          if ( count == 0)
          {
            out << (elementId + this->StartingElementNumber) << ", " << ( idlist->GetId( count )+ this->StartingNodeNumber );
          }
          else
          {
            out << ", " << ( idlist->GetId( count )+ this->StartingNodeNumber );
          }
        }
        out << std::endl;
      }  
    }
        // seperate element sets for disc rings
        double meshseed[3];
        bbox->GetCellData()->GetVectors()->GetTuple(0, meshseed);
        for (int i=0; i<meshseed[2]-1; i++) {
                char name[10];
                strcpy(name, "Ring");
                char buffer[10];
                sprintf(buffer, "%d", i+1);
                strcat(name, buffer);
                out<<"*ELSET,ELSET="<<name<<endl;
                // loop through the first 4 bounding boxes
                double dimtemp[3];
                for (int j=0; j<4; j++) {
                        int startelnum=0;
                        for (int k=0; k<j; k++) {
                                bbox->GetCellData()->GetVectors()->GetTuple(k, dimtemp);
                                startelnum = startelnum + (static_cast<int>(dimtemp[0])-1)*
                                             (static_cast<int>(dimtemp[1])-1) * 
                                             (static_cast<int>(dimtemp[2])-1);
                        }
                        // loop through all the elements of a given face
                        double dim[3];
                        bbox->GetCellData()->GetVectors()->GetTuple(j, dimtemp);
                        dim[0] = dimtemp[2]-1;
                        dim[1] = dimtemp[0]-1;
                        dim[2] = dimtemp[1]-1;

                        for(int y=0; y < dim[2]; y++)
                        {
                                for(int x=0; x < dim[1]; x++)
                                {
                                        out<<startelnum+this->StartingElementNumber+(y*dim[0]*dim[1]+x*dim[0])+i<<endl;
                                }
                        }
                }

        }
        //element set for nucleus pulposus
        out<<"*ELSET,ELSET=NUCLEUSPULPOSUS"<<endl;
        int startelnum=0;
        double dimtemp[3];
        for (int k=0; k<4; k++) {
                bbox->GetCellData()->GetVectors()->GetTuple(k, dimtemp);
                startelnum = startelnum + (static_cast<int>(dimtemp[0])-1) * 
                             (static_cast<int>(dimtemp[1])-1) * 
                             (static_cast<int>(dimtemp[2])-1);
        }
        // skip the first 4 bounding boxes
        for (int i=startelnum; i<input->GetNumberOfCells(); i++) {
                out<<i+this->StartingElementNumber<<endl;
        }
    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << endl;
    out << Modulus << ", " << PoissonsRatio << endl;
  }

  void vtkMimxIntervertebralDiscAbaqusWriter::SetDisc(vtkUnstructuredGrid *Disc)
  {
          this->SetInput(0, Disc);
  }

  void vtkMimxIntervertebralDiscAbaqusWriter::SetDiscBoundingBox(vtkUnstructuredGrid *BBox)
  {
          this->SetInput(1, BBox);
  }

void vtkMimxIntervertebralDiscAbaqusWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
