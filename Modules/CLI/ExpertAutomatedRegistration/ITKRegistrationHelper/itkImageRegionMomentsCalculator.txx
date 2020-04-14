/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageRegionMomentsCalculator.txx,v $
  Language:  C++
  Date:      $Date: 2008-10-06 08:54:44 $
  Version:   $Revision: 1.56 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkImageRegionMomentsCalculator_txx
#define itkImageRegionMomentsCalculator_txx
#include "itkImageRegionMomentsCalculator.h"

#include "vnl/algo/vnl_real_eigensystem.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"
#include "itkImageRegionConstIteratorWithIndex.h"

namespace itk
{

class InvalidImageRegionMomentsError : public ExceptionObject
{
public:
  /*
   * Constructor. Needed to ensure the exception object can be copied.
   */
  InvalidImageRegionMomentsError(const char *file, unsigned int lineNumber) : ExceptionObject(file, lineNumber)
  {
    this->SetDescription("No valid image moments are available.");
  }

  /*
   * Constructor. Needed to ensure the exception object can be copied.
   */
  InvalidImageRegionMomentsError(const std::string& file, unsigned int lineNumber) : ExceptionObject(file, lineNumber)
  {
    this->SetDescription("No valid image moments are available.");
  }

  itkTypeMacro(InvalidImageRegionMomentsError, ExceptionObject);
};

// ----------------------------------------------------------------------
// Construct without computing moments
template <class TImage>
ImageRegionMomentsCalculator<TImage>::ImageRegionMomentsCalculator()
{
  m_Valid = false;
  m_Image = nullptr;
  m_SpatialObjectMask = nullptr;
  m_M0 = NumericTraits<ScalarType>::ZeroValue();
  m_M1.Fill(NumericTraits<typename VectorType::ValueType>::ZeroValue());
  m_M2.Fill(NumericTraits<typename MatrixType::ValueType>::ZeroValue());
  m_Cg.Fill(NumericTraits<typename VectorType::ValueType>::ZeroValue());
  m_Cm.Fill(NumericTraits<typename MatrixType::ValueType>::ZeroValue());
  m_Pm.Fill(NumericTraits<typename VectorType::ValueType>::ZeroValue());
  m_Pa.Fill(NumericTraits<typename MatrixType::ValueType>::ZeroValue());
  m_UseRegionOfInterest = false;
  m_RegionOfInterestPoint1.Fill(0);
  m_RegionOfInterestPoint2.Fill(0);
}

// ----------------------------------------------------------------------
// Destructor
template <class TImage>
ImageRegionMomentsCalculator<TImage>::
~ImageRegionMomentsCalculator() = default;

template <class TInputImage>
void
ImageRegionMomentsCalculator<TInputImage>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Image: " << m_Image.GetPointer() << std::endl;
  os << indent << "Valid: " << m_Valid << std::endl;
  os << indent << "Zeroth Moment about origin: " << m_M0 << std::endl;
  os << indent << "First Moment about origin: " << m_M1 << std::endl;
  os << indent << "Second Moment about origin: " << m_M2 << std::endl;
  os << indent << "Center of Gravity: " << m_Cg << std::endl;
  os << indent << "Second central moments: " << m_Cm << std::endl;
  os << indent << "Principal Moments: " << m_Pm << std::endl;
  os << indent << "Principal axes: " << m_Pa << std::endl;
  os << indent << "Use RegionOfInterest : " << m_UseRegionOfInterest << std::endl;
  os << indent << "RegionOfInterest Point1: " << m_RegionOfInterestPoint1 << std::endl;
  os << indent << "RegionOfInterest Point2: " << m_RegionOfInterestPoint2 << std::endl;
}

// ----------------------------------------------------------------------
// Compute moments for a new or modified image
template <class TImage>
void
ImageRegionMomentsCalculator<TImage>::Compute()
{
  m_M0 = NumericTraits<ScalarType>::ZeroValue();
  m_M1.Fill(NumericTraits<typename VectorType::ValueType>::ZeroValue());
  m_M2.Fill(NumericTraits<typename MatrixType::ValueType>::ZeroValue());
  m_Cg.Fill(NumericTraits<typename VectorType::ValueType>::ZeroValue());
  m_Cm.Fill(NumericTraits<typename MatrixType::ValueType>::ZeroValue());

  typedef typename ImageType::IndexType IndexType;

  if( !m_Image )
    {
    return;
    }

  ImageRegionConstIteratorWithIndex<ImageType> it( m_Image,
                                                   m_Image->GetRequestedRegion() );

  while( !it.IsAtEnd() )
    {
    double value = it.Value();

    IndexType indexPosition = it.GetIndex();

    Point<double, ImageDimension> physicalPosition;
    m_Image->TransformIndexToPhysicalPoint(indexPosition, physicalPosition);

    bool isInsideRegionOfInterest = true;
    if( m_UseRegionOfInterest )
      {
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        if( !( (physicalPosition[i] <= m_RegionOfInterestPoint1[i]
                && physicalPosition[i] >= m_RegionOfInterestPoint2[i])
               || (physicalPosition[i] <= m_RegionOfInterestPoint2[i]
                   && physicalPosition[i] >= m_RegionOfInterestPoint1[i]) ) )
          {
          isInsideRegionOfInterest = false;
          break;
          }
        }
      }

    if( isInsideRegionOfInterest &&
        (m_SpatialObjectMask.IsNull()
         || m_SpatialObjectMask->IsInside(physicalPosition) ) )
      {
      m_M0 += value;
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        m_M1[i] += static_cast<double>( indexPosition[i] ) * value;
        for( unsigned int j = 0; j < ImageDimension; j++ )
          {
          double weight = value * static_cast<double>( indexPosition[i] )
            * static_cast<double>( indexPosition[j] );
          m_M2[i][j] += weight;
          }
        }
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        m_Cg[i] += physicalPosition[i] * value;
        for( unsigned int j = 0; j < ImageDimension; j++ )
          {
          double weight = value * physicalPosition[i] * physicalPosition[j];
          m_Cm[i][j] += weight;
          }

        }
      }

    ++it;
    }

  // Throw an error if the total mass is zero
  if( m_M0 == 0.0 )
    {
    itkExceptionMacro(
      << "Compute(): Total Mass of the image was zero. Aborting here to prevent division by zero later on.");
    }
  // Normalize using the total mass
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    m_Cg[i] /= m_M0;
    m_M1[i] /= m_M0;
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      m_M2[i][j] /= m_M0;
      m_Cm[i][j] /= m_M0;
      }
    }
  // Center the second order moments
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      m_M2[i][j] -= m_M1[i] * m_M1[j];
      m_Cm[i][j] -= m_Cg[i] * m_Cg[j];
      }
    }

  // Compute principal moments and axes
  vnl_symmetric_eigensystem<double> eigen( m_Cm.GetVnlMatrix().as_ref() );
  vnl_diag_matrix<double>           pm = eigen.D;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    m_Pm[i] = pm(i, i) * m_M0;
    }
  m_Pa = eigen.V.transpose();

  // Add a final reflection if needed for a proper rotation,
  // by multiplying the last row by the determinant
  vnl_real_eigensystem                  eigenrot( m_Pa.GetVnlMatrix().as_ref() );
  vnl_diag_matrix<std::complex<double> > eigenval = eigenrot.D;
  std::complex<double>                   det( 1.0, 0.0 );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    det *= eigenval( i, i );
    }
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    m_Pa[ImageDimension - 1][i] *= std::real( det );
    }

  /* Remember that the moments are valid */
  m_Valid = true;

}

// ---------------------------------------------------------------------
// Get sum of intensities
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::ScalarType
ImageRegionMomentsCalculator<TImage>::GetTotalMass() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetTotalMass() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_M0;
}

// --------------------------------------------------------------------
// Get first moments about origin, in index coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::VectorType
ImageRegionMomentsCalculator<TImage>::GetFirstMoments() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetFirstMoments() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_M1;
}

// --------------------------------------------------------------------
// Get second moments about origin, in index coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::MatrixType
ImageRegionMomentsCalculator<TImage>::GetSecondMoments() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetSecondMoments() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_M2;
}

// --------------------------------------------------------------------
// Get center of gravity, in physical coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::VectorType
ImageRegionMomentsCalculator<TImage>::GetCenterOfGravity() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetCenterOfGravity() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_Cg;
}

// --------------------------------------------------------------------
// Get second central moments, in physical coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::MatrixType
ImageRegionMomentsCalculator<TImage>::GetCentralMoments() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetCentralMoments() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_Cm;
}

// --------------------------------------------------------------------
// Get principal moments, in physical coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::VectorType
ImageRegionMomentsCalculator<TImage>::GetPrincipalMoments() const
{
  if( !m_Valid )
    {
    itkExceptionMacro(
      << "GetPrincipalMoments() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_Pm;
}

// --------------------------------------------------------------------
// Get principal axes, in physical coordinates
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::MatrixType
ImageRegionMomentsCalculator<TImage>::GetPrincipalAxes() const
{
  if( !m_Valid )
    {
    itkExceptionMacro( << "GetPrincipalAxes() invoked, but the moments have not been computed. Call Compute() first.");
    }
  return m_Pa;
}

// --------------------------------------------------------------------
// Get principal axes to physical axes transform
template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::AffineTransformPointer
ImageRegionMomentsCalculator<TImage>::GetPrincipalAxesToPhysicalAxesTransform() const
{
  typename AffineTransformType::MatrixType matrix;
  typename AffineTransformType::OffsetType offset;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    offset[i]  = m_Cg[i];
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      matrix[j][i] = m_Pa[i][j];    // Note the transposition
      }
    }

  AffineTransformPointer result = AffineTransformType::New();

  result->SetMatrix(matrix);
  result->SetOffset(offset);

  return result;
}

// --------------------------------------------------------------------
// Get physical axes to principal axes transform

template <class TImage>
typename ImageRegionMomentsCalculator<TImage>::AffineTransformPointer
ImageRegionMomentsCalculator<TImage>::GetPhysicalAxesToPrincipalAxesTransform() const
{
  typename AffineTransformType::MatrixType matrix;
  typename AffineTransformType::OffsetType offset;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    offset[i]    = m_Cg[i];
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      matrix[j][i] = m_Pa[i][j];    // Note the transposition
      }
    }

  AffineTransformPointer result = AffineTransformType::New();
  result->SetMatrix(matrix);
  result->SetOffset(offset);

  AffineTransformPointer inverse = AffineTransformType::New();
  result->GetInverse(inverse);

  return inverse;
}

} // end namespace itk

#endif
