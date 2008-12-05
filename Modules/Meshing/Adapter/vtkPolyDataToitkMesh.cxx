/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkPolyDataToitkMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
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

#include <iostream>
#include "vtkPolyDataToitkMesh.h"

#ifndef vtkDoubleType
#define vtkDoubleType double
#endif

#ifndef vtkFloatingPointType
# define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#endif

vtkPolyDataToitkMesh
::vtkPolyDataToitkMesh()
{

  m_itkMesh = TriangleMeshType::New();
  m_PolyData = vtkPolyData::New();
  
}


vtkPolyDataToitkMesh
::~vtkPolyDataToitkMesh()
{
  if (m_PolyData)
    {
      m_PolyData->Delete();
    }
}

void
vtkPolyDataToitkMesh
::SetInput(vtkPolyData * polydata)
{
  m_PolyData = polydata;
  this->ConvertvtkToitk();
}

vtkPolyDataToitkMesh::TriangleMeshType *
vtkPolyDataToitkMesh
::GetOutput()
{
  return m_itkMesh;
}

void
vtkPolyDataToitkMesh
::ConvertvtkToitk()
{
  //
  // Transfer the points from the vtkPolyData into the itk::Mesh
  //
  const unsigned int numberOfPoints = m_PolyData->GetNumberOfPoints();
  vtkPoints * vtkpoints =  m_PolyData->GetPoints();
 
  m_itkMesh->GetPoints()->Reserve( numberOfPoints );
  
  for(int p =0; p < numberOfPoints; p++)
    {

    vtkFloatingPointType * apoint = vtkpoints->GetPoint( p );
    
    // Need to convert the point to PoinType
    TriangleMeshType::PointType pt;
    for(unsigned int i=0;i<3; i++)
      {
       pt[i] = apoint[i];
       }
     m_itkMesh->SetPoint( p, pt);

    }
  //
  // Transfer the cells from the vtkPolyData into the itk::Mesh
  //
  vtkCellArray * triangleStrips = m_PolyData->GetStrips();

  vtkIdType  * cellPoints;
  vtkIdType    numberOfCellPoints;

  //
  // First count the total number of triangles from all the triangle strips.
  //
  unsigned int numberOfTriangles = 0;

  triangleStrips->InitTraversal();
  while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
    {
    numberOfTriangles += numberOfCellPoints-2;
    }

   vtkCellArray * polygons = m_PolyData->GetPolys();
  
   polygons->InitTraversal();

   while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     if( numberOfCellPoints == 3 )
       {
        numberOfTriangles ++;
       }
     }

   //
  // Reserve memory in the itk::Mesh for all those triangles
  //
   m_itkMesh->GetCells()->Reserve( numberOfTriangles );

  // 
  // Copy the triangles from vtkPolyData into the itk::Mesh
  //
  //

   typedef TriangleMeshType::CellType   CellType;

   typedef itk::TriangleCell< CellType > TriangleCellType;

  // first copy the triangle strips
   int cellId = 0;
   triangleStrips->InitTraversal();
   while( triangleStrips->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     unsigned int numberOfTrianglesInStrip = numberOfCellPoints - 2;

     unsigned long pointIds[3];
     pointIds[0] = cellPoints[0];
     pointIds[1] = cellPoints[1];
     pointIds[2] = cellPoints[2];
       
     for( unsigned int t=0; t < numberOfTrianglesInStrip; t++ )
       {
        TriangleMeshType::CellAutoPointer c;
        TriangleCellType * tcell = new TriangleCellType;
        tcell->SetPointIds( pointIds );
        c.TakeOwnership( tcell );
        m_itkMesh->SetCell( cellId, c );
        cellId++;
        pointIds[0] = pointIds[1];
        pointIds[1] = pointIds[2];
        pointIds[2] = cellPoints[t+3];
       }
       
     }

   // then copy the triangles 
   polygons->InitTraversal();
   while( polygons->GetNextCell( numberOfCellPoints, cellPoints ) )
     {
     if( numberOfCellPoints !=3 ) // skip any non-triangle.
       {
       continue;
       }
     TriangleMeshType::CellAutoPointer c;
     TriangleCellType * t = new TriangleCellType;
     t->SetPointIds( (unsigned long*)cellPoints );
     c.TakeOwnership( t );
     m_itkMesh->SetCell( cellId, c );
     cellId++;
     } 
  
}
