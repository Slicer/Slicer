#ifndef __BRAINSFitBSpline_h
#define __BRAINSFitBSpline_h

#include <itkImageToImageMetric.h>

#include <itkBSplineDeformableTransform.h>
#include "itkBSplineDeformableTransformInitializer.h"
#include <itkLBFGSBOptimizer.h>
#include <itkTimeProbesCollectorBase.h>
#include <itkImageRegistrationMethod.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>

#include "genericRegistrationHelper.h"

/**
  * This class is the BSpline component of the BRAINSFit program developed at
  *the University of Iowa.
  * The master version of this file is always located from the nitric site.
  * http://www.nitrc.org/projects/multimodereg/
  */

template< class RegisterImageType, class ImageMaskSpatialObjectType, class BSplineTransformType >
typename BSplineTransformType::Pointer
DoBSpline(typename BSplineTransformType::Pointer InitializerBsplineTransform,
          typename RegisterImageType::Pointer m_FixedVolume,
          typename RegisterImageType::Pointer m_MovingVolume,
          typename itk::ImageToImageMetric<
                 RegisterImageType, RegisterImageType >::Pointer CostMetricObject,
          const double m_MaxBSplineDisplacement,
          const float m_CostFunctionConvergenceFactor,
          const float m_ProjectedGradientTolerance,
          const bool m_DisplayDeformedImage,
          const bool m_PromptUserAfterDisplay)
{
  /*
    *  Begin straightline BSpline optimization, after
    *GTRACT/Common/itkAnatomicalBSplineFilter.
    */

  typedef typename RegisterImageType::Pointer       RegisterImagePointer;
  typedef typename RegisterImageType::ConstPointer  RegisterImageConstPointer;
  typedef typename RegisterImageType::RegionType    RegisterImageRegionType;
  typedef typename RegisterImageType::SizeType      RegisterImageSizeType;
  typedef typename RegisterImageType::SpacingType   RegisterImageSpacingType;
  typedef typename RegisterImageType::PointType     RegisterImagePointType;
  typedef typename RegisterImageType::PixelType     RegisterImagePixelType;
  typedef typename RegisterImageType::DirectionType RegisterImageDirectionType;
  typedef typename RegisterImageType::IndexType     RegisterImageIndexType;

  typedef typename BSplineTransformType::RegionType     TransformRegionType;
  typedef typename TransformRegionType::SizeType        TransformSizeType;
  typedef typename BSplineTransformType::SpacingType    TransformSpacingType;
  typedef typename BSplineTransformType::OriginType     TransformOriginType;
  typedef typename BSplineTransformType::DirectionType  TransformDirectionType;
  typedef typename BSplineTransformType::ParametersType TransformParametersType;

  typedef typename itk::LBFGSBOptimizer OptimizerType;

  typedef typename itk::LinearInterpolateImageFunction<
    RegisterImageType,
    double >          InterpolatorType;

  typedef typename itk::ImageRegistrationMethod<
    RegisterImageType,
    RegisterImageType >          RegistrationType;

  typedef typename BSplineTransformType::Pointer     TransformTypePointer;
  typedef typename OptimizerType::Pointer            OptimizerTypePointer;
  typedef typename OptimizerType::ParametersType     OptimizerParameterType;
  typedef typename OptimizerType::ScalesType         OptimizerScalesType;
  typedef typename OptimizerType::BoundSelectionType OptimizerBoundSelectionType;
  typedef typename OptimizerType::BoundValueType     OptimizerBoundValueType;

  typedef typename InterpolatorType::Pointer InterpolatorTypePointer;
  typedef typename RegistrationType::Pointer RegistrationTypePointer;

  typedef typename itk::ResampleImageFilter<
    RegisterImageType,
    RegisterImageType >     ResampleFilterType;

  // TODO:  Expose these to the command line for consistancy.
  const int m_MaximumNumberOfIterations = 1500;

  const int m_MaximumNumberOfEvaluations = 900;
  const int m_MaximumNumberOfCorrections = 12;

  OptimizerTypePointer    optimizer     = OptimizerType::New();
  InterpolatorTypePointer interpolator  = InterpolatorType::New();
  RegistrationTypePointer registration  = RegistrationType::New();

  typename BSplineTransformType::Pointer m_OutputBSplineTransform = BSplineTransformType::New();
  m_OutputBSplineTransform->SetIdentity();
  m_OutputBSplineTransform->SetBulkTransform( InitializerBsplineTransform->GetBulkTransform() );
  m_OutputBSplineTransform->SetFixedParameters( InitializerBsplineTransform->GetFixedParameters() );
  m_OutputBSplineTransform->SetParametersByValue( InitializerBsplineTransform->GetParameters() );

  /** Set up the Registration */
  registration->SetMetric(CostMetricObject);
  registration->SetOptimizer(optimizer);
  registration->SetInterpolator(interpolator);
  registration->SetTransform(m_OutputBSplineTransform);

  /** Setup the Registration */
  registration->SetFixedImage(m_FixedVolume);
  registration->SetMovingImage(m_MovingVolume);

  RegisterImageRegionType fixedImageRegion = m_FixedVolume->GetBufferedRegion();

  registration->SetFixedImageRegion(fixedImageRegion);

  registration->SetInitialTransformParameters( m_OutputBSplineTransform->GetParameters() );

  OptimizerBoundSelectionType boundSelect( m_OutputBSplineTransform->GetNumberOfParameters() );
  OptimizerBoundValueType     upperBound( m_OutputBSplineTransform->GetNumberOfParameters() );
  OptimizerBoundValueType     lowerBound( m_OutputBSplineTransform->GetNumberOfParameters() );

  /**
    *
    * Set the boundary condition for each variable, where
    * select[i] = 0 if x[i] is unbounded,
    *           = 1 if x[i] has only a lower bound,
    *           = 2 if x[i] has both lower and upper bounds, and
    *           = 3 if x[1] has only an upper bound
    */
  // TODO:  For control points outside the fixed image mask, it might be good to
  // constrian
  // the parameters to something different than those control points inside the
  // fixed image mask.
  if ( vcl_abs(m_MaxBSplineDisplacement) < 1e-12 )
    {
    boundSelect.Fill(0);
    }
  else
    {
    boundSelect.Fill(2);
    }
  upperBound.Fill(m_MaxBSplineDisplacement);
  lowerBound.Fill(-m_MaxBSplineDisplacement);
  optimizer->SetBoundSelection(boundSelect);
  optimizer->SetUpperBound(upperBound);
  optimizer->SetLowerBound(lowerBound);

  optimizer->SetCostFunctionConvergenceFactor(m_CostFunctionConvergenceFactor);
  optimizer->SetProjectedGradientTolerance(m_ProjectedGradientTolerance);
  optimizer->SetMaximumNumberOfIterations(m_MaximumNumberOfIterations);
  optimizer->SetMaximumNumberOfEvaluations(m_MaximumNumberOfEvaluations);
  optimizer->SetMaximumNumberOfCorrections(m_MaximumNumberOfCorrections);


  // Create the Command observer and register it with the optimizer.
  // TODO:  make this output optional.
  //

  const bool ObserveIterations = true;
  if ( ObserveIterations == true )
    {
    typedef BRAINSFit::CommandIterationUpdate< OptimizerType, BSplineTransformType, RegisterImageType >
    CommandIterationUpdateType;
    typename CommandIterationUpdateType::Pointer observer =
      CommandIterationUpdateType::New();
    observer->SetDisplayDeformedImage(m_DisplayDeformedImage);
    observer->SetPromptUserAfterDisplay(m_PromptUserAfterDisplay);
    observer->SetPrintParameters(false);
    observer->SetMovingImage(m_MovingVolume);
    observer->SetFixedImage(m_FixedVolume);
    observer->SetTransform(m_OutputBSplineTransform);
    optimizer->AddObserver(itk::IterationEvent(), observer);
    }

  /* Now start the execute function */

  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  std::cout << std::endl << "Starting Registration" << std::endl;

  try
    {
    collector.Start("Registration");
    registration->StartRegistration();
    collector.Stop("Registration");
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return NULL;
    }

  OptimizerType::ParametersType finalParameters =
    registration->GetLastTransformParameters();

  collector.Report();
  std::cout << "Stop condition from optimizer." << optimizer->GetStopConditionDescription() << std::endl;

  /* This call is required to copy the parameters */
  m_OutputBSplineTransform->SetParametersByValue(finalParameters);
  //  std::cout << "DELETEME:  " << finalParameters << std::endl;
  return m_OutputBSplineTransform;
}

#endif // __BRAINSFitBSpline_H_
