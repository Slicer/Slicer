/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxSurfaceMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.6 $

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

#include "vtkMimxBoundingBoxSurfaceMesh.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkCellArray.h"
#include "vtkCellLocator.h"
#include "vtkPoints.h"

///////////////////////////////////////////////////////////////
vtkCxxRevisionMacro(vtkMimxBoundingBoxSurfaceMesh, "$Revision: 1.6 $");
vtkStandardNewMacro(vtkMimxBoundingBoxSurfaceMesh);

vtkMimxBoundingBoxSurfaceMesh::vtkMimxBoundingBoxSurfaceMesh()
{
        BoundingBoxSurfaceMeshPolyData = vtkPolyData::New();
        BoundingBoxSurfaceMeshPolyDataMapper = vtkPolyDataMapper::New();
        BoundingBoxSurfaceMeshActor = vtkActor::New();
        BoundingBoxPoints = vtkPoints::New();
        BoundingBoxCellArray = vtkCellArray::New();
}

vtkMimxBoundingBoxSurfaceMesh::~vtkMimxBoundingBoxSurfaceMesh()
{
        BoundingBoxSurfaceMeshPolyData->Delete();
        BoundingBoxSurfaceMeshPolyDataMapper->Delete();
        BoundingBoxSurfaceMeshActor->Delete();
}

void vtkMimxBoundingBoxSurfaceMesh::Execute()
{
}

void vtkMimxBoundingBoxSurfaceMesh::CreateSurfaceMesh(vtkPolyData* polydata)
{
        vtkCellArray* cellarray = polydata->GetPolys();
        vtkPoints* points = polydata->GetPoints();
        int count = 0;
        vtkIdType* pts=0;
        vtkIdType t=0;
        cellarray->InitTraversal();
        while(cellarray->GetNextCell(t,pts))
        {
                if(count%6 == 0)
                {
                        points->GetPoint(pts[0],this->x0);      points->GetPoint(pts[1],this->x1);
                        points->GetPoint(pts[2],this->x5);      points->GetPoint(pts[3],this->x4);
                        cellarray->GetNextCell(t,pts);
                        count++;
                        points->GetPoint(pts[0],this->x3);      points->GetPoint(pts[1],this->x2);
                        points->GetPoint(pts[2],this->x6);      points->GetPoint(pts[3],this->x7);
                }
                for(int i=0; i<4; i++)  
                {cellarray->GetNextCell(t,pts);
                count++;}
        }
        this->GenerateNodes();
}

void vtkMimxBoundingBoxSurfaceMesh::GenerateNodes()
{
        this->I_Div = 5;
        this->J_Div = 7;
        this->K_Div = 20;
        double shapex = 1.0/double(this->I_Div);
        double shapey = 1.0/double(this->J_Div);
        double shapez = 1.0/double(this->K_Div);
        double x[3];
        int count = 0;
        for(int k = 0; k<= this->K_Div; k++)
        {
                for(int j = 0; j<= this->J_Div; j++)
                {
                        for(int i = 0; i<= this->I_Div; i++)
                        {
                                double zi,eta,zeta;
                                zi = (2.0*i-this->I_Div)*shapex ;
                                eta = (2.0*j-this->J_Div)*shapey ;
                                zeta =  (2.0*k-this->K_Div)*shapez;
                                double shape[8];
                                shape[0] = (1.0-zi)*(1.0-eta)*(1.0-zeta)/8.0;
                                shape[1] = (1.0-zi)*(1.0-eta)*(1.0+zeta)/8.0;
                                shape[2] = (1.0-zi)*(1.0+eta)*(1.0+zeta)/8.0;
                                shape[3] = (1.0-zi)*(1.0+eta)*(1.0-zeta)/8.0;
                                shape[4] = (1.0+zi)*(1.0-eta)*(1.0-zeta)/8.0;
                                shape[5] = (1.0+zi)*(1.0-eta)*(1.0+zeta)/8.0;
                                shape[6] = (1.0+zi)*(1.0+eta)*(1.0+zeta)/8.0;
                                shape[7] = (1.0+zi)*(1.0+eta)*(1.0-zeta)/8.0;

                                x[0] =  x0[0]*shape[0] + x1[0]*shape[1] + x2[0]*shape[2] + x3[0]*shape[3]
                                        + x4[0]*shape[4] + x5[0]*shape[5] + x6[0]*shape[6] + x7[0]*shape[7];
                                x[1] =  x0[1]*shape[0] + x1[1]*shape[1] + x2[1]*shape[2] + x3[1]*shape[3]
                                        + x4[1]*shape[4] + x5[1]*shape[5] + x6[1]*shape[6] + x7[1]*shape[7];
                                x[2] =  x0[2]*shape[0] + x1[2]*shape[1] + x2[2]*shape[2] + x3[2]*shape[3]
                                        + x4[2]*shape[4] + x5[2]*shape[5] + x6[2]*shape[6] + x7[2]*shape[7];
                                this->BoundingBoxPoints->InsertPoint(count,x);
//                              // cout <<count<<"  "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
                                count++;
                        }
                }
        }

        int node1,node2,node3,node4;
        //      generate element connectivity to all the surface elements
        // face 5
//      // cout <<" Face5"<<endl;
        for(int j=0; j < this->J_Div; j++)
        {
                for(int i=0; i < this->I_Div; i++)
                {
                        node1 = j*(this->I_Div+1)+i; node2 = (j+1)*(this->I_Div+1)+i;
                        node3 = node2+1; node4 = node1 +1;
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                }
        }
        // face 6
        //// cout <<" Face6"<<endl;
        for(int j=0; j < this->J_Div; j++)
        {
                for(int i=0; i < this->I_Div; i++)
                {
                        node1 = (this->K_Div)*(this->J_Div+1)*(this->I_Div+1)+j*(this->I_Div+1)+i; 
                        node2 = node1+1;
                        node3 = (this->K_Div)*(this->J_Div+1)*(this->I_Div+1)+(j+1)*(this->I_Div+1)+i+1; 
                        node4 = node3-1;
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                }
        }

        //// cout <<" Face1"<<endl;
        for(int k=0; k < this->K_Div; k++)
        {
                for(int i=0; i < this->I_Div; i++)
                {
                        node1 = k*(this->I_Div+1)*(this->J_Div+1)+i; 
                        node2 = node1+1;
                        node3 = (k+1)*(this->I_Div+1)*(this->J_Div+1)+i+1; 
                        node4 = node3 -1;
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                }
        }

        //// cout <<" Face2"<<endl;
        for(int k=0; k < this->K_Div; k++)
        {
                for(int i=0; i < this->I_Div; i++)
                {
                        node1 = k*(this->I_Div+1)*(this->J_Div+1)+i+
                                (this->I_Div+1)*this->J_Div; 
                        node2 = node1+1;
                        node3 = (k+1)*(this->I_Div+1)*(this->J_Div+1)
                                +i+1+(this->I_Div+1)*this->J_Div; 
                        node4 = node3 -1;
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                }
        }

        //// cout <<" Face3"<<endl;
        for(int k=0; k < this->K_Div; k++)
        {
                for(int j=0; j < this->J_Div; j++)
                {
                        node1 = k*(this->I_Div+1)*(this->J_Div+1)+j*
                                (this->I_Div+1); 
                        node2 = node1+ (this->I_Div+1)*(this->J_Div+1);                         
                        node3 = node2+ this->I_Div+1;
                        node4 = node1 + (this->I_Div+1);
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                }
        }

        //// cout <<" Face4"<<endl;
        for(int k=0; k < this->K_Div; k++)
        {
                for(int j=0; j < this->J_Div; j++)
                {
                        node1 = k*(this->I_Div+1)*(this->J_Div+1)+j*
                                (this->I_Div+1)+this->I_Div; 
                        node2 = node1+ (this->I_Div+1)*(this->J_Div+1);                         
                        node3 = node2+ this->I_Div+1;
                        node4 = node1 + (this->I_Div+1);
                        //// cout << node1<<"  "<<node2<<"  "<<node3<<"  "<<node4<<endl;
                        this->BoundingBoxCellArray->InsertNextCell(4);
                        this->BoundingBoxCellArray->InsertCellPoint(node1);
                        this->BoundingBoxCellArray->InsertCellPoint(node4);
                        this->BoundingBoxCellArray->InsertCellPoint(node3);
                        this->BoundingBoxCellArray->InsertCellPoint(node2);
                }
        }
        this->BoundingBoxSurfaceMeshPolyData->SetPoints(this->BoundingBoxPoints);
        this->BoundingBoxSurfaceMeshPolyData->SetPolys(this->BoundingBoxCellArray);
        this->BoundingBoxSurfaceMeshPolyDataMapper->SetInput(this->BoundingBoxSurfaceMeshPolyData);
        this->BoundingBoxSurfaceMeshActor->SetMapper(this->BoundingBoxSurfaceMeshPolyDataMapper);
}

void vtkMimxBoundingBoxSurfaceMesh::BoundingBoxClosestPointProjection(vtkMimxSTLSource* STL)
{
        vtkCellLocator *CellLocator = vtkCellLocator::New();
        CellLocator->SetDataSet(STL->GetPolyData());
        CellLocator->BuildLocator();
        double x[3],dist,ClosestPoint[3];
        vtkIdType CellId;
        int SubId;
        for(int i=0; i <this->BoundingBoxPoints->GetNumberOfPoints(); i++)
        {
                this->BoundingBoxPoints->GetPoint(i,x);
                CellLocator->FindClosestPoint(x,ClosestPoint,CellId,SubId,dist);
                this->BoundingBoxPoints->SetPoint(i,ClosestPoint);
        }
        this->BoundingBoxSurfaceMeshPolyData->Modified();
}
