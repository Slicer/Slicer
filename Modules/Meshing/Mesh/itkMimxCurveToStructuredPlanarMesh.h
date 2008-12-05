/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxCurveToStructuredPlanarMesh.h,v $
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

#ifndef __itkCurveToStructuredPlanarMesh_h
#define __itkCurveToStructuredPlanarMesh_h

#include "itkMeshToMeshFilter.h"
#include "itkPolygon3DSpatialObject.h"
#include <vector>
#include "itkMesh.h"
#include "itkQuadrilateralCell.h"

namespace itk
{

        //CurveToStructuredPlanarMesh
        template <class TInputMesh, class TOutputMesh>
        class ITK_EXPORT CurveToStructuredPlanarMesh : public MeshToMeshFilter<TInputMesh,TOutputMesh>
        {
        public:
                /** Standard class typedefs. */
                typedef CurveToStructuredPlanarMesh  Self;
                typedef MeshToMeshFilter<TInputMesh,TOutputMesh>  Superclass;
                typedef SmartPointer<Self>  Pointer;
                typedef SmartPointer<const Self>  ConstPointer;
                itkNewMacro(Self);
                
                /** Run-time type information (and related methods). */
                itkTypeMacro(CurveToStructuredPlanarMesh, MeshToMeshFilter);

                /** Some Mesh related typedefs. */
                typedef   TInputMesh                             InputMeshType;
                typedef   typename InputMeshType::Pointer        InputMeshPointer;
                typedef   typename InputMeshType::ConstPointer   InputMeshConstPointer;

                /** Some Mesh related typedefs. */
                typedef   TOutputMesh                             OutputMeshType;
                typedef   typename OutputMeshType::Pointer        OutputMeshPointer;
                typedef   typename InputMeshType::ConstPointer   InputMeshConstPointer;

                //      type def for basic storage
                typedef unsigned long IdentifierType;
                typedef typename OutputMeshType::CellType CellType;
                typedef typename CellType::CellAutoPointer CellAutoPointer;
                typedef QuadrilateralCell< CellType >      QuadrilateralCell;
                typedef Array< IdentifierType >              IdentifierArrayType;
                /**  Meshing related functions */
                //      generate quadrilateral mesh given 4 boundaries ( closed polygon)
                bool GenerateQuadrilateralMesh();
                //      Set the 4 curves that make up the boundary of a quadrilateral mesh
                void SetCurve(IdentifierType, InputMeshPointer);
                // access the boundary curves that make up the 
                InputMeshType::Pointer GetCurve(IdentifierType);
                

        protected:
                CurveToStructuredPlanarMesh();
                ~CurveToStructuredPlanarMesh();
//              void PrintSelf(std::ostream& os, Indent indent) const;
                bool CheckValidity();                                   //      check whether the 4 curves are valid
                void DesignateBoundaryNodes();                  //      copy boundary nodes
                void PlanarTransfiniteInterpolation();  //      generate internal surface nodes with boundary nodes known
                void GenerateQuadrilateralMeshConnectivity();

                typedef std::vector< InputMeshType::Pointer >   InputMeshPointerContainerVector;        //      vector of pointers
                                                                                //      for four curves
                void SetPoint(IdentifierType , InputMeshType::PointType);
                //      add quadrilateral cell
                IdentifierType AddQuadrilateral(const IdentifierArrayType& );

        private:
                CurveToStructuredPlanarMesh(const CurveToStructuredPlanarMesh&); //purposely not implemented
                void operator=(const CurveToStructuredPlanarMesh&); //purposely not implemented
                /** Member variables definition */
                /** variables for 4 input curves that make up the boundary */
                // 4 contiguous curves either CW or CCW
                //      vector for 4 curves
                InputMeshPointerContainerVector m_InputMeshPointer;
                //      planar mesh generated from 4 curves
                OutputMeshPointer m_PlanarMesh;
        };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCurveToStructuredPlanarMesh.txx"
#endif

#endif
