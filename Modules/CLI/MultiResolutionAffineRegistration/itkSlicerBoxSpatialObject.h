/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkSlicerBoxSpatialObject.h,v $
  Language:  C++
  Date:      $Date: 2009-04-07 14:33:56 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkSlicerBoxSpatialObject_h
#define __itkSlicerBoxSpatialObject_h

#include "itkSpatialObject.h"
#include "itkAffineTransform.h"
#include "itkFixedArray.h"

namespace itk
{

/** \class SlicerBoxSpatialObject
 *
 * \brief
 * The class may be used to represent N-dimensional boxes.
 * In two dimensions it is a rectangle, In three dimensions it is a cuboid...
 *
 * This is a locally modified version itk::BoxSpatialObject
 * TODO: This should be removed when the version after ITK 3.18 is released
 *
 */
template <unsigned int TDimension = 3>
class ITK_EXPORT SlicerBoxSpatialObject
  : public SpatialObject<TDimension>
{

public:

  typedef SlicerBoxSpatialObject                    Self;
  typedef double                                    ScalarType;
  typedef SmartPointer<Self>                        Pointer;
  typedef SmartPointer<const Self>                  ConstPointer;
  typedef SpatialObject<TDimension>                 Superclass;
  typedef SmartPointer<Superclass>                  SuperclassPointer;
  typedef typename Superclass::PointType            PointType;
  typedef typename Superclass::TransformType        TransformType;
  typedef typename Superclass::BoundingBoxType      BoundingBoxType;
  typedef FixedArray<double, TDimension>            SizeType;
  typedef VectorContainer<unsigned long, PointType> PointContainerType;

  itkNewMacro( Self );
  itkTypeMacro( SlicerBoxSpatialObject, SpatialObject );

  /** Set/Get the size of the box spatial object. */
  itkSetMacro( Size, SizeType );
  itkGetConstReferenceMacro( Size, SizeType);

  /** Returns a degree of membership to the object.
   *  That's useful for fuzzy objects. */
  virtual bool ValueAt( const PointType & point, double & value, unsigned int depth = 0, char * name = NULL) const;

  /** Return true if the object provides a method to evaluate the value
   * at the specified point, false otherwise. */
  virtual bool IsEvaluableAt( const PointType & point, unsigned int depth = 0, char * name = NULL) const;

  /** Test whether a point is inside or outside the object */
  virtual bool IsInside( const PointType & point, unsigned int depth, char *) const;

  /** Test whether a point is inside or outside the object
   *  For computational speed purposes, it is faster if the method does not
   *  check the name of the class and the current depth */
  virtual bool IsInside( const PointType & point) const;

  /** Get the boundaries of a specific object.  This function needs to
   *  be called every time one of the object's components is
   *  changed. */
  virtual bool ComputeLocalBoundingBox() const;

protected:
  SlicerBoxSpatialObject(const Self &); // purposely not implemented
  void operator=(const Self &);         // purposely not implemented

  SlicerBoxSpatialObject( void );
  ~SlicerBoxSpatialObject( void );

  SizeType m_Size;

  /** Print the object informations in a stream. */
  virtual void PrintSelf( std::ostream& os, Indent indent ) const;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSlicerBoxSpatialObject.txx"
#endif

#endif // __itkSlicerBoxSpatialObject_h
