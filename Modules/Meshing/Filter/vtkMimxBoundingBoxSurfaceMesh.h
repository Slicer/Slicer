/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxSurfaceMesh.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
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

#ifndef _vtkMimxBoundingBoxSurfaceMesh_h
#define _vtkMimxBoundingBoxSurfaceMesh_h

#include "vtkFilter.h"

#include "vtkPolyDataSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkMimxSTLSource.h"

class vtkPoints;
class vtkCellArray;

//        class for storing polydata generated from vtkSTLReader
class VTK_MIMXFILTER_EXPORT vtkMimxBoundingBoxSurfaceMesh : public vtkPolyDataSource
{
public:
        vtkTypeRevisionMacro(vtkMimxBoundingBoxSurfaceMesh,vtkPolyDataSource);
        static vtkMimxBoundingBoxSurfaceMesh *New();
        void Execute();
        void CreateSurfaceMesh(vtkPolyData* polydata);
        vtkMimxBoundingBoxSurfaceMesh();
        ~vtkMimxBoundingBoxSurfaceMesh();
        vtkPolyData* BoundingBoxSurfaceMeshPolyData;
        vtkPolyDataMapper* BoundingBoxSurfaceMeshPolyDataMapper;
        vtkActor* BoundingBoxSurfaceMeshActor;
        void BoundingBoxClosestPointProjection(vtkMimxSTLSource* STL);
        int I_Div,J_Div, K_Div;
private:
        void GenerateNodes();
        double x0[3],x1[3],x2[3],x3[3],x4[3],x5[3],x6[3],x7[3];
        vtkPoints* BoundingBoxPoints;
        vtkCellArray* BoundingBoxCellArray;
protected:

};

#endif
