/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageRegionMomentsCalculator.h,v $
  Language:  C++
  Date:      $Date: 2008-10-06 08:54:43 $
  Version:   $Revision: 1.39 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkImageRegionMomentsCalculator_h
#define itkImageRegionMomentsCalculator_h

#include "itkAffineTransform.h"
#include "itkMacro.h"
#include "itkImage.h"
#include "itkSpatialObject.h"

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_diag_matrix.h"

namespace itk
{

/** \class ImageRegionMomentsCalculator
 * \brief Compute moments of an n-dimensional image.
 *
 * This class provides methods for computing the moments and related
 * properties of a single-echo image.  Computing the (non-central)
 * moments of a large image can easily take a million times longer
 * than computing the various other values derived from them, so we
 * compute the moments only on explicit request, and save their values
 * (in an ImageRegionMomentsCalculator object) for later retrieval by the user.
 *
 * The non-central moments computed by this class are not really
 * intended for general use and are therefore in index coordinates;
 * that is, we pretend that the index that selects a particular
 * pixel also equals its physical coordinates.  The center of gravity,
 * central moments, principal moments and principal axes are all
 * more generally useful and are computed in the physical coordinates
 * defined by the Origin and Spacing parameters of the image.
 *
 * The methods that return values return the values themselves rather
 * than references because the cost is small compared to the cost of
 * computing the moments and doing so simplifies memory management for
 * the caller.
 *
 * \ingroup Operators
 *
 * \todo It's not yet clear how multi-echo images should be handled here.
 */
template <class TImage>
class ImageRegionMomentsCalculator : public Object
{
public:
  /** Standard class typedefs. */
  typedef ImageRegionMomentsCalculator<TImage> Self;
  typedef Object                               Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageRegionMomentsCalculator, Object);

  /** Extract the dimension of the image. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TImage::ImageDimension);

  /** Standard scalar type within this class. */
  typedef double ScalarType;

  typedef typename TImage::PointType PointType;

  /** Standard vector type within this class. */
  typedef Vector<ScalarType, itkGetStaticConstMacro(ImageDimension)> VectorType;

  /** Spatial Object type within this class. */
  typedef SpatialObject<itkGetStaticConstMacro(ImageDimension)> SpatialObjectType;

  /** Spatial Object member types used within this class. */
  typedef typename SpatialObjectType::Pointer      SpatialObjectPointer;
  typedef typename SpatialObjectType::ConstPointer SpatialObjectConstPointer;

  /** Standard matrix type within this class. */
  typedef Matrix<ScalarType,
                 itkGetStaticConstMacro(ImageDimension),
                 itkGetStaticConstMacro(ImageDimension)>   MatrixType;

  /** Standard image type within this class. */
  typedef TImage ImageType;

  /** Standard image type pointer within this class. */
  typedef typename ImageType::Pointer      ImagePointer;
  typedef typename ImageType::ConstPointer ImageConstPointer;

  /** Affine transform for mapping to and from principal axis */
  typedef AffineTransform<double, itkGetStaticConstMacro(ImageDimension)> AffineTransformType;
  typedef typename AffineTransformType::Pointer                           AffineTransformPointer;

  /** Set the input image. */
  virtual void SetImage( const ImageType * image )
  {
    if( m_Image != image )
      {
      m_Image = image;
      this->Modified();
      m_Valid = false;
      }
  }

  /** Set the spatial object mask. */
  virtual void SetSpatialObjectMask( const SpatialObject<itkGetStaticConstMacro( ImageDimension )> * so )
  {
    if( m_SpatialObjectMask != so )
      {
      m_SpatialObjectMask = so;
      this->Modified();
      m_Valid = false;
      }
  }

  /** Method for controlling the region of interest that optionally limits the
   *   spatial extent of the computations */
  itkSetMacro(UseRegionOfInterest, bool);
  itkGetMacro(UseRegionOfInterest, bool);
  virtual void SetRegionOfInterest( const PointType & point1, const PointType & point2 )
  {
    if( m_RegionOfInterestPoint1 != point1 || m_RegionOfInterestPoint2 != point2 )
      {
      m_RegionOfInterestPoint1 = point1;
      m_RegionOfInterestPoint2 = point2;
      this->Modified();
      m_Valid = false;
      }
  }

  itkGetMacro(RegionOfInterestPoint1, PointType);
  itkGetMacro(RegionOfInterestPoint2, PointType);

  /** Compute moments of a new or modified image.
   * This method computes the moments of the image given as a
   * parameter and stores them in the object.  The values of these
   * moments and related parameters can then be retrieved by using
   * other methods of this object. */
  void Compute();

  /** Return the total mass (or zeroth moment) of an image.
   * This method returns the sum of pixel intensities (also known as
   * the zeroth moment or the total mass) of the image whose moments
   * were last computed by this object. */
  ScalarType GetTotalMass() const;

  /** Return first moments about origin, in index coordinates.
   * This method returns the first moments around the origin of the
   * image whose moments were last computed by this object.  For
   * simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  VectorType GetFirstMoments() const;

  /** Return second moments about origin, in index coordinates.
   * This method returns the second moments around the origin
   * of the image whose moments were last computed by this object.
   * For simplicity, these moments are computed in index coordinates
   * rather than physical coordinates. */
  MatrixType GetSecondMoments() const;

  /** Return center of gravity, in physical coordinates.
   * This method returns the center of gravity of the image whose
   * moments were last computed by this object.  The center of
   * gravity is computed in physical coordinates. */
  VectorType GetCenterOfGravity() const;

  /** Return second central moments, in physical coordinates.
   * This method returns the central second moments of the image
   * whose moments were last computed by this object.  The central
   * moments are computed in physical coordinates. */
  MatrixType GetCentralMoments() const;

  /** Return principal moments, in physical coordinates.
   * This method returns the principal moments of the image whose
   * moments were last computed by this object.  The moments are
   * returned as a vector, with the principal moments ordered from
   * smallest to largest.  The moments are computed in physical
   * coordinates.   */
  VectorType GetPrincipalMoments() const;

  /** Return principal axes, in physical coordinates.
   * This method returns the principal axes of the image whose
   * moments were last computed by this object.  The moments are
   * returned as an orthogonal matrix, each row of which corresponds
   * to one principal moment; for example, the principal axis
   * corresponding to the smallest principal moment is the vector
   * m[0], where m is the value returned by this method.  The matrix
   * of principal axes is guaranteed to be a proper rotation; that
   * is, to have determinant +1 and to preserve parity.  (Unless you
   * have foolishly made one or more of the spacing values negative;
   * in that case, _you_ get to figure out the consequences.)  The
   * moments are computed in physical coordinates. */
  MatrixType GetPrincipalAxes() const;

  /** Get the affine transform from principal axes to physical axes
   * This method returns an affine transform which transforms from
   * the principal axes coordinate system to physical coordinates. */
  AffineTransformPointer GetPrincipalAxesToPhysicalAxesTransform() const;

  /** Get the affine transform from physical axes to principal axes
   * This method returns an affine transform which transforms from
   * the physical coordinate system to the principal axes coordinate
   * system. */
  AffineTransformPointer GetPhysicalAxesToPrincipalAxesTransform() const;

protected:
  ImageRegionMomentsCalculator();
  ~ImageRegionMomentsCalculator() override;
  void PrintSelf(std::ostream& os, Indent indent) const override;

private:
  ImageRegionMomentsCalculator(const Self &); // purposely not implemented
  void operator=(const Self &);               // purposely not implemented

  bool       m_Valid;                // Have moments been computed yet?
  ScalarType m_M0;                   // Zeroth moment
  VectorType m_M1;                   // First moments about origin
  MatrixType m_M2;                   // Second moments about origin
  VectorType m_Cg;                   // Center of gravity (physical units)
  MatrixType m_Cm;                   // Second central moments (physical)
  VectorType m_Pm;                   // Principal moments (physical)
  MatrixType m_Pa;                   // Principal axes (physical)

  bool      m_UseRegionOfInterest;
  PointType m_RegionOfInterestPoint1;
  PointType m_RegionOfInterestPoint2;

  ImageConstPointer         m_Image;
  SpatialObjectConstPointer m_SpatialObjectMask;

};  // class ImageRegionMomentsCalculator

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageRegionMomentsCalculator.txx"
#endif

#endif /* itkImageRegionMomentsCalculator_h */
