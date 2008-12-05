/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxCurveSpatialObject.txx,v $
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


#ifndef __itk_CurveSpatialObject_txx_
#define __itk_CurveSpatialObject_txx_
#include "itkCurveSpatialObject.h"
#include "itkExceptionObject.h"

namespace itk
{
        template <unsigned int TDimension >
                double 
                CurveSpatialObject<TDimension>
                ::MeshSeed(double ElementLength, CurveSpatialObject::Pointer TargetCurve, double Tol)
        {
                int i;
        
                //      we need more than one point for the curve 
                if(TargetCurve->GetNumberOfPoints() <=1)
                {

                }
                else
                {
                        double TargetCurvePerimeter = TargetCurve->MeasurePerimeter();
                        //      check the length of the curve
                        //      if length is less than element length stop mesh seeding
                        if(TargetCurvePerimeter <= ElementLength)
                        {

                        }
                        else
                        {
                                //      Initially estimate number of divisions, given the element length
                                unsigned int NumberOfElements = int(TargetCurvePerimeter/ElementLength);
                                double ActualElementLength = itk::CurveSpatialObject::MeshSeed(NumberOfElements,TargetCurve,Tol);
                        }
                }
                return ActualElementLength;
        }
//////////////////////////////////////////////////////////////////////////////////
        template <unsigned int TDimension >
                double 
                CurveSpatialObject<TDimension>
                ::MeshSeed(unsigned int NumberOfElements, CurveSpatialObject::Pointer TargetCurve, double Tol)
        {
                int i =0,j;
                double Distance = 0.0;  //      variable to store the remainder of the curve 
                //      generate the initial mesh seed with number of elements/divisions known
                double ElementLength = TargetCurve->MeasurePerimeter()/NumberOfElements;
                do 
                {
                        if(i == NumberOfElements)
                                ElementLength = ElementLength + Distance/NumberOfElements;
                        else    ElementLength = ElementLength + 
                                Distance*(i-NumberOfElements)/NumberOfElements;
                        this->Clear();
                        i=0;
                        do{
                                typename SpatialObjectPoint<TDimension>::PointType FirstPoint 
                                        = TargetCurve->GetPoint(i);
                                typename SpatialObjectPoint<TDimension>::PointType SecondPoint 
                                        = TargetCurve->GetPoint(i+1);
                                typename SpatialObjectPoint<TDimension>::PointType Center;
                                if(i==0)        this->AddPoint(FirstPoint);
                                Center = this->GetPoint(this->GetNumberOfPoints()-1);
                                Distance = Center.EuclideanDistanceTo(SecondPoint);
                                if(Distance <= ElementLength)
                                {
                                        do{
                                                if(i < TargetCurve->GetNumberOfPoints()-1)
                                                {
                                                        SecondPoint = TargetCurve->GetPoint(++i);
                                                        Distance = Center.EuclideanDistanceTo(SecondPoint);
                                                }
                                                else    break;
                                        }while(Distance <=ElementLength)
                                }
                                if(i < TargetCurve->GetNumberOfPoints())
                                {
                                        do
                                        {
                                                typename SpatialObjectPoint<TDimension>::PointType FirstIntPoint;
                                                typename SpatialObjectPoint<TDimension>::PointType SecondIntPoint;
                                                IntersectLineWithSphere(FirstPoint,SecondPoint,Center,
                                                        ElementLength, FirstIntPoint, SecondIntPoint);
                                                this->AddPoint(SecondIntPoint);
                                                i++;
                                                Distance = SecondIntPoint.EuclideanDistanceTo(SecondPoint);
                                                if(Distance < ElementLength)    break;
                                        }while( i < TargetCurve->GetNumberOfPoints());
                                }
                                Distance = this->GetPoint(this->GetNumberOfPoints()-1).EuclideanDistanceTo(
                                        TargetCurve->GetPoint(TargetCurve->GetNumberOfPoints-1));
                        }while(Distance > Tol || this->GetNumberOfPoints() != NumberOfElements);
                }
        }

        template <unsigned int TDimension >
                bool 
                CurveSpatialObject<TDimension>
                ::IntersectLineWithSphere(PointType& FirstPoint, PointType& SecondPoint, PointType& Center, double Radius, 
                                                                PointType& FirstIntPoint, PointType& SecondIntPoint)
        {
                //      method followed is from http://astronomy.swin.edu.au/~pbourke/geometry/sphereline/
                //      define a,b and c as in a quadratic equation
                double a = FirstPoint.EuclideanDistanceTo(SecondPoint);
                a = a*a;
                int i;
                double b = 0.0;
                for(i=0; i < 3; i++)    b = b + 2.0*((SecondPoint[i]-FirstPoint[i]) * (FirstPoint[i]-Center[i]));
                double c = 0.0;
                for(i=0; i < 3; i++)    c = c + Center[i]*Center[i];
                for(i=0; i < 3; i++)    c = c + FirstPoint[i]*FirstPoint[i];
                for(i=0; i < 3; i++)    c = c - 2.0*Center[i]*FirstPoint[i];
                c = c - Radius*Radius;
                if((b*b - 4.0*a*c) < 0.0) return false;
                else
                {
                        double Soln = (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
                        for(i=0; i < 3; i++)    
                        {
                                SecondIntPoint[i] = 0.0;
                                SecondIntPoint[i] = FirstPoint[i] + Soln*(SecondPoint[i]-FirstPoint[i]);
                        }
                        return true;
                }
        }
        }
#endif
