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
#ifndef __vtkUnstructuredGridToitkMesh_h__
#define __vtkUnstructuredGridToitkMesh_h__

#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkHexahedron.h"
#include "vtkTetra.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkMesh.h"
#include "itkAutomaticTopologyMeshSource.h"
#include "itkHexahedronCell.h"
#include "vtkCellArray.h"
#include "vtkMimxAdapterWin32Header.h"


/** 
  \class vtkUnstructuredGridToitkMesh
  \brief 
    \warning
  \sa 
*/



class VTK_MIMXADAPTER_EXPORT vtkUnstructuredGridToitkMesh
{
 public:
  vtkUnstructuredGridToitkMesh( void );
  ~vtkUnstructuredGridToitkMesh( void );

  typedef itk::DefaultStaticMeshTraits<double, 3, 3,float,float> MeshTraits;
  typedef itk::Mesh<double,3, MeshTraits> MeshType;

  /**
  Useful Internal Typedefs
  */
  typedef itk::AutomaticTopologyMeshSource< MeshType >  MeshSourceType;
  typedef MeshSourceType::Pointer  MeshSourceTypePointer;

  /**
  The SetInput method provides pointer to the vtkUnstructuredGrid
  */

  void SetInput( vtkUnstructuredGrid * grid);
  MeshType::Pointer GetOutput();
  void ConvertvtkToitk();

  MeshType::Pointer   m_itkMesh;
  vtkUnstructuredGrid *m_Grid;
};



#endif

