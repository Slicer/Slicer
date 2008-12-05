/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxPolygon2DSpatialObject.h,v $
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

#ifndef __itkPolygon2DSpatialObject_h
#define __itkPolygon2DSpatialObject_h
#include "itkPolygon3DSpatialObject.h"
#include "itkPolygonGroupOrientation.h"

namespace itk
{
template <unsigned int TDimension = 3> class 
Polygon2DSpatialObject
  :public Polygon3DSpatialObject<TDimension>
{
public:
  typedef Polygon2DSpatialObject<TDimension>         Self;
  typedef Polygon3DSpatialObject< TDimension >            Superclass;
  typedef SmartPointer < Self >                      Pointer;
  typedef SmartPointer < const Self >                ConstPointer;
  typedef typename Superclass::PointType             PointType;
  typedef typename Superclass::TransformType         TransformType;
  typedef typename Superclass::PointListType         PointListType;
  typedef typename Superclass::BlobPointType         BlobPointType;
  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Method for creation through the object factory. */
  itkTypeMacro( Polygon2DSpatialObject, Polygon3DSpatialObject );
  
  /** Method returning plane alignment of strand */
  PolygonGroupOrientation Plane();

  /** Method sets the thickness of the current strand */
  itkSetMacro(Thickness,double);

  /** Method gets the thickness of the current strand */
  itkGetMacro(Thickness,double);

  /** Method returns area of Polygon2D described by points */
  double MeasureArea();

  /** Method returns the volume of the strand */
  double MeasureVolume();

  /** Test whether a point is inside or outside the object. */ 
  virtual bool IsInside( const PointType & point,
                         unsigned int depth,
                         char * name) const;

  /** Test whether a point is inside or outside the object 
   *  For computational speed purposes, it is faster if the method does not
   *  check the name of the class and the current depth */ 
  virtual bool IsInside( const PointType & point) const
  {
    return this->IsInside(point, 0, NULL);
  };

private:
  PolygonGroupOrientation m_Orientation;
  double m_Thickness;
  Polygon2DSpatialObject() 
  { 
    m_Orientation = Unknown; 
    m_Thickness = 0.0;
  }
};
}
#ifndef ITK_MANUAL_INSTANTIATION 
#include "itkPolygon2DSpatialObject.txx" 
#endif 

#endif  // __itkPolygon2DSpatialObject_h
