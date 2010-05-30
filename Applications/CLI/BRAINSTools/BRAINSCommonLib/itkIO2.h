#ifndef _ITKIO2_H_
#define _ITKIO2_H_

#include "itkWindowedSincInterpolateImageFunction.h"

#include "itkTransformFactory.h"

#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"

#include "ConvertToRigidAffine.h"
#include "itkTransformFileReader.h"
#include "ApplyField.h"

namespace itkUtil
{
  template <class ImageType>
    void Denormalize(typename ImageType::Pointer & input, int nx, int ny, int nz )

      {
      typedef typename itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( input, input->GetLargestPossibleRegion() );

      for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
        {
        in1.Set(in1.Get() * nx * ny * nz);
        }
      }

  template <class ImageType>
    void Normalize(typename ImageType::Pointer & input )

      {
      itk::ImageRegionIterator<ImageType> in1( input,
        input->GetLargestPossibleRegion() );

      for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
        {
        const typename ImageType::PixelType value
          = static_cast<typename ImageType::PixelType>( static_cast<float>( in1.Get() )
            / 255.0F );
        in1.Set(value);
        }
      }

  // This function will allocate a new image with all the same ImageBase
  // information (i.e. orientation, size, resolution, origin, MetaData).
  template <class OutputImageType, class InputImagePointerType>
    typename OutputImageType::Pointer
    AllocateSimilarImage(InputImagePointerType ReferenceImage)
      {
      // create a OutputImage containing the upper left hand corner and
      // a chunk out of the middle
      typename OutputImageType::Pointer OutputImage = OutputImageType::New();
      OutputImage->SetRegions( ReferenceImage->GetLargestPossibleRegion() );
      OutputImage->CopyInformation(ReferenceImage);
      OutputImage->Allocate();
      OutputImage->FillBuffer(itk::NumericTraits<typename OutputImageType::
        PixelType>::Zero);
      return OutputImage;
      }

  typedef itk::Image<itk::Vector<float, 3>, 3> TDeformationField;
  typedef itk::Image<float, 3>                 FloatImage3DType;

  template <class ImageType>
    void ImageClear(typename ImageType::Pointer & input )
      {
      typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( input, input->GetLargestPossibleRegion() );

      for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
        {
        in1.Set(0.0);
        }
      }

  template <class ImageType>
    float GetMax( const typename ImageType::Pointer & rhs)
      {
      typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

      in1.GoToBegin();
      typename ImageType::PixelType maxvalue = in1.Get();
      // first element read above, skip it and look at rest of image.
      for ( ++in1; !in1.IsAtEnd(); ++in1 )
        {
        typename ImageType::PixelType current = in1.Get();
        maxvalue = ( current > maxvalue ) ? current : maxvalue;
        }
      return maxvalue;
      }

  template <class ImageType>
    float GetMin( const typename ImageType::Pointer & rhs)
      {
      typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

      in1.GoToBegin();
      typename ImageType::PixelType minvalue = in1.Get();

      // first element read above, skip it and look at rest of image.
      for ( ++in1; !in1.IsAtEnd(); ++in1 )
        {
        const typename ImageType::PixelType current = in1.Get();
        minvalue = ( current < minvalue ) ? current : minvalue;
        }
      return minvalue;
      }

  // versions of GetMin and GetMax for images with vector pixeltype
  template <class ImageType>
    float GetMax( const typename ImageType::Pointer & rhs, unsigned Index)
      {
      typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

      in1.GoToBegin();
      typename ImageType::PixelType maxvalue = in1.Get();
      for ( ++in1; !in1.IsAtEnd(); ++in1 )
        {
        typename ImageType::PixelType current = in1.Get();
        maxvalue[Index]
          = ( current[Index] > maxvalue[Index] ) ? current[Index] : maxvalue[Index];
        }
      return maxvalue[Index];
      }

  template <class ImageType>
    float GetMin( const typename ImageType::Pointer & rhs, unsigned Index)
      {
      typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
      ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

      in1.GoToBegin();
      typename ImageType::PixelType minvalue = in1.Get();

      for ( ++in1; !in1.IsAtEnd(); ++in1 )
        {
        const typename ImageType::PixelType current = in1.Get();
        minvalue[Index]
          = ( current[Index] < minvalue[Index] ) ? current[Index] : minvalue[Index];
        }
      return minvalue[Index];
      }
}

// a nasty hack to get around recursive dependencies between
// itkIO.h and TransformToDeformationField.h
#include "TransformToDeformationField.h"

namespace itkUtil
{
  /** AllocateImageFromExample creates and allocates an image of the type OutputImageType,
   * using TemplateImageType as the source of size and spacing...
   *
   */
  template <class TemplateImageType, class OutputImageType>
    typename OutputImageType::Pointer
    AllocateImageFromExample(
      const typename TemplateImageType::Pointer & TemplateImage)
      {
      typename OutputImageType::Pointer rval = OutputImageType::New();
      rval->CopyInformation(TemplateImage);
      rval->SetRegions( TemplateImage->GetLargestPossibleRegion() );
      rval->Allocate();
      return rval;
      }

  //
  // convenience function where template and output images type are the same
  template <class ImageType>
    typename ImageType::Pointer
    AllocateImageFromExample(const typename ImageType::Pointer & TemplateImage)
      {
      return AllocateImageFromExample<ImageType, ImageType>(TemplateImage);
      }

#if 0
  template <class WarperImageType>
    typename WarperImageType::Pointer MIWarper(
      const std::string & SubjToAtlasRegistrationFilename,
      const std::string & MaskName)
      {
      itk::AffineTransform<double,
      3>::Pointer transformObject
        = itkUtil::ReadITKDesiredTransform<itk::AffineTransform<double, 3> >(
          SubjToAtlasRegistrationFilename);

      typename WarperImageType::Pointer InputImage
        = itkUtil::ReadImage<WarperImageType>(MaskName);
      if ( InputImage.IsNull() )
        {
        std::cerr << "failed to read " << MaskName << std::endl;
        throw;
        }
      typedef itk::ResampleImageFilter<
        WarperImageType,
        WarperImageType>    ResampleFilterType;

      typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();

      resampler->SetTransform( transformObject );
      resampler->SetInput( InputImage );
      // TODO:  Verify that this is always a mapping onto the same space!
      // TODO:  Since this is mapping of masks, it should use a signed distance
      // mapping.
      // TODO:  or at least a nearest neighbor. Unless a probability map is being
      // generated.
      // TODO:  in which case it should just be nearest neighbor.
      resampler->SetOutputParametersFromImage( InputImage );
      typename WarperImageType::PixelType DefaultPixelValue(
        static_cast<typename WarperImageType::PixelType>( 0 ) );
      resampler->SetDefaultPixelValue( DefaultPixelValue );
      try
        {
        resampler->Update();
        }
      catch ( itk::ExceptionObject & e )
        {
        std::cerr << "Exception in resampling from transformation." << std::endl;
        std::cerr << e.GetDescription() << std::endl;
        std::cerr << e.GetLocation() << std::endl;
        exit(-1);
        }

      typename WarperImageType::Pointer returnImage = resampler->GetOutput();
      // std::cout << returnImage << std::endl;
      std::cout << "Registration Complete for: " << MaskName << "." << std::endl;
      return returnImage;
      }

  template <class WarperImageType>
    typename WarperImageType::Pointer TDWarper(
      const std::string & SubjToAtlasRegistrationFilename,
      const std::string & MaskName)
      {
      typedef itk::Image<itk::Vector<float, 3>, 3> TDWarperTDeformationFieldType;
      try
        {
        TDWarperTDeformationFieldType::Pointer SubjectToAtlasDeformationField
          = itkUtil::ReadImage<TDWarperTDeformationFieldType>(
            SubjToAtlasRegistrationFilename.c_str() );
        typename itk::ApplyField<TDWarperTDeformationFieldType, WarperImageType,
                 WarperImageType>::Pointer applyfield
                   = itk::ApplyField<TDWarperTDeformationFieldType, WarperImageType,
                 WarperImageType>::New();
        applyfield->SetDeformationField(SubjectToAtlasDeformationField);
        typename WarperImageType::Pointer MaskImage
          = itkUtil::ReadImage<WarperImageType>(MaskName);
        typename itk::MinimumMaximumImageCalculator<WarperImageType>::Pointer
          MinMaxFilter = itk::MinimumMaximumImageCalculator<WarperImageType>::New();
        MinMaxFilter->SetImage(MaskImage);
        MinMaxFilter->Compute();
        std::cout << "Image " << MaskName << " [min,max]= ["
          << static_cast<double>( MinMaxFilter->GetMinimum() ) << ","
          << static_cast<double>( MinMaxFilter->GetMaximum() ) << "]."
          << std::endl;

        // VerifyNonZeroImage<UCHARImageType>(MaskImage,MaskName);
        applyfield->SetInputImage(MaskImage);
        applyfield->Execute();
        std::cout << "Mask Registration Complete." << std::endl;
        typename WarperImageType::Pointer OutputImage = applyfield->GetOutputImage();
        MinMaxFilter->SetImage(OutputImage);
        MinMaxFilter->Compute();
        std::cout << "Image " << MaskName << " [min,max]= ["
          << static_cast<double>( MinMaxFilter->GetMinimum() ) << ","
          << static_cast<double>( MinMaxFilter->GetMaximum() ) << "]."
          << std::endl;
        const bool PrintVerbose = false; // HACK : This should really be something
        // controlled by the command line.
        if ( PrintVerbose == true )
          {
          std::cout << "===============" << std::endl;
          std::cout << SubjectToAtlasDeformationField << std::endl;
          std::cout << "===============" << std::endl;
          std::cout << MaskImage << std::endl;
          std::cout << "===============" << std::endl;
          std::cout << OutputImage << std::endl;
          }
        return OutputImage;
        }
      catch ( itk::ExceptionObject & e )
        {
        std::cerr << "Exception in Registration." << std::endl;
        std::cerr << e.GetDescription() << std::endl;
        std::cerr << e.GetLocation() << std::endl;
        exit(-1);
        }
      return 0;
      }
#endif
}
#endif // __ITKIO2.h
