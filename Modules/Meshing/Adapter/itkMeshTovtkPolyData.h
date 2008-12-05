/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMeshTovtkPolyData.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.2 $

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

#ifndef __itkMeshTovtkPolyData_h__
#define __itkMeshTovtkPolyData_h__

#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkMesh.h"
#include "itkTriangleCell.h"
#include "itkPoint.h"
#include "vtkMimxAdapterWin32Header.h"

/** 
  \class itkMeshTovtkPolyData
  \brief 
    \warning
  \sa 
  */

class VTK_MIMXADAPTER_EXPORT itkMeshTovtkPolyData
{

 public:

  itkMeshTovtkPolyData( void );
  virtual ~itkMeshTovtkPolyData( void );

  typedef itk::DefaultDynamicMeshTraits<double, 3, 3,double,double> TriangleMeshTraits;
  typedef itk::Mesh<double,3, TriangleMeshTraits> TriangleMeshType;
  typedef TriangleMeshType::PointType                       PointType;
  typedef TriangleMeshType::PointsContainer                 InputPointsContainer;
  typedef InputPointsContainer::Pointer            InputPointsContainerPointer;
  typedef InputPointsContainer::Iterator           InputPointsContainerIterator;
  typedef TriangleMeshType::CellType                        CellType; 
  
  typedef TriangleMeshType::CellsContainerPointer           CellsContainerPointer;
  typedef TriangleMeshType::CellsContainerIterator          CellsContainerIterator;
  /**
  The SetInput method provides pointer to the vtkPolyData
  */
  void SetInput(TriangleMeshType::Pointer mesh);
  vtkPolyData * GetOutput();
  void ConvertitkTovtk();

  TriangleMeshType::Pointer m_itkTriangleMesh;

  vtkPoints  * m_Points;
  vtkPolyData * m_PolyData;
  vtkCellArray * m_Polys;
  
};

#endif
