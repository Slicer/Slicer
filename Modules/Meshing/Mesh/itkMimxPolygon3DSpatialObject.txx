/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxPolygon3DSpatialObject.txx,v $
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


#ifndef __itk_Polygon3DSpatialObject_txx_
#define __itk_Polygon3DSpatialObject_txx_
#include "itkPolygon3DSpatialObject.h"
#include "itkExceptionObject.h"

namespace itk
{

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::IsClosed()
{
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  itend--;
  return (*it).GetPosition() == (*itend).GetPosition();
}

template <unsigned int TDimension >
unsigned int
Polygon3DSpatialObject<TDimension>
::NumberOfPoints() const
{
  return (this->GetPoints()).size();
}

template <unsigned int TDimension >
typename Polygon3DSpatialObject<TDimension>::PointType
Polygon3DSpatialObject<TDimension>
::ClosestPoint(PointType &curPoint)
{
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  double distance = NumericTraits<double>::max();
  
  if(it == itend)
    { 
    ExceptionObject exception(__FILE__, __LINE__);
    exception.SetDescription(
      "Polygon3DSpatialObject: ClosestPoint called using an empty point list");
    throw exception;
    }
    
  PointType closestPoint;
  while (it != itend)
    {
    typename SpatialObjectPoint<TDimension>::PointType curpos 
      = (*it).GetPosition();
    double curdistance = curpos.EuclideanDistanceTo(curPoint);
    if(curdistance < distance)
      {
      closestPoint = (*it).GetPosition();
      }
    it++;
    }
  return closestPoint;
}


template <unsigned int TDimension >
double 
Polygon3DSpatialObject<TDimension>
::MeasurePerimeter()
{
  double perimeter = 0.0;
  int numpoints = this->NumberOfPoints();
  if(numpoints < 3)
    {
    return 0;
    }
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  PointType start = (*it).GetPosition();
  for(int i = 0; i < numpoints; i++)
    {
    PointType a = (*it).GetPosition();
    PointType b;
    it++;
    if(i == numpoints - 1)
      {
      b = start;
      }
    else
      {
      b = (*it).GetPosition();
      }
    //
    // closed Polygon3DGroup has first and last points the same
    if(a == b)
      {
      continue;
      }
    double curdistance = a.EuclideanDistanceTo(b);
    perimeter += curdistance;
    }
  return perimeter;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::DeletePoint(PointType &pointToDelete)
{
    
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  if(it == itend)
    { 
    return false;
    }
    
  while (it != itend)
    {
    BlobPointType &curPoint = (*it);
    typename SpatialObjectPoint<TDimension>::PointType curpos 
      = curPoint.GetPosition();
    if(curpos == pointToDelete)
      {
      points.erase(it);
      return true;
      }
    it++;
    }
  return false;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::AddPoint(PointType &pointToAdd)
{
  BlobPointType newPoint;
  newPoint.SetPosition(pointToAdd);
  this->GetPoints().push_back(newPoint);
  return true;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::InsertPoint(PointType &point1, PointType &pointToAdd)
{
  
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  if(it == itend)
    { 
    this->AddPoint(pointToAdd);
    return true;
    }
          
  while (it != itend)
    {
    BlobPointType &curPoint = (*it);
    typename SpatialObjectPoint<TDimension>::PointType curpos 
      = curPoint.GetPosition();
    if(curpos == point1)
      {
      typename PointListType::iterator after = it;
      after++;
      BlobPointType newPoint;
      newPoint.SetPosition(pointToAdd);
      points.insert(after,1,newPoint);
      return true;
      }
    it++;
    }
  return false;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::ReplacePoint(PointType &oldpoint, PointType &newPoint)
{
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  if(it == itend)
    { 
    this->AddPoint(newPoint);
    return true;
    }
    
  while(it != itend)
    {
    BlobPointType &curPoint = (*it);
    typename SpatialObjectPoint<TDimension>::PointType curpos 
      = curPoint.GetPosition();
    if(curpos == oldpoint)
      {
      typename PointListType::iterator after = it;
      after++;
      BlobPointType newBlobPoint;
      newBlobPoint.SetPosition(newPoint);
      points.insert(after,1,newBlobPoint);
      points.erase(it);
      return true;
      }
    it++;
    }
  return false;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::RemoveSegment(PointType &startpoint, PointType &endPoint)
{
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  typename PointListType::iterator first;
  typename PointListType::iterator last;
  if(it == itend)
    { 
    return false;
    }
  int foundcount = 0;
  while(it != itend)
    {
    BlobPointType &curPoint = (*it);
    typename SpatialObjectPoint<TDimension>::PointType curpos 
      = curPoint.GetPosition();
    if(curpos == startpoint) 
      {
      first = it;
      foundcount++;
      } 
    //
    // make sure you find the start before you find the end
    else if(foundcount > 0 && curpos == endPoint)
      {
      last = it;
      foundcount++;
      }
    if(foundcount == 2)
      {
      break;
      }
    it++;
    }
  if(foundcount != 2)
    {
    return false;
    }

  points.erase(last);         // stl_vector doesn't erase end of range
  points.erase(first,last);
  return true;
}

template <unsigned int TDimension >
double 
Polygon3DSpatialObject<TDimension>
::MeshSeed(double ElementLength, PointListType& TargetPointList, double Tol)
{
        typedef itk::Polygon3DSpatialObject<3>  PolygonType3D;
        PolygonType3D::Pointer  PolygonPointer3D = PolygonType3D::New();
        PolygonPointer3D->SetPoints(TargetPointList);
        double ActualElementLength = PolygonPointer3D->MeasurePerimeter();
        //      we need more than one point for the curve 
        if(PolygonPointer3D->GetNumberOfPoints() <=1)
        {
                ExceptionObject exception(__FILE__, __LINE__);
                exception.SetDescription(
                        "Polygon3DSpatialObject: Number of nodes in the curve < 2 : invalid input");
                throw exception;

        }
        else
        {
                double PolygonPointer3DPerimeter = PolygonPointer3D->MeasurePerimeter();
                //      check the length of the curve
                //      if length is less than element length stop mesh seeding
                if(PolygonPointer3DPerimeter > ElementLength)
                {
                        //      Initially estimate number of divisions, given the element length
                        unsigned int NumberOfElements = int(PolygonPointer3DPerimeter/ElementLength);
                        double ActualElementLength = this->MeshSeed(NumberOfElements,PolygonPointer3D->GetPoints(),Tol);

                }
        }
        return ActualElementLength;
}
//////////////////////////////////////////////////////////////////////////////////
template <unsigned int TDimension >
double 
Polygon3DSpatialObject<TDimension>
::MeshSeed(unsigned int NumberOfElements, PointListType& TargetPointList, double Tol)
{
        typedef itk::Polygon3DSpatialObject<3>  PolygonType3D;
        PolygonType3D::Pointer  PolygonPointer3D = PolygonType3D::New();
        PolygonPointer3D->SetPoints(TargetPointList);
        typedef itk::SpatialObjectPoint<3> SpatialObjectPointType;
        PolygonType3D::PointListType List;
        int i =0,j;
        int num = NumberOfElements;
        j=PolygonPointer3D->GetNumberOfPoints();
        double Distance = 0.0;  //      variable to store the remainder of the curve 
        //      generate the initial mesh seed with number of elements/divisions known
        double ElementLength = PolygonPointer3D->MeasurePerimeter()/NumberOfElements;
        do 
        {
                j = this->GetNumberOfPoints();
                if(j == NumberOfElements)
                        ElementLength = ElementLength + Distance/num;
                else    ElementLength =  ElementLength + (Distance*(j-num))/num;
                this->SetPoints(List);  //      to erase all the contents from earlier iteration. should find a 
                                                                //      better way to initialize the contents
                j = this->GetNumberOfPoints();
                itk::Point<double,3> FirstPoint;
                itk::Point<double,3> SecondPoint;
                itk::Point<double,3> CenterPoint;
                i=0;

                do{
                                if(i==0)        
                        {
                                FirstPoint = PolygonPointer3D->GetPoint(i++)->GetPosition();
                                SecondPoint = PolygonPointer3D->GetPoint(i)->GetPosition();
                                this->AddPoint(FirstPoint);     
        //                      cout <<FirstPoint[0]<<"  "<<FirstPoint[1]<<"  "<<FirstPoint[2]<<std::endl;
                        }
                        CenterPoint = this->GetPoint(this->GetNumberOfPoints()-1)->GetPosition();
                        Distance = CenterPoint.EuclideanDistanceTo(SecondPoint);
                        if(Distance <= ElementLength)
                        {
                                do{
                                        if(i < PolygonPointer3D->GetNumberOfPoints()-1)
                                        {
                                                SecondPoint = PolygonPointer3D->GetPoint(++i)->GetPosition();
                                                Distance = CenterPoint.EuclideanDistanceTo(SecondPoint);
                                        }
                                        else    break;
                                }while(Distance <=ElementLength);
                        }
                        if(i < PolygonPointer3D->GetNumberOfPoints())
                        {
                                do
                                {
                                        itk::Point<double,3> FirstIntPoint;
                                        itk::Point<double,3> SecondIntPoint;
                                        FirstPoint = CenterPoint;
                                        IntersectLineWithSphere(FirstPoint,SecondPoint,CenterPoint,
                                                ElementLength, FirstIntPoint, SecondIntPoint);
                                        this->AddPoint(SecondIntPoint);
        //                              cout <<SecondIntPoint[0]<<"  "<<SecondIntPoint[1]<<"  "<<SecondIntPoint[2]<<std::endl;
                                        Distance = SecondIntPoint.EuclideanDistanceTo(SecondPoint);
                                        CenterPoint = SecondIntPoint;
                                        if(Distance < ElementLength)    break;
                                }while( i < PolygonPointer3D->GetNumberOfPoints());
                        }
                        }while(i < PolygonPointer3D->GetNumberOfPoints()-1);

                        FirstPoint = this->GetPoint(this->GetNumberOfPoints()-1)->GetPosition();
                        SecondPoint = PolygonPointer3D->GetPoint(PolygonPointer3D->GetNumberOfPoints()-1)->GetPosition();
                        Distance = FirstPoint.EuclideanDistanceTo(SecondPoint);
                }while(Distance > Tol || this->GetNumberOfPoints() != NumberOfElements);
        return ElementLength;
}

template <unsigned int TDimension >
bool 
Polygon3DSpatialObject<TDimension>
::IntersectLineWithSphere(PointType& FirstPoint, PointType& SecondPoint, PointType& Center, double Radius, 
                                                  PointType& FirstIntPoint, PointType& SecondIntPoint)
{
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
