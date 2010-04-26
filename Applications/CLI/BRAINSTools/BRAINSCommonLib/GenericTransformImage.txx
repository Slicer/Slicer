#ifndef _GenericTransformImage_txx_
#define _GenericTransformImage_txx_

#include <iostream>
#include "GenericTransformImage.h"





template <class TransformType, class InterpolatorType>
typename ImageType::Pointer
TransformResample(
    typename ImageType::Pointer & inputImage,
    typename RefImageType::Pointer & ReferenceImage,
    typename ImageType::PixelType defaultValue,
    typename TransformType::Pointer & transform)
{
  typename InterpolatorType::Pointer interp = InterpolatorType::New();

  interp->SetInputImage(inputImage);

  typedef typename itk::ResampleImageFilter<ImageType, 
    ImageType> ResampleImageFilter;
  typename ResampleImageFilter::Pointer resample = ResampleImageFilter::New();
  resample->SetInput(inputImage);
  resample->SetTransform(transform);
  resample->SetInterpolator(interp);

  resample->SetOutputParametersFromImage(ReferenceImage);
  // should happen automatically, but make sure orientation matches input
  resample->SetOutputDirection( ReferenceImage->GetDirection() );
  resample->SetDefaultPixelValue( defaultValue );
  resample->Update();
  typename ImageType::Pointer returnval = resample->GetOutput();
  returnval->DisconnectPipeline();
  return returnval;
}





template <typename ImageType,
    typename ReferenceImageType,
    typename DeformationImageType>
typename ImageType::Pointer GenericTransformImage( 
    typename ImageType::Pointer OperandImage,
    typename ReferenceImageType::Pointer ReferenceImage,
    typename DeformationImageType::Pointer DeformationField,
    typename ImageType::PixelType defaultValue,
    typename BSplineTransformType::Pointer itkBSplineTransform,
    typename AffineTransformType::Pointer ITKAffineTransform,
    const std::string &interpolationMode,
    bool binaryFlag)
{


    // FIRST will need to convert binary image to signed distance in case binaryFlag is true. 
    
    typename ImageType::Pointer PrincipalOperandImage;
     
    // Splice in a case for dealing with binary images,
    // where signed distance maps are warped and thresholds created.
    if ( binaryFlag )
      {
      if ( interpolationMode == "NearestNeighbor" )
        {
        std::cout << "WARNING:  Using NearestNeighbor and SignedDistance" << std::endl
                  << "          for binary images is an unlikely combination." << std::endl
                  << "          you probably want Linear interpolationMode for" << std::endl
                  << "          the signed distance map implied by your choice" << std::endl
                  << "          of pixelType binary." << std::endl;
        }
      /* We make the values inside the structures positive and outside negative using
        BinaryThresholdImageFilter. As the lower and upper threshold values are 0
        only values of 0 in the image are filled with 0.0 and other values are 1.0
        */

      typedef itk::BinaryThresholdImageFilter<ImageType,
              ImageType> FloatThresholdFilterType;
      typename FloatThresholdFilterType::Pointer initialFilter
        = FloatThresholdFilterType::New();
      initialFilter->SetInput( OperandImage );
        {
        const PixelType outsideValue = 1.0;
        const PixelType insideValue  = 0.0;
        initialFilter->SetOutsideValue( outsideValue );
        initialFilter->SetInsideValue(  insideValue  );
        const PixelType lowerThreshold = 0;
        const PixelType upperThreshold = 0;
        initialFilter->SetLowerThreshold( lowerThreshold );
        initialFilter->SetUpperThreshold( upperThreshold );
        }
      initialFilter->Update();
        {
        typedef itk::SignedMaurerDistanceMapImageFilter<ImageType,
                ImageType> DistanceFilterType;
        typename DistanceFilterType::Pointer DistanceFilter = DistanceFilterType::New();
        DistanceFilter->SetInput( initialFilter->GetOutput () );
        // DistanceFilter->SetNarrowBandwidth( m_BandWidth );
        DistanceFilter->SetInsideIsPositive( true );
        DistanceFilter->SetUseImageSpacing( true );
        DistanceFilter->SetSquaredDistance( false );

        DistanceFilter->Update();
        PrincipalOperandImage = DistanceFilter->GetOutput();
        //PrincipalOperandImage->DisconnectPipeline();
        }
#if 1
        // Using defaultValue based on the size of the image so that intensity values
        // are kept to a reasonable range.  (A costlier way calculates the image min.)
        const typename ImageType::SizeType size = PrincipalOperandImage->GetLargestPossibleRegion().GetSize();
        const typename ImageType::SpacingType spacing = PrincipalOperandImage->GetSpacing();
        double diagonalLength=0;
        for(int s=0;s< ImageType::ImageDimension ; s++)
          {
          diagonalLength += size[s]*spacing[s];
          }
        // Consider the 3D diagonal value, to guarantee that the background filler is
        // unlikely to add shapes to the thresholded signed distance image.
        // This is an easy enough proof of a lower bound on the image min, since it
        // works even if the mask is a single voxel in the image field corner.
        //        defaultValue=vcl_sqrt( diagonalLength );
        // In most cases, a heuristic fraction of the diagonal value is an even better
        // lower bound: if the midpoint of the image is inside the mask, 1/2 is a lower
        // bound as well, and the background is unlikely to drive the upper limit of the
        // intensity range when we visualize the intermediate image for debugging.

        defaultValue=-vcl_sqrt( diagonalLength )*0.5; 
#else
        // To denote the least possible value, the lowest intensity, must avoid the
        // pitfall of using the PixelType min();  min() is an epsilon, similar to 
        // machine precision.  The lowest conceivable PixelType value is -max().
        defaultValue = -vcl_numeric_limits<PixelType>::max();
#endif
      }
    else // other than if (pixelType == "binary")
      {
      PrincipalOperandImage = OperandImage;
      }



  // RESAMPLE with the appropriate transform and interpolator:

  typename ImageType::Pointer TransformedImage;  // One name for the intermediate resampled float image.

  if ( DeformationField.IsNull() ) // (useTransform)
    {
      std::cout<< " Deformation Field is Null... " << std::endl; 
    if ( itkBSplineTransform.IsNotNull() ) // (definitelyBSpline)
      {    
        std::cout<<" BSplint Transform is applied...." << std::endl;
      if ( interpolationMode == "NearestNeighbor" )
        {
        typedef typename itk::NearestNeighborInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<BSplineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          itkBSplineTransform);
        }
      else if ( interpolationMode == "Linear" )
        {
        typedef typename itk::LinearInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<BSplineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          itkBSplineTransform);
        }
      else if ( interpolationMode == "BSpline" )
        {
        typedef typename itk::BSplineInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<BSplineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          itkBSplineTransform);
        }
      else if ( interpolationMode == "WindowedSinc" )
        {
        typedef typename itk::ConstantBoundaryCondition<ImageType>
                  BoundaryConditionType;
        static const unsigned int WindowedSincHammingWindowRadius = 5;
        typedef typename itk::Function::HammingWindowFunction<
          WindowedSincHammingWindowRadius, double, double> WindowFunctionType;
        typedef typename itk::WindowedSincInterpolateImageFunction<
          ImageType,
          WindowedSincHammingWindowRadius,
          WindowFunctionType,
          BoundaryConditionType,
          double>   WindowedSincInterpolatorType;

        TransformedImage = TransformResample<BSplineTransformType,WindowedSincInterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          itkBSplineTransform);
        }
      else
        {
        std::cout << "Error: Invalid interpolation mode specified" << std::endl;
        std::cout << "\tValid modes: NearestNeighbor, Linear, BSpline, WindowedSinc"
          << std::endl;
        }
      }
    else if ( ITKAffineTransform.IsNotNull() )// definitely Linear, and already converted to Affine in ITKAffineTransform:
      {
      std::cout<<"itk Affine Transform is not Nulli, Affine Trasnform applied..." << std::endl;
      if ( interpolationMode == "NearestNeighbor" )
        {
        typedef typename itk::NearestNeighborInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<AffineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          ITKAffineTransform);
        }
      else if ( interpolationMode == "Linear" )
        {
        typedef typename itk::LinearInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<AffineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          ITKAffineTransform);
        }
      else if ( interpolationMode == "BSpline" )
        {
        typedef typename itk::BSplineInterpolateImageFunction<ImageType, double> 
                  InterpolatorType;

        TransformedImage = TransformResample<AffineTransformType,InterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          ITKAffineTransform);
        }
      else if ( interpolationMode == "WindowedSinc" )
        {
        typedef typename itk::ConstantBoundaryCondition<ImageType>
                  BoundaryConditionType;
        static const unsigned int WindowedSincHammingWindowRadius = 5;
        typedef typename itk::Function::HammingWindowFunction<
          WindowedSincHammingWindowRadius, double, double> WindowFunctionType;
        typedef typename itk::WindowedSincInterpolateImageFunction<
          ImageType,
          WindowedSincHammingWindowRadius,
          WindowFunctionType,
          BoundaryConditionType,
          double>   WindowedSincInterpolatorType;

        TransformedImage = TransformResample<AffineTransformType,WindowedSincInterpolatorType>(
          PrincipalOperandImage,
          ReferenceImage,
          defaultValue,
          ITKAffineTransform);
        }
      else
        {
        std::cout << "Error: Invalid interpolation mode specified" << std::endl;
        std::cout << "\tValid modes: NearestNeighbor, Linear, BSpline, WindowedSinc"
          << std::endl;
        }
      }
    }

  else if ( DeformationField.IsNotNull() )
    {
    //  std::cout<< "Deformation Field is given, so applied to the image..." << std::endl;
    typedef typename itk::WarpImageFilter<ImageType,
            ImageType,
            DeformationImageType>  FilterType;

    typename FilterType::Pointer warper = FilterType::New();

    if ( interpolationMode == "NearestNeighbor" )
      {
      typedef typename itk::NearestNeighborInterpolateImageFunction<ImageType, double> 
                InterpolatorType;
      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
      warper->SetInterpolator( interpolator );
      }
    else if ( interpolationMode == "Linear" )
      {
      typedef typename itk::LinearInterpolateImageFunction<ImageType, double>
                InterpolatorType;
      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
      warper->SetInterpolator( interpolator );
      }
    else if ( interpolationMode == "BSpline" )
      {
      typedef typename itk::BSplineInterpolateImageFunction<ImageType, double> 
                InterpolatorType;

      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
      warper->SetInterpolator( interpolator );
      }
    else if ( interpolationMode == "WindowedSinc" )
      {
      typedef typename itk::ConstantBoundaryCondition<ImageType>
      BoundaryConditionType;
      static const unsigned int WindowedSincHammingWindowRadius = 5;
      typedef typename itk::Function::HammingWindowFunction<
        WindowedSincHammingWindowRadius, double, double> WindowFunctionType;
      typedef typename itk::WindowedSincInterpolateImageFunction<
        ImageType,
        WindowedSincHammingWindowRadius,
        WindowFunctionType,
        BoundaryConditionType,
        double>    WindowedSincInterpolatorType;

      typename WindowedSincInterpolatorType::Pointer interpolator = WindowedSincInterpolatorType::New();
      warper->SetInterpolator( interpolator );
      }
    else
      {
      std::cout << "Error: Invalid interpolation mode specified" << std::endl;
      std::cout << "\tValid modes: NearestNeighbor, Linear, BSpline, WindowedSinc"
        << std::endl;
      }


    if ( ReferenceImage.IsNotNull() )
      {
        /*
        std::cout<< " Give Reference image's Spacing, Origin and Direction...."<<std::endl;
        std::cout<< " --- Spacing::   " <<  ReferenceImage->GetSpacing()  << std::endl;
        std::cout<< " --- Origin::    " <<  ReferenceImage->GetOrigin()   << std::endl;
        std::cout<< " --- Direction:: " <<  ReferenceImage->GetDirection()<< std::endl;
      */
      warper->SetOutputSpacing(    ReferenceImage->GetSpacing() );
      warper->SetOutputOrigin(     ReferenceImage->GetOrigin() );
      warper->SetOutputDirection(  ReferenceImage->GetDirection() );
      }
    else 
      {
      std::cout << "Alert:  missing Reference Volume information defaulted from Deformation Volume " << std::endl;

      warper->SetOutputSpacing(    DeformationField->GetSpacing() );
      warper->SetOutputOrigin(     DeformationField->GetOrigin() );
      warper->SetOutputDirection(  DeformationField->GetDirection() );
      }

    warper->SetDeformationField( DeformationField );
    warper->SetEdgePaddingValue( static_cast<PixelType>( defaultValue ) );
    warper->SetInput( PrincipalOperandImage );
    warper->Update( );

    TransformedImage = warper->GetOutput();
    }



    // FINALLY will need to convert signed distance to binary image in case binaryFlag is true. 

    typename ImageType::Pointer FinalTransformedImage;

    if ( binaryFlag )
      {
      // A special case for dealing with binary images
      // where signed distance maps are warped and thresholds created
      typedef short int                             MaskPixelType;
      typedef typename itk::Image<MaskPixelType,  GenericTransformImageNS::SpaceDimension> MaskImageType;

      //Now Threshold and write out image
      typedef typename itk::BinaryThresholdImageFilter<ImageType,
              MaskImageType> BinaryThresholdFilterType;
      typename BinaryThresholdFilterType::Pointer finalFilter
        = BinaryThresholdFilterType::New();
      finalFilter->SetInput( TransformedImage );

      const MaskPixelType outsideValue = 0;
      const MaskPixelType insideValue  = 1;
      finalFilter->SetOutsideValue( outsideValue );
      finalFilter->SetInsideValue(  insideValue  );
      // Signed distance boundary voxels are defined as being included in the
      // structure,  therefore the desired distance threshold is in the middle
      // of the enclosing (negative) voxel ribbon around threshold 0.
      const typename ImageType::SpacingType Spacing = ReferenceImage->GetSpacing();
      const PixelType lowerThreshold = -0.5 * 0.333333333333
        * ( Spacing[0] + Spacing[1] + Spacing[2] );
      //  std::cerr << "Lower Threshold == " << lowerThreshold << std::endl;

      const PixelType upperThreshold = vcl_numeric_limits<PixelType>::max();
      finalFilter->SetLowerThreshold( lowerThreshold );
      finalFilter->SetUpperThreshold( upperThreshold );

      finalFilter->Update();

      typedef typename itk::CastImageFilter<MaskImageType, ImageType> CastImageFilter;
      typename CastImageFilter::Pointer castFilter = CastImageFilter::New();
      castFilter->SetInput( finalFilter->GetOutput() );
      castFilter->Update( );

      FinalTransformedImage = castFilter->GetOutput();
      }
    else
      {
      FinalTransformedImage = TransformedImage;
      }

    return FinalTransformedImage;
}



#endif
