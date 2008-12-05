/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxCurveSpatialObject.h,v $
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

#ifndef __itkCurveSpatialObject_h
#define __itkCurveSpatialObject_h
#include "itkPolygonSpatialObject.h"

namespace itk
{
        template <unsigned int TDimension = 3> class 
                CurveSpatialObject
                :public PolygonSpatialObject<TDimension>
        {
        public:
                typedef CurveSpatialObject<TDimension>         Self;
                typedef PolygonSpatialObject< TDimension >            Superclass;
                typedef SmartPointer < Self >                      Pointer;
                typedef SmartPointer < const Self >                ConstPointer;
                typedef typename Superclass::PointType             PointType;
                typedef typename Superclass::TransformType         TransformType;
                typedef typename Superclass::PointListType         PointListType;
                /** Method for creation through the object factory. */
                itkNewMacro( Self );

                /** Method for creation through the object factory. */
                itkTypeMacro( CurveSpatialObject, PolygonSpatialObject);

                /** Mesh seed with number of sub-divisions specified */
                /** MeshSeed(NumberofSubDivisions, CurveSpatialObject to be mesh seeded, 
                                        error allowed in the length of last segment ) */
                double MeshSeed(unsigned int, CurveSpatialObject::Pointer, double);

                /** Mesh seed with element length specified */
                /** MeshSeed(Element length, CurveSpatialObject to be mesh seeded,
                                        error allowed in the length of last segment ) */
                double MeshSeed(double, CurveSpatialObject::Pointer, double);
//              CurveSpatialObject();

        private:

                bool IntersectLineWithSphere(PointType&, PointType&, PointType&, double, 
                        PointType&, PointType&);

        
        protected:
                CurveSpatialObject()
                {
                        int i=1;
                }
                
        
        
        virtual ~CurveSpatialObject();
                
                
        
        };
}
#ifndef ITK_MANUAL_INSTANTIATION 
#include "itkCurveSpatialObject.txx" 
#endif 

#endif  // __itkCurveSpatialObject_h
