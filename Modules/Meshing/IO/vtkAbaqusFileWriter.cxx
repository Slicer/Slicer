/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkAbaqusFileWriter.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAbaqusFileWriter.h"

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
vtkCxxRevisionMacro(vtkAbaqusFileWriter, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkAbaqusFileWriter);

vtkAbaqusFileWriter::vtkAbaqusFileWriter()
{
  Modulus = 200000;
  PoissonsRatio = 0.35;
  AbaqusHeader = "";
//  this->StartingElementNumber = 1;
//  this->StartingNodeNumber = 1;
}

vtkAbaqusFileWriter::~vtkAbaqusFileWriter()
{
}


void vtkAbaqusFileWriter::SetAbaqusHeader( char *header)
{
  AbaqusHeader = header;
}

void vtkAbaqusFileWriter::SetAbaqusHeader( std::string header)
{
  AbaqusHeader = header;
}


//----------------------------------------------------------------------------
int vtkAbaqusFileWriter::RequestData(
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
      if (vtkUnstructuredGrid::SafeDownCast(input)->GetPointData()->GetArray("Node_Numbers") &&
          vtkUnstructuredGrid::SafeDownCast(input)->GetCellData()->GetArray("Element_Numbers"))
        {
        this->UnstGridExecute( vtkUnstructuredGrid::SafeDownCast( input ) );
        }
      else
        {
        vtkErrorMacro("Arrays containing node and element numbers not available");
        return 0;
        }
      }

    return 1;
  }

  void vtkAbaqusFileWriter::UnstGridExecute( vtkUnstructuredGrid *input )
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int numOfCells = input->GetNumberOfCells( );

    /* Output File */
    std::ofstream out;  
    out.open(this->FileName, std::ios::out );

    vtkIntArray *nodearray = vtkIntArray::SafeDownCast(input->GetPointData()->GetArray("Node_Numbers"));
    vtkIntArray *elementarray =
        vtkIntArray::SafeDownCast(input->GetCellData()->GetArray("Element_Numbers"));
    // node definitions
    if (AbaqusHeader.length() > 0)
    {
      out << AbaqusHeader.c_str() << std::endl;
    }
    out << "*NODE" << std::endl;
    double point[3];
    for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    {
      input->GetPoint( pointIndex, point );
      out << nodearray->GetValue(pointIndex) << ", ";
      out << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
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
        if(countelement == 1)          
        {
          out << "** \n** \n*ELEMENT, TYPE=C3D8, ELSET=" << std::endl;
        }
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
        if (countelement == 1)
        {
          out << "** \n** \n*ELEMENT, TYPE=C3D4, ELSET=" << std::endl;
        }
        idlist = input->GetCell( elementId )->GetPointIds( );
        int numOfIds = idlist->GetNumberOfIds( );
        for ( int count = 0; count < numOfIds; count ++ )
        {
          if ( count == 0)
          {
            out << elementarray->GetValue(elementId) << ", ";
            out << ( nodearray->GetValue(idlist->GetId(count)));
          }
          else
          {
            out << ", " << ( nodearray->GetValue(idlist->GetId(count)));
          }
        }
        out << std::endl;
      }
    }

    vtkIntArray *elementsets;
    int i;
    for (i=0; i<input->GetCellData()->GetNumberOfArrays(); i++) 
    {
      if (strcmp(input->GetCellData()->GetArrayName(i), "Element_Numbers"))
      {
        elementsets = vtkIntArray::SafeDownCast(input->GetCellData()->GetArray(i));
        out << "*ELSET,ELSET=" << input->GetCellData()->GetArrayName(i) << std::endl;
        for (int elementId = 0; elementId < numOfCells; elementId++) 
        {
          if (elementsets->GetValue(elementId))
          {
            out << elementarray->GetValue(elementId) << std::endl;
          }
        }
      }
    }

    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << std::endl;
    out << Modulus << ", " << PoissonsRatio << std::endl;

  }

  void vtkAbaqusFileWriter::PolyDataExecute( vtkPolyData *input )
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int numOfCells = input->GetNumberOfCells( );

    vtkCellArray *cellArray = input->GetPolys();

    std::ofstream out;  
    out.open(this->FileName, std::ios::out );

    /* Writing out Nodal Definitions */
    out << "*NODE" << std::endl;
    double point[3];
    for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    {
      input->GetPoint( pointIndex, point );
      out << ( pointIndex + this->StartingNodeNumber ) << ", " << point[0] << ", " << point[1] << ", " << point[2] <<std::endl;
    }

    /* Writing out Element Definitions */
    out << "** \n** \n*ELEMENT, TYPE=, ELSET=" << std::endl;

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
          out << (j + this->StartingElementNumber ) << ", " 
              << ( points[count] + this->StartingNodeNumber );
        }
        else
        {
          out << ", " << ( points[count] + this->StartingNodeNumber );
        }
      }
      out << std::endl;
      j++;
    }

    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << std::endl;
    out << Modulus << ", " << PoissonsRatio << std::endl;
  }

  void vtkAbaqusFileWriter::StructuredGridExecute(vtkStructuredGrid *input)
  {
    int numOfPoints = input->GetNumberOfPoints( );
    int dim[3];
    input->GetDimensions(dim);
    int numOfCells = (dim[0]-1)*(dim[1]-1)*(dim[2]-1);

    std::ofstream out;  
    out.open(this->FileName, std::ios::out );

    /* Writing out Nodal Definitions */
    out << "*NODE" << std::endl;
    double point[3];
    for( int pointIndex = 0; pointIndex < numOfPoints; pointIndex++ )
    {
      input->GetPoint( pointIndex, point );
      out << ( pointIndex + this->StartingNodeNumber ) << ", " 
              << point[0] << ", " << point[1] << ", " << point[2] <<std::endl;
    }

    /* Writing out Element Definitions */
    out << "** \n** \n*ELEMENT, TYPE=, ELSET=" << std::endl;
    int ElementNumber = this->StartingElementNumber;
    for (int k=0; k < dim[2]-1; k++) 
    {
      for (int j=0; j < dim[1]-1; j++) 
      {
        for (int i=0; i < dim[0]-1; i++) 
        {
          int firstnode = k*dim[0]*dim[1]+j*dim[0]+i + this->StartingNodeNumber ;
          int secondnode = firstnode + dim[0]*dim[1];
          out << ElementNumber++ << ",  " << firstnode << ",  " << firstnode+1 << ",  "
              << firstnode+1+dim[0]<<",  " << firstnode+dim[0] << ",  " << secondnode 
              << ",  " << secondnode+1 << ",  " << secondnode+1+dim[0] << ","
              <<secondnode+dim[0]<<std::endl;
        }
      }
    }
    out << "** \n** \n*SOLID SECTION, ELSET=, MATERIAL= \n*MATERIAL, NAME= \n*ELASTIC" << std::endl;
    out << Modulus << ", " << PoissonsRatio << std::endl;

  }
void vtkAbaqusFileWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
