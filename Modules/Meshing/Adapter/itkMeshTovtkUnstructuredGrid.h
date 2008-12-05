/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMeshTovtkUnstructuredGrid.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.7 $

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

#ifndef __itkMeshTovtkUnstructuredGrid_h__
#define __itkMeshTovtkUnstructuredGrid_h__

#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkHexahedron.h"
#include "vtkTetra.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkMesh.h"
#include "itkPoint.h"


/** 
  \class itkMeshTovtkUnstructuredGrid
  \brief 
    \warning
  \sa 
  */

template<class TInputMesh> class itkMeshTovtkUnstructuredGrid
{

 public:

  itkMeshTovtkUnstructuredGrid( void );
  virtual ~itkMeshTovtkUnstructuredGrid( void );

  /** Input Mesh typedefs */ 
  typedef TInputMesh InputMeshType;
  typedef typename InputMeshType::Pointer       InputMeshTypePointer;
  typedef typename InputMeshType::CellType      CellType;
  typedef typename InputMeshType::PointType     PointType;
  typedef typename InputMeshType::CellPixelType CellPixelType;  
  typedef typename InputMeshType::CellsContainer::ConstIterator  CellIterator;

  typedef typename InputMeshType::Pointer         MeshPointer;
  typedef typename InputMeshType::CellTraits      MeshTraits;

  typedef typename InputMeshType::PointsContainer InputPointsContainer;
  typedef typename InputPointsContainer::Pointer  InputPointsContainerPointer;
  typedef typename InputPointsContainer::Iterator InputPointsContainerIterator;
  typedef typename InputMeshType::CellsContainerPointer CellsContainerPointer;
  typedef typename InputMeshType::CellsContainerIterator CellsContainerIterator;
  typedef typename CellType::PointIdIterator      PointIdIteratorType;
/*
  typedef itk::DefaultStaticMeshTraits<double, 3, 3,float,float> MeshTraits;
  typedef itk::Mesh<double,3, MeshTraits>                           MeshType;
  typedef MeshType::PointType                                       PointType;
  typedef MeshType::PointsContainer                                 InputPointsContainer;
  typedef InputPointsContainer::Pointer                             InputPointsContainerPointer;
  typedef InputPointsContainer::Iterator                            InputPointsContainerIterator;
  typedef MeshType::CellType                                        CellType; 
  
  typedef MeshType::CellsContainerPointer                           CellsContainerPointer;
  typedef MeshType::CellsContainerIterator                          CellsContainerIterator;
*/  
  /**
  The SetInput method provides pointer to the vtkPolyData
  */
  void SetInput(InputMeshTypePointer mesh);
  vtkUnstructuredGrid * GetOutput();
  void ConvertitkTovtk();

  InputMeshTypePointer m_itkMesh;

  vtkPoints           * m_Points;
  vtkUnstructuredGrid * m_Grid;
  
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMeshTovtkUnstructuredGrid.txx"
#endif

#endif
