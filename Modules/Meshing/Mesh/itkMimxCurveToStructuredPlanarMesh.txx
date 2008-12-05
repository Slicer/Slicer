/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxCurveToStructuredPlanarMesh.txx,v $
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

#ifndef _itkCurveToStructuredm_PlanarMesh_txx
#define _itkCurveToStructuredm_PlanarMesh_txx
#include "itkCurveToStructuredPlanarMesh.h"
#include "itkExceptionObject.h"
#include "itkMesh.h"
namespace itk
{


template <class TInputMesh, class TOutputMesh>
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::CurveToStructuredPlanarMesh()
{
        this->m_InputMeshPointer = InputMeshPointerContainerVector( 4 );        
        for(int i=0; i <4; i++) this->m_InputMeshPointer[i]=NULL;
        this->m_PlanarMesh = OutputMeshType::New();
}

template <class TInputMesh, class TOutputMesh>
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::~CurveToStructuredPlanarMesh()
{
//      this->m_PlanarMesh->Delete();
}

//      Set the pointers to the four curves which make up the boundary
 template <class TInputMesh, class TOutputMesh>
         void
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::SetCurve(IdentifierType CurveNumber, InputMeshPointer CurvePointer)
{
        this->m_InputMeshPointer[CurveNumber-1] = CurvePointer;
}

//      Get the pointer to the specified curve
template <class TInputMesh, class TOutputMesh>
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>::InputMeshType::Pointer 
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::GetCurve(IdentifierType CurveNumber)
{
        return this->m_InputMeshPointer[CurveNumber-1];
}

//      check the 4 curves that form the boundary are valid?
template <class TInputMesh, class TOutputMesh>
bool 
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::CheckValidity()
{

        if(this->m_InputMeshPointer[0]->GetNumberOfPoints() != this->m_InputMeshPointer[2]->GetNumberOfPoints() ||
                        this->m_InputMeshPointer[1]->GetNumberOfPoints() != this->m_InputMeshPointer[3]->GetNumberOfPoints())   
        {
                ExceptionObject exception(__FILE__, __LINE__);
                exception.SetDescription(
                        "CurveToStructuredPlanarMesh: Number of nodes on the opposite sides do not match up");
                throw exception;
        }

        if(this->m_InputMeshPointer[0]->GetNumberOfPoints() <= 1 || this->m_InputMeshPointer[2]->GetNumberOfPoints() <=1 ||
                this->m_InputMeshPointer[1]->GetNumberOfPoints() <=1 || this->m_InputMeshPointer[3]->GetNumberOfPoints() <=1)
        {
                ExceptionObject exception(__FILE__, __LINE__);
        exception.SetDescription(
                "CurveToStructuredPlanarMesh: Number of nodes cannot be less than 2");
        throw exception;
        }
        return true;
}

//      generate the quadrilateral mesh given four sides.
//      the four curves have to be contiguous
//      transfinite Interpolation used.
 template <class TInputMesh, class TOutputMesh>
bool
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::GenerateQuadrilateralMesh()
{
        if(this->CheckValidity())
        {
                this->DesignateBoundaryNodes();
                this->PlanarTransfiniteInterpolation();
                this->GenerateQuadrilateralMeshConnectivity();
        }
        return true;
}

// store the boundary nodes in the new planar mesh
template <class TInputMesh, class TOutputMesh>
void
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::DesignateBoundaryNodes()
{
        int i,j;
        InputMeshType::PointType x;
        //      copy values from first curve
        for(i=0; i < this->m_InputMeshPointer[0]->GetNumberOfPoints(); i++)
        {
                x = this->m_InputMeshPointer[0]->GetPoint(i)->GetPosition();

                this->SetPoint(i,x);
        }
        //      copy values from second curve
        i = this->m_InputMeshPointer[0]->GetNumberOfPoints();
        for(j=1; j <this->m_InputMeshPointer[1]->GetNumberOfPoints(); j++)
        {
                x = this->m_InputMeshPointer[1]->GetPoint(j)->GetPosition();
                this->SetPoint((j+1)*i-1,x);
        }

        //      copy values from the third curve
        j = this->m_InputMeshPointer[1]->GetNumberOfPoints();
        for(i=1; i < this->m_InputMeshPointer[2]->GetNumberOfPoints(); i++)
        {
                x = this->m_InputMeshPointer[2]->GetPoint(i)->GetPosition();
                        this->SetPoint(j*this->m_InputMeshPointer[0]->GetNumberOfPoints()-(i+1),x);
        }

        //      copy values from the fourth curve
        i = this->m_InputMeshPointer[0]->GetNumberOfPoints();
        for(j=this->m_InputMeshPointer[1]->GetNumberOfPoints()-1; j > 1; j--)
        {
                x = this->m_InputMeshPointer[3]->GetPoint(i)->GetPosition();
                this->SetPoint((j-1)*i,x);
        }
}

// Generate the interior points for 4 boundary curves
template <class TInputMesh, class TOutputMesh>
void
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::PlanarTransfiniteInterpolation()
{
        OutputMeshType::PointType InterpPoint;
        InputMeshType::PointType x;
        int i,j;
        float shapeI, shapeJ;
        shapeI  = 1.0/(this->m_InputMeshPointer[0]->GetNumberOfPoints()-1);
        shapeJ  = 1.0/(this->m_InputMeshPointer[1]->GetNumberOfPoints()-1);
        //      InterpPointolate to calculate interior nodes from boundary nodes
        for(j = 1; j < this->m_InputMeshPointer[1]->GetNumberOfPoints()-1; j++)
        {
                for(i = 1; i < this->m_InputMeshPointer[0]->GetNumberOfPoints()-1; i++)
                {               
                                InterpPoint[0] = 0.0;   InterpPoint[1] = 0.0; InterpPoint[2] = 0.0;
                                x = this->m_InputMeshPointer[0]->GetPoint(i)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] + (1.0 - shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] + (1.0 - shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] + (1.0 - shapeJ*j)*x[2];
                                x = this->m_InputMeshPointer[2]->GetPoint
                                        (this->m_InputMeshPointer[2]->GetNumberOfPoints()-1-i)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] + (shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] + (shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] + (shapeJ*j)*x[2];
                                x = this->m_InputMeshPointer[3]->GetPoint
                                        (this->m_InputMeshPointer[3]->GetNumberOfPoints()-1 - j)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] + (1.0 - shapeI*i)*x[0];
                                InterpPoint[1] = InterpPoint[1] + (1.0 - shapeI*i)*x[1];
                                InterpPoint[2] = InterpPoint[2] + (1.0 - shapeI*i)*x[2];
                                x = this->m_InputMeshPointer[1]->GetPoint(j)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] + (shapeI*i)*x[0];
                                InterpPoint[1] = InterpPoint[1] + (shapeI*i)*x[1];
                                InterpPoint[2] = InterpPoint[2] + (shapeI*i)*x[2];
                                x = this->m_InputMeshPointer[0]->GetPoint(0)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] - (1.0 - shapeI*i)*(1.0 - shapeJ*j)*x[2];
                                x = this->m_InputMeshPointer[0]->GetPoint
                                        (this->m_InputMeshPointer[0]->GetNumberOfPoints()-1)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] - (shapeI*i)*(1.0 - shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] - (shapeI*i)*(1.0 - shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] - (shapeI*i)*(1.0 - shapeJ*j)*x[2];
                                x = this->m_InputMeshPointer[1]->GetPoint
                                        (this->m_InputMeshPointer[1]->GetNumberOfPoints()-1)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] - (shapeI*i)*(shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] - (shapeI*i)*(shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] - (shapeI*i)*(shapeJ*j)*x[2];
                                x = this->m_InputMeshPointer[2]->GetPoint(
                                        this->m_InputMeshPointer[2]->GetNumberOfPoints()-1)->GetPosition();
                                InterpPoint[0] = InterpPoint[0] - (1.0 - shapeI*i)*(shapeJ*j)*x[0];
                                InterpPoint[1] = InterpPoint[1] - (1.0 - shapeI*i)*(shapeJ*j)*x[1];
                                InterpPoint[2] = InterpPoint[2] - (1.0 - shapeI*i)*(shapeJ*j)*x[2];
                                this->m_PlanarMesh->SetPoint(
                                        j*this->m_InputMeshPointer[0]->GetNumberOfPoints()+i,InterpPoint);
//                              cout <<j*this->m_InputMeshPointer[0]->GetNumberOfPoints()+i<<"  "<<
//                                      InterpPoint[0]<<"  "<<InterpPoint[1]<<"  "<<InterpPoint[2]<<std::endl;
                }
        }
}

template <class TInputMesh, class TOutputMesh>
void
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::SetPoint(IdentifierType PointNumber, CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>::InputMeshType::PointType InputPoint)
{
        OutputMeshType::PointType OutputPoint;
        for(int i=0; i<TOutputMesh::PointType::PointDimension; i++)     OutputPoint[i] = InputPoint[i];
        this->m_PlanarMesh->SetPoint(PointNumber,OutputPoint);
}

//      generate mesh connectivity from the nodes
template <class TInputMesh, class TOutputMesh>
void
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>
::GenerateQuadrilateralMeshConnectivity()
{
        int i,j;
        Array<IdentifierType> pointIDs(4);
        CellAutoPointer newCell;
        for(j = 0; j < this->m_InputMeshPointer[1]->GetNumberOfPoints()-1; j++)
        {
                for(i = 0; i < this->m_InputMeshPointer[0]->GetNumberOfPoints()-1; i++)
                {
                        pointIDs[0] = j*this->m_InputMeshPointer[0]->GetNumberOfPoints()+i;
                        pointIDs[1] = pointIDs[0]+1;
                        pointIDs[2] = pointIDs[1]+this->m_InputMeshPointer[0]
                                                        ->GetNumberOfPoints();
                        pointIDs[3] = pointIDs[2]-1;
                        this->AddQuadrilateral(pointIDs);
        //              cout << pointIDs[0]<<"  "<<pointIDs[1]<<"  "
        //                      <<pointIDs[2]<<"  "<<pointIDs[3]<<std::endl;
                }
        }
}

// add quadrilateral cells 
template <class TInputMesh, class TOutputMesh>
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh>::IdentifierType
CurveToStructuredPlanarMesh<TInputMesh,TOutputMesh> 
::AddQuadrilateral( const IdentifierArrayType& pointIDs )
{

                // Construct the cell.
                CellAutoPointer newCell;
                newCell.TakeOwnership( new QuadrilateralCell );
                IdentifierType cellID;
                int i;
                // Add the points and vertices, keeping track of the vertex IDs.
                IdentifierArrayType vertexArray( 4 );
                for( i = 0; i < 4; i++ )
                {
                        IdentifierType pointID = pointIDs[i];
                        newCell->SetPointId( i, pointID );
                }

                cellID = this->m_PlanarMesh->GetNumberOfCells();

                // Add the cell to the mesh.
                this->m_PlanarMesh->SetCell( cellID, newCell ); 
        return cellID;
}

}
#endif
