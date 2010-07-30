/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkESMDemonsRegistrationFunction.txx,v $
 *  Language:  C++
 *  Date:      $Date: 2008-07-11 19:02:04 $
 *  Version:   $Revision: 1.4 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkVectorESMDemonsRegistrationFunction_txx
#define __itkVectorESMDemonsRegistrationFunction_txx

#include "itkVectorESMDemonsRegistrationFunction.h"
#include "itkExceptionObject.h"
#include "vnl/vnl_math.h"

namespace itk
{
/**
  * Default constructor
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::VectorESMDemonsRegistrationFunction()
{
  RadiusType   r;
  unsigned int j;

  for ( j = 0; j < ImageDimension; j++ )
    {
    r[j] = 0;
    }
  this->SetRadius(r);

  m_TimeStep = 1.0;
  m_DenominatorThreshold = 1e-9;
  m_IntensityDifferenceThreshold = 0.001;
  m_MaximumUpdateStepLength = 0.5;

  this->SetMovingImage(NULL);
  this->SetFixedImage(NULL);
  m_FixedImageSpacing.Fill(1.0);
  m_FixedImageOrigin.Fill(0.0);
  m_FixedImageDirection.SetIdentity();
  m_Normalizer = 0.0;

  this->m_UseGradientType = Symmetric;

  m_MovingImageWarperVector.reserve(10);
  m_MovingImageInterpolatorVector.reserve(10);

  m_FixedImageGradientCalculatorVector.reserve(10);
  m_MappedMovingImageGradientCalculatorVector.reserve(10);

  for ( unsigned int i = 0; i < 3; ++i )
    {
    typename DefaultInterpolatorType::Pointer interp =
      DefaultInterpolatorType::New();

    m_MovingImageInterpolator = static_cast< InterpolatorType * >(
      interp.GetPointer() );

    m_MovingImageInterpolatorVector.push_back(m_MovingImageInterpolator);

    m_MovingImageWarper = WarperType::New();
    m_MovingImageWarper->SetInterpolator(m_MovingImageInterpolator);
    m_MovingImageWarper->SetEdgePaddingValue(
      NumericTraits< MovingPixelType >::max() );

    m_MovingImageWarperVector.push_back(m_MovingImageWarper);
    m_FixedImageGradientCalculator = GradientCalculatorType::New();
    m_FixedImageGradientCalculator->UseImageDirectionOff();
    m_FixedImageGradientCalculatorVector.push_back(m_FixedImageGradientCalculator);
    m_MappedMovingImageGradientCalculator =
      MovingImageGradientCalculatorType::New();
    m_MappedMovingImageGradientCalculator->UseImageDirectionOff();
    m_MappedMovingImageGradientCalculatorVector.push_back(m_MappedMovingImageGradientCalculator);
    }

  m_Metric = NumericTraits< double >::max();
  m_SumOfSquaredDifference = 0.0;
  m_NumberOfPixelsProcessed = 0L;
  m_RMSChange = NumericTraits< double >::max();
  m_SumOfSquaredChange = 0.0;
}

/*
  * Standard "PrintSelf" method.
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "UseGradientType: ";
  os << m_UseGradientType << std::endl;
  os << indent << "MaximumUpdateStepLength: ";
  os << m_MaximumUpdateStepLength << std::endl;

  os << indent << "MovingImageIterpolator: ";
  os << m_MovingImageInterpolator.GetPointer() << std::endl;
  os << indent << "FixedImageGradientCalculator: ";
  //  os << m_FixedImageGradientCalculator.GetPointer() << std::endl;
  os << indent << "MappedMovingImageGradientCalculator: ";
  //  os << m_MappedMovingImageGradientCalculator.GetPointer() << std::endl;
  os << indent << "DenominatorThreshold: ";
  os << m_DenominatorThreshold << std::endl;
  os << indent << "IntensityDifferenceThreshold: ";
  os << m_IntensityDifferenceThreshold << std::endl;

  os << indent << "Metric: ";
  os << m_Metric << std::endl;
  os << indent << "SumOfSquaredDifference: ";
  os << m_SumOfSquaredDifference << std::endl;
  os << indent << "NumberOfPixelsProcessed: ";
  os << m_NumberOfPixelsProcessed << std::endl;
  os << indent << "RMSChange: ";
  os << m_RMSChange << std::endl;
  os << indent << "SumOfSquaredChange: ";
  os << m_SumOfSquaredChange << std::endl;
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::SetIntensityDifferenceThreshold(double threshold)
{
  m_IntensityDifferenceThreshold = threshold;
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
double
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::GetIntensityDifferenceThreshold() const
{
  return m_IntensityDifferenceThreshold;
}

/**
  * Set the function state values before each iteration
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::InitializeIteration()
{
  if ( !this->GetMovingImage() || !this->GetFixedImage()
       || !m_MovingImageInterpolator )
    {
    itkExceptionMacro(
      << "MovingImage, FixedImage and/or Interpolator not set");
    }

  // cache fixed image information

  m_FixedImageOrigin  = this->GetFixedImage()->GetOrigin();
  m_FixedImageSpacing = this->GetFixedImage()->GetSpacing();
  m_FixedImageDirection = this->GetFixedImage()->GetDirection();

  // compute the normalizer
  if ( m_MaximumUpdateStepLength > 0.0 )
    {
    m_Normalizer = 0.0;
    for ( unsigned int k = 0; k < ImageDimension; k++ )
      {
      m_Normalizer += m_FixedImageSpacing[k] * m_FixedImageSpacing[k];
      }
    m_Normalizer *= m_MaximumUpdateStepLength * m_MaximumUpdateStepLength
                    / static_cast< double >( ImageDimension );
    }
  else
    {
    // set it to minus one to denote a special case
    // ( unrestricted update length )
    m_Normalizer = -1.0;
    }

  typedef itk::ImageRegionConstIterator< TFixedImage > ConstIteratorType;
  typedef itk::ImageRegionIterator< FixedImageType >   IteratorType;

  ConstIteratorType in_Fix( this->GetFixedImage(),
                            this->GetFixedImage()->GetRequestedRegion() );

  ConstIteratorType in_Mov( this->GetMovingImage(),
                            this->GetMovingImage()->GetRequestedRegion() );

  for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
    {
    typename AdaptorType::Pointer vectorFixedImageToImageAdaptor =
      AdaptorType::New();
    vectorFixedImageToImageAdaptor->SetExtractComponentIndex(i);
    vectorFixedImageToImageAdaptor->SetImage( const_cast< VectorFixedImageType * >(
                                                this->GetFixedImage() ) );
    vectorFixedImageToImageAdaptor->Update();

    typename AdaptorType::Pointer vectorMovingImageToImageAdaptor =
      AdaptorType::New();
    vectorMovingImageToImageAdaptor->SetExtractComponentIndex(i);
    vectorMovingImageToImageAdaptor->SetImage( const_cast< VectorMovingImageType
                                                           * >( this->
                                                                GetMovingImage() ) );
    vectorMovingImageToImageAdaptor->Update();

    // setup gradient calculator
    m_FixedImageGradientCalculatorVector[i]->SetInputImage(
      vectorFixedImageToImageAdaptor);
    m_MappedMovingImageGradientCalculatorVector[i]->SetInputImage(
      vectorMovingImageToImageAdaptor);

    // Compute warped moving image
    m_MovingImageWarperVector[i]->SetOutputOrigin(
      this->GetFixedImage()->GetOrigin() );
    m_MovingImageWarperVector[i]->SetOutputSpacing(
      this->GetFixedImage()->GetSpacing() );

    m_MovingImageWarperVector[i]->SetOutputDirection(m_FixedImageDirection);
    //  m_MovingImageWarperVector[i]->SetInput( movingimage );
    m_MovingImageWarperVector[i]->SetInput(vectorMovingImageToImageAdaptor);
    m_MovingImageWarperVector[i]->SetDeformationField(
      this->GetDeformationField() );
    m_MovingImageWarperVector[i]->GetOutput()->SetRequestedRegion(
      this->GetDeformationField()->GetRequestedRegion() );
    m_MovingImageWarperVector[i]->Update();

    // setup moving image interpolator for further access
    m_MovingImageInterpolatorVector[i]->SetInputImage(
      vectorMovingImageToImageAdaptor);
    }
  // initialize metric computation variables
  m_SumOfSquaredDifference  = 0.0;
  m_NumberOfPixelsProcessed = 0L;
  m_SumOfSquaredChange      = 0.0;
}

/**
  * Compute update at a non boundary neighbourhood
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
typename VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                              TDeformationField >
::PixelType
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::ComputeUpdate( const NeighborhoodType & it, void *gd,
                 const FloatOffsetType & itkNotUsed(offset) )
{
  GlobalDataStruct *globalData = (GlobalDataStruct *)gd;
  PixelType         update;
  IndexType         FirstIndex =
    this->GetFixedImage()->GetLargestPossibleRegion().GetIndex();
  IndexType LastIndex =
    this->GetFixedImage()->GetLargestPossibleRegion().GetIndex()
    + this->GetFixedImage()->
    GetLargestPossibleRegion().GetSize();

  const IndexType index = it.GetIndex();

  // Get fixed image related information
  // Note: no need to check if the index is within
  // fixed image buffer. This is done by the external filter.
  std::vector< CovariantVectorType > usedOrientFreeGradientTimes2;
  std::vector< double >              speedValue;

  for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
    {
    const double fixedValue = static_cast< double >(
      this->GetFixedImage()->GetPixel(index).GetElement(i) );
    /*
      * if(index[0]==32 && index[1]==32 && index[2]==32)
      *  {
      *  std::cout <<  "FV  at index "<< i  << "is " << fixedValue << std::endl;
      *  std::cout <<  "FV  at index is " << this->GetFixedImage()->GetPixel(
      * index) << std::endl;
      *  }
      */
    // Get moving image related information
    // check if the point was mapped outside of the moving image using
    // the "special value" NumericTraits<MovingPixelType>::max()
    MovingPixelType movingPixValue =
      m_MovingImageWarperVector[i]->GetOutput()->GetPixel(index);

    if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
      {
      update.Fill(0.0);
      return update;
      }
    const double movingValue = static_cast< double >( movingPixValue );

    // We compute the gradient more or less by hand.
    // We first start by ignoring the image orientation and introduce it
    // afterwards

    if ( ( this->m_UseGradientType == Symmetric )
         ||   ( this->m_UseGradientType == WarpedMoving ) )
      {
      // we don't use a CentralDifferenceImageFunction here to be able to
      // check for NumericTraits<MovingPixelType>::max()
      CovariantVectorType warpedMovingGradient;
      IndexType           tmpIndex = index;
      for ( unsigned int dim = 0; dim < ImageDimension; dim++ )
        {
        // bounds checking
        if ( FirstIndex[dim] == LastIndex[dim] || index[dim] <
             FirstIndex[dim] || index[dim] >= LastIndex[dim] )
          {
          warpedMovingGradient[dim] = 0.0;
          continue;
          }
        else if ( index[dim] == FirstIndex[dim] )
          {
          // compute derivative
          tmpIndex[dim] += 1;
          movingPixValue = m_MovingImageWarperVector[i]->GetOutput()->GetPixel(
            tmpIndex);
          if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
            {
            // weird crunched border case
            warpedMovingGradient[dim] = 0.0;
            }
          else
            {
            // forward difference
            warpedMovingGradient[dim] = static_cast< double >( movingPixValue )
                                        - movingValue;
            warpedMovingGradient[dim] /= m_FixedImageSpacing[dim];
            }
          tmpIndex[dim] -= 1;
          continue;
          }
        else if ( index[dim] == ( LastIndex[dim] - 1 ) )
          {
          // compute derivative
          tmpIndex[dim] -= 1;
          movingPixValue = m_MovingImageWarperVector[i]->GetOutput()->GetPixel(
            tmpIndex);
          if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
            {
            // weird crunched border case
            warpedMovingGradient[dim] = 0.0;
            }
          else
            {
            // backward difference
            warpedMovingGradient[dim] = movingValue - static_cast< double >(
              movingPixValue );
            warpedMovingGradient[dim] /= m_FixedImageSpacing[dim];
            }
          tmpIndex[dim] += 1;
          continue;
          }

        // compute derivative
        tmpIndex[dim] += 1;
        movingPixValue = m_MovingImageWarperVector[i]->GetOutput()->GetPixel(
          tmpIndex);
        if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
          {
          // backward difference
          warpedMovingGradient[dim] = movingValue;

          tmpIndex[dim] -= 2;
          movingPixValue = m_MovingImageWarperVector[i]->GetOutput()->GetPixel(
            tmpIndex);
          if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
            {
            // weird crunched border case
            warpedMovingGradient[dim] = 0.0;
            }
          else
            {
            // backward difference
            warpedMovingGradient[dim] -= static_cast< double >(
              m_MovingImageWarperVector[i]->GetOutput()->GetPixel(tmpIndex) );

            warpedMovingGradient[dim] /= m_FixedImageSpacing[dim];
            }
          }
        else
          {
          warpedMovingGradient[dim] = static_cast< double >( movingPixValue );

          tmpIndex[dim] -= 2;
          movingPixValue = m_MovingImageWarperVector[i]->GetOutput()->GetPixel(
            tmpIndex);
          if ( movingPixValue == NumericTraits< MovingPixelType >::max() )
            {
            // forward difference
            warpedMovingGradient[dim] -= movingValue;
            warpedMovingGradient[dim] /= m_FixedImageSpacing[dim];
            }
          else
            {
            // normal case, central difference
            warpedMovingGradient[dim] -= static_cast< double >( movingPixValue );
            warpedMovingGradient[dim] *= 0.5 / m_FixedImageSpacing[dim];
            }
          }
        tmpIndex[dim] += 1;
        }

      if ( this->m_UseGradientType == Symmetric )
        {
        // Compute orientation-free gradient with calculator
        const CovariantVectorType fixedGradient =
          m_FixedImageGradientCalculatorVector[i]->EvaluateAtIndex(index);

        usedOrientFreeGradientTimes2.push_back(
          fixedGradient + warpedMovingGradient);
        }
      else if ( this->m_UseGradientType == WarpedMoving )
        {
        usedOrientFreeGradientTimes2.push_back(
          warpedMovingGradient + warpedMovingGradient);
        }
      else
        {
        itkExceptionMacro(<< "Unknown gradient type");
        }
      }
    else if ( this->m_UseGradientType == Fixed )
      {
      // Compute orientation-free gradient with calculator
      const CovariantVectorType fixedGradient =
        m_FixedImageGradientCalculatorVector[i]->EvaluateAtIndex(index);

      usedOrientFreeGradientTimes2.push_back(fixedGradient + fixedGradient);
      }
    else if ( this->m_UseGradientType == MappedMoving )
      {
      PointType mappedPoint;
      this->GetFixedImage()->TransformIndexToPhysicalPoint(index, mappedPoint);
      for ( unsigned int j = 0; j < ImageDimension; j++ )
        {
        mappedPoint[j] += it.GetCenterPixel()[j];
        }

      const CovariantVectorType mappedMovingGradient =
        m_MappedMovingImageGradientCalculatorVector[i]->Evaluate(mappedPoint);

      usedOrientFreeGradientTimes2.push_back(
        mappedMovingGradient + mappedMovingGradient);
      }
    else
      {
      itkExceptionMacro(<< "Unknown gradient type");
      }
    speedValue.push_back(fixedValue - movingValue);
    }

  std::vector< CovariantVectorType > usedGradientTimes2;
  usedGradientTimes2.reserve(10);

#ifdef ITK_USE_ORIENTED_IMAGE_DIRECTION

  for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
    {
    CovariantVectorType tempGradientTimes2;
    this->GetFixedImage()->TransformLocalVectorToPhysicalVector(
      usedOrientFreeGradientTimes2[i],
      tempGradientTimes2);
    usedGradientTimes2.push_back(tempGradientTimes2);
    }
#else
  for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
    {
    usedGradientTimes2.push_back(usedOrientFreeGradientTimes2[i]);
    }
#endif

  /**
    * Compute Update.
    * We avoid the mismatch in units between the two terms.
    * and avoid large step using a normalization term.
    */

  CovariantVectorType tempGradient = usedGradientTimes2[0];
  double              sum_speedValue = speedValue[0];
  double              sqr_speedValue = vnl_math_sqr(speedValue[0]);
  for ( unsigned int i = 1; i < usedGradientTimes2.size(); ++i )
    {
    tempGradient += usedGradientTimes2[i];
    sum_speedValue += speedValue[i];
    sqr_speedValue += vnl_math_sqr(speedValue[i]);
    }
  const double usedGradientTimes2SquaredMagnitude = tempGradient.GetSquaredNorm();

  //  const double usedGradientTimes2SquaredMagnitude =
  // usedGradientTimes2.GetSquaredNorm();

  //  const double speedValue = fixedValue - movingValue;
  if ( vnl_math_abs(speedValue[0]) < m_IntensityDifferenceThreshold )
    {
    update.Fill(0.0);
    }
  else
    {
    double denom;
    if (  m_Normalizer > 0.0 )
      {
      // "ITK-Thirion" normalization
      denom =  usedGradientTimes2SquaredMagnitude
              + ( vnl_math_sqr(sum_speedValue) / m_Normalizer );
      }
    else
      {
      // least square solution of the system
      denom =  usedGradientTimes2SquaredMagnitude;
      }

    if ( denom < m_DenominatorThreshold )
      {
      update.Fill(0.0);
      }
    else
      {
      const double factor = 2.0 * sum_speedValue / denom;

      for ( unsigned int j = 0; j < ImageDimension; j++ )
        {
        update[j] = factor * tempGradient[j];
        }
      }
    }

  // WARNING!! We compute the global data without taking into account the
  // current update step.
  // There are several reasons for that: If an exponential, a smoothing or any
  // other operation
  // is applied on the update field, we cannot compute the newMappedCenterPoint
  // here; and even
  // if we could, this would be an often unnecessary time-consuming task.
  if ( globalData )
    {
    globalData->m_SumOfSquaredDifference += vnl_math_sqr(sqr_speedValue);
    globalData->m_NumberOfPixelsProcessed +=
      this->GetFixedImage()->GetVectorLength();
    globalData->m_SumOfSquaredChange += update.GetSquaredNorm();
    }

  return update;
}

/**
  * Update the metric and release the per-thread-global data.
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
VectorESMDemonsRegistrationFunction< TFixedImage, TMovingImage,
                                     TDeformationField >
::ReleaseGlobalDataPointer(void *gd) const
{
  GlobalDataStruct *globalData = (GlobalDataStruct *)gd;

  m_MetricCalculationLock.Lock();
  m_SumOfSquaredDifference += globalData->m_SumOfSquaredDifference;
  m_NumberOfPixelsProcessed += globalData->m_NumberOfPixelsProcessed;
  m_SumOfSquaredChange += globalData->m_SumOfSquaredChange;
  if ( m_NumberOfPixelsProcessed )
    {
    m_Metric = m_SumOfSquaredDifference
               / static_cast< double >( m_NumberOfPixelsProcessed );
    m_RMSChange = vcl_sqrt( m_SumOfSquaredChange
                            / static_cast< double >( m_NumberOfPixelsProcessed ) );
    }
  m_MetricCalculationLock.Unlock();

  delete globalData;
}
} // end namespace itk

#endif
