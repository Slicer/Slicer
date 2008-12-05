/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMeshTovtkPolyData.cxx,v $
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
#include "itkMeshTovtkPolyData.h"

#ifndef vtkDoubleType
#define vtkDoubleType double
#endif

/////////////// VTK Version Compatibility   //////////////////////////////
#ifndef vtkFloatingPointType
# define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#endif
//////////////////////////////////////////////////////////////////////////


itkMeshTovtkPolyData
::itkMeshTovtkPolyData()
{

  m_itkTriangleMesh = TriangleMeshType::New();
  m_Points = vtkPoints::New();
  m_PolyData = vtkPolyData::New();
  m_Polys = vtkCellArray::New();
}


itkMeshTovtkPolyData
::~itkMeshTovtkPolyData()
{
  
}

void
itkMeshTovtkPolyData
::SetInput(TriangleMeshType::Pointer mesh)
{
  m_itkTriangleMesh = mesh;
  this->ConvertitkTovtk();
}

vtkPolyData *
itkMeshTovtkPolyData
::GetOutput()
{
  return m_PolyData;
}

void
itkMeshTovtkPolyData
::ConvertitkTovtk()
{
  int numPoints =  m_itkTriangleMesh->GetNumberOfPoints();

  InputPointsContainerPointer      myPoints = m_itkTriangleMesh->GetPoints();
  InputPointsContainerIterator     points = myPoints->Begin();
  PointType point;
  
  if (numPoints == 0)
    {
      printf( "Aborting: No Points in GRID\n");
      return; 
    }

  m_Points->SetNumberOfPoints(numPoints);
  
  int idx=0;
  vtkFloatingPointType vpoint[3];
  while( points != myPoints->End() ) 
    {   
    point = points.Value();
    vpoint[0]= point[0];
    vpoint[1]= point[1];
    vpoint[2]= point[2];
    m_Points->SetPoint(idx++,vpoint);
    points++;
    }

  m_PolyData->SetPoints(m_Points);

  m_Points->Delete();

  CellsContainerPointer cells = m_itkTriangleMesh->GetCells();
  CellsContainerIterator cellIt = cells->Begin();
  vtkIdType pts[3];
  while ( cellIt != cells->End() )
    {
  CellType *nextCell = cellIt->Value();
    CellType::PointIdIterator pointIt = nextCell->PointIdsBegin() ;
    PointType  p;
    int i;
    
    switch (nextCell->GetType())
      {
      case CellType::VERTEX_CELL:
      case CellType::LINE_CELL:
      case CellType::POLYGON_CELL:
        break;        
      case CellType::TRIANGLE_CELL:
        i=0;
        while (pointIt != nextCell->PointIdsEnd() ) 
        {
        pts[i++] = *pointIt++;  
        }
        m_Polys->InsertNextCell(3,pts);
        break;
      default:
        printf("something \n");
      }
    cellIt++;
    
    }
  
  m_PolyData->SetPolys(m_Polys);
  m_Polys->Delete();

}
