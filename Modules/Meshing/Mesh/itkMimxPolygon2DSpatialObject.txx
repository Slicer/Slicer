/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxPolygon2DSpatialObject.txx,v $
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


#ifndef __itk_Polygon2DSpatialObject_txx_
#define __itk_Polygon2DSpatialObject_txx_
#include "itkPolygon2DSpatialObject.h"
#include "itkExceptionObject.h"

namespace itk
{

template <unsigned int TDimension >
PolygonGroupOrientation
Polygon2DSpatialObject<TDimension>
::Plane()
{
  PolygonGroupOrientation plane;
  // local typedef to shut up the compiler...
  
  PointListType &points = this->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  double min[3],max[3];       // x, y, z
  int i;
  for(i = 0; i < 3; i++)
    {
    max[i] = NumericTraits<double>::NonpositiveMin();
    min[i] = NumericTraits<double>::max();
    }
  while (it != itend)
    {
    PointType curpoint = (*it).GetPosition();
    for(i = 0; i < 3; i++) 
      {
      if(min[i] > curpoint[i]) min[i] = curpoint[i];
      if(max[i] < curpoint[i]) max[i] = curpoint[i];
      }
    it++;
    }
  if(min[0] == max[0] && min[1] != max[1] && min[2] != max[2])
    {
    plane = Sagittal;
    }
  else if(min[0] != max[0] && min[1] == max[1] && min[2] != max[2])
    {
    plane = Axial;
    }
  else if(min[0] != max[0] && min[1] != max[1] && min[2] == max[2])
    {
    plane = Coronal;
    }
  return plane;
}


template <unsigned int TDimension >
double
Polygon2DSpatialObject<TDimension>
::MeasureArea()
{
  //To find the area of a planar polygon not in the x-y plane, use:
  //2 A(P) = abs(N . (sum_{i=0}^{n-1} (v_i x v_{i+1})))
  //where N is a unit vector normal to the plane. The `.' represents the
  //dot product operator, the `x' represents the cross product operator,
  //        and abs() is the absolute value function.
  double area = 0.0;
  int numpoints = this->NumberOfPoints();
  int X, Y;
  if(numpoints < 3)
    {
    return 0;
    }
  switch(this->Plane())
    {
    case Sagittal:
      X = 1; Y = 2;
      break;
    case Axial:
      X = 0; Y = 2;
      break;
    case Coronal:
      X = 0; Y = 1;
      break;
    default:
      ExceptionObject exception(__FILE__, __LINE__);
      exception.SetDescription("File cannot be read");
      throw exception;
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
    // closed PolygonGroup has first and last points the same
    if(a == b)
      {
      continue;
      }
    area += a[X] * b[Y] - a[Y] * b[X];
    }
  area *= 0.5;
  return area < 0.0 ? -area : area;
}

template <unsigned int TDimension >
double 
Polygon2DSpatialObject<TDimension>
::MeasureVolume()
{
  return m_Thickness * this->MeasureArea();
}

template <unsigned int TDimension >
bool 
Polygon2DSpatialObject<TDimension>
::IsInside( const PointType & point,unsigned int ,char * ) const
{
  int numpoints = this->NumberOfPoints();
  int X, Y;
  if(numpoints < 3)
    {
    return false;
    }
  switch(const_cast<Self *>(this)->Plane())
    {
    case Sagittal:
      X = 1; Y = 2;
      break;
    case Axial:
      X = 0; Y = 2;
      break;
    case Coronal:
      X = 0; Y = 1;
      break;
    default:
      ExceptionObject exception(__FILE__, __LINE__);
      exception.SetDescription("non-planar polygon");
      throw exception;
    }

  typename TransformType::Pointer inverse = TransformType::New();
  if(!this->GetIndexToWorldTransform()->GetInverse(inverse))
    {
    return false;
    }

  PointType transformedPoint = inverse->TransformPoint(point);

  PointListType &points = const_cast<Self *>(this)->GetPoints();
  typename PointListType::iterator it = points.begin();
  typename PointListType::iterator itend = points.end();
  PointType start = (*it).GetPosition();
  PointType last = (*itend).GetPosition();
  //
  // if last point same as first, don't bother with it.
  if(start == last)
    {
    numpoints--;
    }
  bool oddNodes = false;
  for(int i = 0; i < numpoints; i++)
    {
    PointType start = (*it).GetPosition();
    it++;
    PointType end;
    if(i == numpoints - 1)
      {
      end = start;
      }
    else
      {
      end = (*it).GetPosition();
      }
    double x = transformedPoint[X]; double y = transformedPoint[Y];

    if((start[Y] < y && end[Y] >= y) ||
       (end[Y] < y && start[Y] >= y))
      {
      if( start[X] + (y - start[Y])/
          (end[Y] - start[Y]) * (end[X] - start[X]) < x )
        {
        oddNodes = !oddNodes;
        }
      }
    }
  return oddNodes;
}
}
#endif
