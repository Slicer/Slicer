/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkSlicerBoxSpatialObject.txx,v $
  Language:  C++
  Date:      $Date: 2008-06-29 01:56:12 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkSlicerBoxSpatialObject_txx
#define __itkSlicerBoxSpatialObject_txx

#include "itkSlicerBoxSpatialObject.h" 
#include "itkNumericTraits.h"

namespace itk 
{ 

/** Constructor */
template< unsigned int TDimension >
SlicerBoxSpatialObject< TDimension >
::SlicerBoxSpatialObject()
{
  this->SetTypeName("SlicerBoxSpatialObject");
  m_Size.Fill(0);
  this->SetDimension(TDimension);
} 

/** Destructor */
template< unsigned int TDimension >
SlicerBoxSpatialObject< TDimension >
::~SlicerBoxSpatialObject()  
{
  
}


/** Test whether a point is inside or outside the object 
 *  For computational speed purposes, it is faster if the method does not
 *  check the name of the class and the current depth */ 
template< unsigned int TDimension >
bool 
SlicerBoxSpatialObject< TDimension >
::IsInside( const PointType & point) const
{
  if( !this->SetInternalInverseTransformToWorldToIndexTransform() )
    {
    return false;
    }

  const PointType transformedPoint = 
    this->GetInternalInverseTransform()->TransformPoint(point);

  bool isInside = true;
  for(unsigned int i=0;i<TDimension;i++)
    {
    if( m_Size[i] )
      {
      if( (transformedPoint[i] > m_Size[i]) || (transformedPoint[i] < 0) )
        {
        isInside = false;
        break;
        }
      }
    else
      {
      itkExceptionMacro(<< "Size of the SlicerBoxSpatialObject must be non-zero!" );
      }
    }
  
  return isInside;
}

/** Test if the given point is inside the box. A point on the border is 
 *  considered inside. */
template< unsigned int TDimension >
bool 
SlicerBoxSpatialObject< TDimension > 
::IsInside( const PointType & point, unsigned int depth, char * name ) const 
{
  itkDebugMacro( "Checking the point [" 
                 << point << "] is inside the AxisAlignedBox" );
    
  if(name == NULL)
    {
    if(IsInside(point))
      {
      return true;
      }
    }
  else if(strstr(typeid(Self).name(), name))
    {
    if(IsInside(point))
      {
      return true;
      }
    }

  return Superclass::IsInside(point, depth, name);
} 

/** Compute the bounds of the box */
template< unsigned int TDimension >
bool
SlicerBoxSpatialObject< TDimension >
::ComputeLocalBoundingBox() const
{ 
  itkDebugMacro( "Computing SlicerBoxSpatialObject bounding box" );

  if( this->GetBoundingBoxChildrenName().empty() 
      || strstr(typeid(Self).name(), 
                this->GetBoundingBoxChildrenName().c_str()) )
    {
    
    // First we compute the bounding box in the index space
    typename BoundingBoxType::Pointer bb = BoundingBoxType::New();

    PointType pntMin;
    PointType pntMax;
    unsigned int i;
    for(i=0; i<TDimension;i++)
      {
      pntMin[i] = NumericTraits< ITK_TYPENAME PointType::ValueType >::Zero;
      pntMax[i] = static_cast< typename PointType::ValueType >( m_Size[i] );
      }
    
    bb->SetMinimum(pntMin);
    bb->SetMaximum(pntMax);

    const_cast< BoundingBoxType * >(this->GetBounds())->SetMinimum( pntMin );
    const_cast< BoundingBoxType * >(this->GetBounds())->SetMinimum( pntMax );

    bb->ComputeBoundingBox();

    typedef typename BoundingBoxType::PointsContainer PointsContainer;
    const PointsContainer * corners = bb->GetCorners();
    typename BoundingBoxType::PointsContainer::const_iterator 
                                                      it = corners->begin();
    while(it != corners->end())
      {
      PointType pnt = this->GetIndexToWorldTransform()->TransformPoint(*it);
      const_cast<BoundingBoxType *>(this->GetBounds())->ConsiderPoint(pnt);
      ++it;
      }
    }
  return true;
} 


/** Returns if the box is evaluable at one point */
template< unsigned int TDimension >
bool
SlicerBoxSpatialObject< TDimension >
::IsEvaluableAt( const PointType & point, 
                 unsigned int depth, char * name ) const
{
  itkDebugMacro( "Checking if the SlicerBoxSpatialObject is evaluable at " << point );
  return IsInside(point, depth, name);
}

/** Returns the value at one point */
template< unsigned int TDimension >
bool
SlicerBoxSpatialObject< TDimension >
::ValueAt( const PointType & point, double & value, unsigned int depth,
           char * name ) const
{
  itkDebugMacro( "Getting the value of the SlicerBoxSpatialObject at " << point );
  if( IsInside(point, 0, name) )
    {
    value = this->GetDefaultInsideValue();
    return true;
    }
  else
    {
    if( Superclass::IsEvaluableAt(point, depth, name) )
      {
      Superclass::ValueAt(point, value, depth, name);
      return true;
      }
    else
      {
      value = this->GetDefaultOutsideValue();
      return false;
      }
    }
  return false;
}

/** Print Self function */
template< unsigned int TDimension >
void 
SlicerBoxSpatialObject< TDimension >
::PrintSelf( std::ostream& os, Indent indent ) const
{

  Superclass::PrintSelf(os, indent);
  os << "Size: " << m_Size << std::endl;

}

} // end namespace itk

#endif
