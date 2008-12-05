/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMeshTovtkUnstructuredGrid.txx,v $
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

#ifndef __itkMeshTovtkUnstructuredGrid_txx
#define __itkMeshTovtkUnstructuredGrid_txx

#include <iostream>
#include "itkMeshTovtkUnstructuredGrid.h"


#ifndef vtkDoubleType
#define vtkDoubleType double
#endif

#ifndef vtkFloatingPointType
# define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#endif

template <class TInputMesh>
itkMeshTovtkUnstructuredGrid<TInputMesh>
::itkMeshTovtkUnstructuredGrid()
{
  m_Points = vtkPoints::New();
  m_Grid = vtkUnstructuredGrid::New();
}

template <class TInputMesh>
itkMeshTovtkUnstructuredGrid<TInputMesh>
::~itkMeshTovtkUnstructuredGrid()
{
  
}

template <class TInputMesh>
void
itkMeshTovtkUnstructuredGrid<TInputMesh>
::SetInput(InputMeshTypePointer mesh)
{
  m_itkMesh = mesh;
  this->ConvertitkTovtk();
}

template <class TInputMesh>
vtkUnstructuredGrid *
itkMeshTovtkUnstructuredGrid<TInputMesh>
::GetOutput()
{
  return m_Grid;
}

template <class TInputMesh>
void
itkMeshTovtkUnstructuredGrid<TInputMesh>
::ConvertitkTovtk()
{
  int numPoints =  m_itkMesh->GetNumberOfPoints();

  InputPointsContainerPointer      myPoints = m_itkMesh->GetPoints();
  InputPointsContainerIterator     points = myPoints->Begin();
  PointType point;
  
  //Reset Grid - The Input may have changed
  m_Grid->Reset();
  
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
    m_Points->SetPoint(idx,vpoint);
    idx++;
    points++;
    }

  m_Grid->SetPoints(m_Points);

  m_Points->Delete();

  CellsContainerPointer cells = m_itkMesh->GetCells();
  CellsContainerIterator cellIt = cells->Begin();
  
  while ( cellIt != cells->End() )
    {
    CellType *nextCell = cellIt->Value();
    PointIdIteratorType pointIt = nextCell->PointIdsBegin() ;
    PointType  p;
    int i;
    
    /* Currently this Adapter Only supports Hex and Tet Meshes */
    switch (nextCell->GetType())
      {
      case CellType::HEXAHEDRON_CELL:
        {
        /* This will need to be changed if the HexMesh Generation Changes */
        vtkIdType pts[8];
        i=0;
        while (pointIt != nextCell->PointIdsEnd() ) 
          {
          pts[i++] = *pointIt++;  
          }
        m_Grid->InsertNextCell(VTK_HEXAHEDRON, 8, pts);
        }
        break;
      case CellType::TETRAHEDRON_CELL:
        {
        vtkIdType pts[4];
        i=0;
        while (pointIt != nextCell->PointIdsEnd() ) 
          {
          pts[i++] = *pointIt++;  
          }
        m_Grid->InsertNextCell(VTK_TETRA, 4, pts);
        }      
      }
    cellIt++;
    
    }

}

#endif
