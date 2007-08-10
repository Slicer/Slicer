/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAbaqusFileWriter.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.8 $

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

#include "vtkMimxAbaqusFileWriter.h"

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


vtkCxxRevisionMacro(vtkMimxAbaqusFileWriter, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkMimxAbaqusFileWriter);

vtkMimxAbaqusFileWriter::vtkMimxAbaqusFileWriter()
{
  Modulus = 200000;
  PoissonsRatio = 0.35;
//  this->StartingElementNumber = 1;
//  this->StartingNodeNumber = 1;
}

vtkMimxAbaqusFileWriter::~vtkMimxAbaqusFileWriter()
{
}


//----------------------------------------------------------------------------
int vtkMimxAbaqusFileWriter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
  {
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkDataSet *input = vtkDataSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));

    if(input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    { 
    if(vtkUnstructuredGrid::SafeDownCast(input)->GetPointData()->GetArray("Node_Numbers") &&
      vtkUnstructuredGrid::SafeDownCast(input)->GetCellData()->GetArray("Element_Numbers"))
    {
    this->UnstGridExecute( vtkUnstructuredGrid::SafeDownCast( input ) );
    }
    else{
      vtkErrorMacro("Arrays containing node and element numbers not available");
      return 0;
    }
    }

 /*   if(input->GetDataObjectType() == VTK_POLY_DATA )
    {
      this->PolyDataExecute( vtkPolyData::SafeDownCast( input ) );
    }
  if(input->GetDataObjectType() == VTK_STRUCTURED_GRID )
  {
    this->StructuredGridExecute( vtkStructuredGrid::SafeDownCast( input ) );
  }*/

    return 1;
  }

  void vtkMimxAbaqusFileWriter::UnstGridExecute( vtkUnstructuredGrid *input )
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int numOfCells = input->GetNumberOfCells( );
    //
    std::ofstream out;  
    out.open(this->FileName, std::ios::out );
    //
    ///* Writing out Nodal Definitions */
    //out << "*NODE" << endl;
    //double point[3];
    //for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    //{
    //  input->GetPoint( pointIndex, point );
    //  out << ( pointIndex + this->StartingNodeNumber ) << ", " << point[0] << ", " << point[1] << ", " << point[2] <<std::endl;
    //}

    ///* Writing out Element Definitions */
    //out << "** \n** \n*ELEMENT, TYPE=, ELSET=" << endl;
    //vtkIdList *idlist;
    //for( int elementId = 0; elementId < numOfCells; elementId++ )
    //{
    //  int cellType = input->GetCellType( elementId );
    //  if ( cellType == VTK_HEXAHEDRON || cellType == VTK_TETRA )
    //  {
    //    idlist = input->GetCell( elementId )->GetPointIds( );
    //    int numOfIds = idlist->GetNumberOfIds( );
    //    for ( int count = 0; count < numOfIds; count ++ )
    //    {
    //      if ( count == 0)
    //      {
    //        out << (elementId + this->StartingElementNumber) << ", " << ( idlist->GetId( count )+ this->StartingNodeNumber );
    //      }
    //      else
    //      {
    //        out << ", " << ( idlist->GetId( count )+ this->StartingNodeNumber );
    //      }
    //    }
    //    out << std::endl;
    //  }  
    //}
    //out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << endl;
    //out << Modulus << ", " << PoissonsRatio << endl;
    vtkIntArray *nodearray = vtkIntArray::SafeDownCast(input->GetPointData()->GetArray("Node_Numbers"));
    vtkIntArray *elementarray = vtkIntArray::SafeDownCast(input->GetCellData()->GetArray("Element_Numbers"));
    // node definitions
    out << "*NODE" << endl;
    double point[3];
    for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    {
      input->GetPoint( pointIndex, point );
      out << nodearray->GetValue(pointIndex) << ", " << point[0] << ", " << point[1] << ", " << point[2] <<std::endl;
    }
    /* Writing out Element Definitions */
    // hex element
    vtkIdList *idlist;
    int countelement = 0;
    for( int elementId = 0; elementId < numOfCells; elementId++ )
    {
      int cellType = input->GetCellType( elementId );
      if ( cellType == VTK_HEXAHEDRON )
      {
        countelement ++;
        if(countelement == 1)      out << "** \n** \n*ELEMENT, TYPE=C3D8, ELSET=" << endl;
        idlist = input->GetCell( elementId )->GetPointIds( );
        int numOfIds = idlist->GetNumberOfIds( );
        for ( int count = 0; count < numOfIds; count ++ )
        {
          if ( count == 0)
          {
            out << elementarray->GetValue(elementId) << ", " << ( nodearray->GetValue(idlist->GetId(count)));
          }
          else
          {
            out << ", " << ( nodearray->GetValue(idlist->GetId(count)));
          }
        }
        out << std::endl;
      }  
    }
    // for tetrahedral elements
    countelement = 0;
    for( int elementId = 0; elementId < numOfCells; elementId++ )
    {
      int cellType = input->GetCellType( elementId );
      if ( cellType == VTK_TETRA )
      {
        countelement ++;
        if(countelement == 1)  out << "** \n** \n*ELEMENT, TYPE=C3D4, ELSET=" << endl;
        idlist = input->GetCell( elementId )->GetPointIds( );
        int numOfIds = idlist->GetNumberOfIds( );
        for ( int count = 0; count < numOfIds; count ++ )
        {
          if ( count == 0)
          {
            out << elementarray->GetValue(elementId) << ", " << ( nodearray->GetValue(idlist->GetId(count)));
          }
          else
          {
            out << ", " << ( nodearray->GetValue(idlist->GetId(count)));
          }
        }
        out << std::endl;
      }  
    }
    // element sets
    //vtkIntArray *elementsets;
    //int count = 0;
    //do {
     // count ++;
     // elementsets = NULL;
     // char name[20];
     // strcpy(name, "Element_Set_");
     // char buffer[20];
     // sprintf(buffer, "%d", count);
     // strcat(name, buffer);
     // elementsets = vtkIntArray::SafeDownCast(input->GetCellData()->GetArray(name));
     // if(elementsets)
     // {
      //  out<<"*ELSET,ELSET="<<name<<endl;
      //  for (int elementId = 0; elementId < numOfCells; elementId++) {
      //    if(elementsets->GetValue(elementId))
      //    {
      //      out<<elementarray->GetValue(elementId)<<endl;
      //    }
      //  }
     // }
    //} while(elementsets);
    vtkIntArray *elementsets;
    int i;
    for (i=0; i<input->GetCellData()->GetNumberOfArrays(); i++) {
      if(strcmp(input->GetCellData()->GetArrayName(i), "Element_Numbers"))
      {
      if( strcmp(input->GetCellData()->GetArrayName(i), "Fe_Mesh"))
      {
        elementsets = vtkIntArray::SafeDownCast(input->GetCellData()->GetArray(i));
        out<<"*ELSET,ELSET="<<input->GetCellData()->GetArrayName(i)<<endl;
        for (int elementId = 0; elementId < numOfCells; elementId++) {
            if(elementsets->GetValue(elementId))
            {
              out<<elementarray->GetValue(elementId)<<endl;
            }
        }
      }
      }
    }

    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << endl;
    out << Modulus << ", " << PoissonsRatio << endl;

  }

  void vtkMimxAbaqusFileWriter::PolyDataExecute( vtkPolyData *input )
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int numOfCells = input->GetNumberOfCells( );

    vtkCellArray *cellArray = input->GetPolys();
    
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

    vtkIdType elementId;
    vtkIdType *points = 0;
    cellArray->InitTraversal( );
    int j = 0;
    int maxCellSize =  cellArray->GetMaxCellSize();
    while ( cellArray->GetNextCell( elementId, points) )
    {
      for ( int count = 0; count < elementId; count ++ )
      {
        if ( count == 0)
        {
          out << (j + this->StartingElementNumber ) << ", " << ( points[count] + this->StartingNodeNumber );
        }
        else
        {
          out << ", " << ( points[count] + this->StartingNodeNumber );
        }
      }
      out << std::endl;
      j++;
    }  
  
    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << endl;
    out << Modulus << ", " << PoissonsRatio << endl;
  }

  void vtkMimxAbaqusFileWriter::StructuredGridExecute(vtkStructuredGrid *input)
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int dim[3];
    input->GetDimensions(dim);
    int numOfCells = (dim[0]-1)*(dim[1]-1)*(dim[2]-1);

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
    int ElementNumber = this->StartingElementNumber;
    for (int k=0; k < dim[2]-1; k++) {
      for (int j=0; j < dim[1]-1; j++) {
        for (int i=0; i < dim[0]-1; i++) {
          int firstnode = k*dim[0]*dim[1]+j*dim[0]+i + this->StartingNodeNumber ;
          int secondnode = firstnode + dim[0]*dim[1];
          out << ElementNumber++<<",  "<<firstnode <<",  "<<firstnode+1 <<",  "
            <<firstnode+1+dim[0]<<",  "<<firstnode+dim[0]<<",  "<<secondnode <<
            ",  "<<secondnode+1 <<",  "<<secondnode+1+dim[0]<<",  "<<secondnode+dim[0]<<std::endl;
        }
      }
    }
    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << endl;
    out << Modulus << ", " << PoissonsRatio << endl;

  }
void vtkMimxAbaqusFileWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
