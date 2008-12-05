/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxPolygon3DSpatialObject.h,v $
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

#ifndef __itkPolygon3DSpatialObject_h
#define __itkPolygon3DSpatialObject_h
#include "itkBlobSpatialObject.h"

namespace itk
{
template <unsigned int TDimension = 3> class 
Polygon3DSpatialObject
  :public BlobSpatialObject<TDimension>
{
public:
  typedef Polygon3DSpatialObject<TDimension>         Self;
  typedef BlobSpatialObject< TDimension >            Superclass;
  typedef SmartPointer < Self >                      Pointer;
  typedef SmartPointer < const Self >                ConstPointer;
  typedef typename Superclass::PointType             PointType;
  typedef typename Superclass::TransformType         TransformType;
  typedef typename Superclass::PointListType         PointListType;
  typedef typename Superclass::BlobPointType         BlobPointType;
  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Method for creation through the object factory. */
  itkTypeMacro( Polygon3DSpatialObject, BlobSpatialObject );
  
 
  /** Returns if the Polygon3D is closed */
  bool IsClosed();

  /** Returns the number of points of the Polygon3D */
  unsigned int NumberOfPoints() const;

  /** Method returns the Point closest to the given point */
  PointType ClosestPoint(PointType &curPoint);

 
  /** Method returns the length of the perimeter */
  double MeasurePerimeter();

  /** Method deletes a point from the strand */
  bool DeletePoint(PointType &pointToDelete);

  /** Method adds a point to the end of the strand */
  bool AddPoint(PointType &pointToAdd);

  /** Method inserts point after point1 */
  bool InsertPoint(PointType &point1, PointType &pointToAdd);

  /** Method replaces a point */
  bool ReplacePoint(PointType &oldpoint, PointType &newPoint);

  /** Method removes the series of points between startpoint and endpoint */
  bool RemoveSegment(PointType &startpoint,PointType &endPoint);

  /** Mesh seed with number of sub-divisions specified */
  /** MeshSeed(NumberofSubDivisions, CurveSpatialObject to be mesh seeded, 
  error allowed in the length of last segment ) */
  double MeshSeed(unsigned int, PointListType&, double);

  /** Mesh seed with element length specified */
  /** MeshSeed(Element length, CurveSpatialObject to be mesh seeded,
  error allowed in the length of last segment ) */
  double MeshSeed(double, PointListType&, double);

  Polygon3DSpatialObject(){}

private:

        bool IntersectLineWithSphere(PointType&, PointType&, PointType&, double, 
                PointType&, PointType&);
        Polygon3DSpatialObject(const Self&); //purposely not implemented
        void operator=(const Self&); //purposely not implemented
private:

};
}
#ifndef ITK_MANUAL_INSTANTIATION 
#include "itkPolygon3DSpatialObject.txx" 
#endif 

#endif  // __itkPolygon3DSpatialObject_h
