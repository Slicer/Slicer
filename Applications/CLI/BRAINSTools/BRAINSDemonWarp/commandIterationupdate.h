/*
  *  commandIterationupdate.h
  *  ThirionCLP
  *
  *  Created by Yong Qiang Zhao on 8/21/08.
  *  Copyright 2008 __MyCompanyName__. All rights reserved.
  *
  */
#include "itkCommand.h"
#include "itkPDEDeformableRegistrationFilter.h"

#include "itkMultiResolutionPDEDeformableRegistration.h"
#include "itkFastSymmetricForcesDemonsRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationWithMaskFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "GenericTransformImage.h"

#include "itkMultiResolutionLogDomainDeformableRegistration.h"
#include "itkLogDomainDemonsRegistrationFilter.h"
#include "itkSymmetricLogDomainDemonsRegistrationFilter.h"

#include "itkDisplacementFieldJacobianDeterminantFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkWarpHarmonicEnergyCalculator.h"
#include "itkVectorCentralDifferenceImageFunction.h"
// #include <getopt.h>
#include <iostream>

#include "BRAINSDemonWarpTemplates.h"

template< class TPixel = float, unsigned int VImageDimension = 3 >
class CommandIterationUpdate:public itk::Command
{
public:
  typedef  CommandIterationUpdate    Self;
  typedef  itk::Command              Superclass;
  typedef  itk::SmartPointer< Self > Pointer;

  typedef itk::Image< TPixel, VImageDimension >          InternalImageType;
  typedef itk::Vector< TPixel, VImageDimension >         VectorPixelType;
  typedef itk::Image< VectorPixelType, VImageDimension > DeformationFieldType;
  typedef itk::Image< VectorPixelType, VImageDimension > VelocityFieldType;

  typedef itk::DemonsRegistrationFilter<
    InternalImageType,
    InternalImageType,
    DeformationFieldType >   DemonsRegistrationFilterType;

  typedef itk::DiffeomorphicDemonsRegistrationFilter<
    InternalImageType,
    InternalImageType,
    DeformationFieldType >   DiffeomorphicDemonsRegistrationFilterType;

  typedef itk::DiffeomorphicDemonsRegistrationWithMaskFilter<
    InternalImageType,
    InternalImageType,
    DeformationFieldType >   DiffeomorphicDemonsRegistrationWithMaskFilterType;

  typedef itk::FastSymmetricForcesDemonsRegistrationFilter<
    InternalImageType,
    InternalImageType,
    DeformationFieldType >   FastSymmetricForcesDemonsRegistrationFilterType;

  typedef itk::LogDomainDeformableRegistrationFilter<
    InternalImageType,
    InternalImageType,
    VelocityFieldType >       LogDomainDeformableRegistrationFilterType;

  typedef itk::MultiResolutionLogDomainDeformableRegistration<
    InternalImageType, InternalImageType,
    VelocityFieldType, TPixel >         MultiLogResRegistrationFilterType;

  typedef itk::MultiResolutionPDEDeformableRegistration<
    InternalImageType, InternalImageType,
    DeformationFieldType, TPixel >   MultiResRegistrationFilterType;

  typedef itk::DisplacementFieldJacobianDeterminantFilter<
    DeformationFieldType, TPixel, InternalImageType > JacobianFilterType;

  typedef itk::MinimumMaximumImageCalculator< InternalImageType >
  MinMaxFilterType;

  typedef itk::WarpHarmonicEnergyCalculator< DeformationFieldType >
  HarmonicEnergyCalculatorType;

  typedef itk::VectorCentralDifferenceImageFunction< DeformationFieldType >
  WarpGradientCalculatorType;

  typedef typename WarpGradientCalculatorType::OutputType WarpGradientType;

  itkNewMacro(Self);
private:
  std::ofstream m_Fid;
  bool          m_headerwritten;
  typename JacobianFilterType::Pointer m_JacobianFilter;
  typename MinMaxFilterType::Pointer m_Minmaxfilter;
  typename HarmonicEnergyCalculatorType::Pointer m_HarmonicEnergyCalculator;
  typename DeformationFieldType::ConstPointer m_TrueField;
  typename WarpGradientCalculatorType::Pointer m_TrueWarpGradientCalculator;
  typename WarpGradientCalculatorType::Pointer m_CompWarpGradientCalculator;
  typename InternalImageType::Pointer m_MovingImage;
  typename InternalImageType::Pointer m_FixedImage;
public:
  void SetMovingImage(typename InternalImageType::Pointer & img)
  {
    m_MovingImage = img;
  }

  void SetFixedImage(typename InternalImageType::Pointer & img)
  {
    m_FixedImage = img;
  }

  void SetTrueField(const DeformationFieldType *truefield)
  {
    m_TrueField = truefield;

    m_TrueWarpGradientCalculator = WarpGradientCalculatorType::New();
    m_TrueWarpGradientCalculator->SetInputImage(m_TrueField);

    m_CompWarpGradientCalculator =  WarpGradientCalculatorType::New();
  }

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event );
  }

  void Execute(const itk::Object *object, const itk::EventObject & event)
  {
    if ( !( itk::IterationEvent().CheckEvent(&event) ) )
      {
      return;
      }

    typename DeformationFieldType::Pointer deffield = 0;
    unsigned int iter = vcl_numeric_limits< unsigned int >::max();
    double       metricbefore = -1.0;

    if ( const DiffeomorphicDemonsRegistrationFilterType * DDfilter =
           dynamic_cast< const DiffeomorphicDemonsRegistrationFilterType * >(
             object ) )
      {
      iter = DDfilter->GetElapsedIterations() - 1;
      metricbefore = DDfilter->GetMetric();
      deffield = const_cast< DiffeomorphicDemonsRegistrationFilterType * >
                 ( DDfilter )->GetDeformationField();
      }
    else if ( const DiffeomorphicDemonsRegistrationWithMaskFilterType * DDWMfilter =
                dynamic_cast< const
                              DiffeomorphicDemonsRegistrationWithMaskFilterType
                              * >(
                  object ) )
      {
      iter = DDWMfilter->GetElapsedIterations() - 1;
      metricbefore = DDWMfilter->GetMetric();
      deffield = const_cast< DiffeomorphicDemonsRegistrationWithMaskFilterType * >
                 ( DDWMfilter )->GetDeformationField();
      }
    else if ( const FastSymmetricForcesDemonsRegistrationFilterType * FSDfilter =
                dynamic_cast< const
                              FastSymmetricForcesDemonsRegistrationFilterType * >(
                  object ) )
      {
      iter = FSDfilter->GetElapsedIterations() - 1;
      metricbefore = FSDfilter->GetMetric();
      deffield = const_cast< FastSymmetricForcesDemonsRegistrationFilterType * >
                 ( FSDfilter )->GetDeformationField();
      }
    else if ( const DemonsRegistrationFilterType * Dfilter =
                dynamic_cast< const DemonsRegistrationFilterType * >( object ) )
      {
      iter = Dfilter->GetElapsedIterations() - 1;
      metricbefore = Dfilter->GetMetric();
      deffield = const_cast< DemonsRegistrationFilterType * >
                 ( Dfilter )->GetDeformationField();
      }
    else if ( const MultiResRegistrationFilterType * multiresfilter =
                dynamic_cast< const MultiResRegistrationFilterType * >( object ) )
      {
      std::cout << "Finished Multi-resolution iteration :"
                << multiresfilter->GetCurrentLevel() - 1 << std::endl;
      std::cout << "==============================" << std::endl << std::endl;
      }
    else if ( const LogDomainDeformableRegistrationFilterType * LDDfilter =
                dynamic_cast< const LogDomainDeformableRegistrationFilterType * >( object ) )
      {
      iter = LDDfilter->GetElapsedIterations() - 1;
      metricbefore = LDDfilter->GetMetric();
      deffield = const_cast< LogDomainDeformableRegistrationFilterType * >
                 ( LDDfilter )->GetDeformationField();
      }
    else if ( const MultiLogResRegistrationFilterType * multiresLfilter =
                dynamic_cast< const MultiLogResRegistrationFilterType * >( object ) )
      {
      std::cout << "Finished Multi-resolution iteration :" << multiresLfilter->GetCurrentLevel() - 1 << std::endl;
      std::cout << "==============================" << std::endl << std::endl;
      }
    else
      {
      return;
      }

    if ( deffield.IsNotNull() )
      {
      std::cout << iter << ": MSE " << metricbefore << " - ";

      double fieldDist = -1.0;
      double fieldGradDist = -1.0;
      double tmp;
      if ( m_TrueField )
        {
        typedef itk::ImageRegionConstIteratorWithIndex< DeformationFieldType >
        FieldIteratorType;
        FieldIteratorType currIter(
          deffield, deffield->GetLargestPossibleRegion() );
        FieldIteratorType trueIter(
          m_TrueField, deffield->GetLargestPossibleRegion() );

        m_CompWarpGradientCalculator->SetInputImage(deffield);

        fieldDist = 0.0;
        fieldGradDist = 0.0;
        for ( currIter.GoToBegin(), trueIter.GoToBegin();
              !currIter.IsAtEnd(); ++currIter, ++trueIter )
          {
          fieldDist += ( currIter.Value() - trueIter.Value() ).GetSquaredNorm();

          // No need to add Id matrix here as we do a substraction
          tmp = (
            ( m_CompWarpGradientCalculator->EvaluateAtIndex( currIter.GetIndex() )
              - m_TrueWarpGradientCalculator->EvaluateAtIndex( trueIter.
                                                               GetIndex() )
            ).GetVnlMatrix() ).frobenius_norm();
          fieldGradDist += tmp * tmp;
          }
        fieldDist = sqrt( fieldDist / (double)(
                            deffield->GetLargestPossibleRegion().
                            GetNumberOfPixels() ) );
        fieldGradDist = sqrt( fieldGradDist / (double)(
                                deffield->GetLargestPossibleRegion().
                                GetNumberOfPixels() ) );

        std::cout << "d(.,true) " << fieldDist << " - ";
        std::cout << "d(.,Jac(true)) " << fieldGradDist << " - ";
        }
#if defined( USE_DEBUG_IMAGE_VIEWER )
      if ( DebugImageDisplaySender.Enabled() )
        {
        DebugImageDisplaySender.SendImage< DeformationFieldType >(deffield, 0, 0);
        DebugImageDisplaySender.SendImage< DeformationFieldType >(deffield, 1, 1);
        DebugImageDisplaySender.SendImage< DeformationFieldType >(deffield, 2, 2);
#  if 0
        typedef typename itk::WarpImageFilter< InternalImageType,
                                               InternalImageType, DeformationFieldType > WarpFilterType;
        typename WarpFilterType::Pointer warper = WarpFilterType::New();
        warper->SetInput(m_MovingImage);
        warper->SetOutputSpacing( deffield->GetSpacing() );
        warper->SetOutputOrigin( deffield->GetOrigin() );
        warper->SetOutputDirection( deffield->GetDirection() );
        warper->SetDeformationField(deffield);
        warper->Update();
        typename InternalImageType::Pointer
        DeformedMovingImagePtr = warper->GetOutput();
#  else
        typename InternalImageType::Pointer
        DeformedMovingImagePtr =
          TransformWarp< InternalImageType, InternalImageType, DeformationFieldType >(
            m_MovingImage,
            deffield,
            0,
            GetInterpolatorFromString< RealImageType >("Linear"),
            deffield);
        DebugImageDisplaySender.SendImage< InternalImageType >(DeformedMovingImagePtr, 3);
        //        std::cerr << std::endl << "************IMAGES
        // SENT*************" << std::endl;
        }
#  endif
#endif  // defined(USE_DEBUG_IMAGE_VIEWER)

      m_HarmonicEnergyCalculator->SetImage(deffield);
      m_HarmonicEnergyCalculator->Compute();
      const double harmonicEnergy =
        m_HarmonicEnergyCalculator->GetHarmonicEnergy();
      std::cout << "harmo. " << harmonicEnergy << " - ";

      m_JacobianFilter->SetInput(deffield);
      m_JacobianFilter->UpdateLargestPossibleRegion();

      const unsigned int numPix = m_JacobianFilter->
                                  GetOutput()->GetLargestPossibleRegion().
                                  GetNumberOfPixels();

      TPixel *pix_start = m_JacobianFilter->GetOutput()->GetBufferPointer();
      TPixel *pix_end = pix_start + numPix;

      TPixel *jac_ptr;

      // Get percentage of det(Jac) below 0
      unsigned int jacBelowZero(0u);
      for ( jac_ptr = pix_start; jac_ptr != pix_end; ++jac_ptr )
        {
        if ( *jac_ptr <= 0.0 )
          {
          ++jacBelowZero;
          }
        }
      const double jacBelowZeroPrc = static_cast< double >( jacBelowZero )
                                     / static_cast< double >( numPix );

      // Get min an max jac
      const double minJac = *( std::min_element (pix_start, pix_end) );
      const double maxJac = *( std::max_element (pix_start, pix_end) );

      // Get some quantiles
      // We don't need the jacobian image
      // we can modify/sort it in place
      jac_ptr = pix_start + static_cast< unsigned int >( 0.002 * numPix );
      std::nth_element(pix_start, jac_ptr, pix_end);
      const double Q002 = *jac_ptr;

      jac_ptr = pix_start + static_cast< unsigned int >( 0.01 * numPix );
      std::nth_element(pix_start, jac_ptr, pix_end);
      const double Q01 = *jac_ptr;

      jac_ptr = pix_start + static_cast< unsigned int >( 0.99 * numPix );
      std::nth_element(pix_start, jac_ptr, pix_end);
      const double Q99 = *jac_ptr;

      jac_ptr = pix_start + static_cast< unsigned int >( 0.998 * numPix );
      std::nth_element(pix_start, jac_ptr, pix_end);
      const double Q998 = *jac_ptr;

      std::cout << "max|Jac| " << maxJac << " - "
                << "min|Jac| " << minJac << " - "
                << "ratio(|Jac|<=0) " << jacBelowZeroPrc << std::endl;

      if ( this->m_Fid.is_open() )
        {
        if ( !m_headerwritten )
          {
          this->m_Fid << "Iteration"
                      << ", MSE before"
                      << ", Harmonic energy"
                      << ", min|Jac|"
                      << ", 0.2% |Jac|"
                      << ", 01% |Jac|"
                      << ", 99% |Jac|"
                      << ", 99.8% |Jac|"
                      << ", max|Jac|"
                      << ", ratio(|Jac|<=0)";

          if ( m_TrueField )
            {
            this->m_Fid << ", dist(warp,true warp)"
                        << ", dist(Jac,true Jac)";
            }

          this->m_Fid << std::endl;

          m_headerwritten = true;
          }

        this->m_Fid << iter
                    << ", " << metricbefore
                    << ", " << harmonicEnergy
                    << ", " << minJac
                    << ", " << Q002
                    << ", " << Q01
                    << ", " << Q99
                    << ", " << Q998
                    << ", " << maxJac
                    << ", " << jacBelowZeroPrc;

        if ( m_TrueField )
          {
          this->m_Fid << ", " << fieldDist
                      << ", " << fieldGradDist;
          }

        this->m_Fid << std::endl;
        }
      }
  }

protected:
  CommandIterationUpdate():
    m_Fid("metricvalues.csv"),
    m_headerwritten(false)
  {
    m_JacobianFilter = JacobianFilterType::New();
    m_JacobianFilter->SetUseImageSpacing(true);
    m_JacobianFilter->ReleaseDataFlagOn();

    m_Minmaxfilter = MinMaxFilterType::New();

    m_HarmonicEnergyCalculator = HarmonicEnergyCalculatorType::New();

    m_TrueField = 0;
    m_TrueWarpGradientCalculator = 0;
    m_CompWarpGradientCalculator = 0;
  }

  ~CommandIterationUpdate()
  {
    this->m_Fid.close();
  }
};
