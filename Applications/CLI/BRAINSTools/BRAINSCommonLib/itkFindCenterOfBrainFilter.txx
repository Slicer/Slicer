#ifndef itkFindCenterOfBrainFilter_txx
#define itkFindCenterOfBrainFilter_txx
#include "itkFindCenterOfBrainFilter.h"
#include "itkNumericTraits.h"
#include "itkImageMomentsCalculator.h"
#include "itkImageDuplicator.h"
#include "itkImageMaskSpatialObject.h"

// #define USE_DEBUGGIN_IMAGES
#include "itksys/SystemTools.hxx"

namespace itk
{
template< class TInputImage, class TMaskImage >
FindCenterOfBrainFilter< TInputImage, TMaskImage >
::FindCenterOfBrainFilter():
  m_Maximize(true),
  m_Axis(2),
  m_OtsuPercentileThreshold(0.001),
  m_ClosingSize(7),
  m_HeadSizeLimit(1000),
  m_HeadSizeEstimate(0),
  m_BackgroundValue(NumericTraits< typename ImageType::PixelType >::Zero),
  m_GenerateDebugImages(false),
  // ITK smart pointers construct as null pointers, but
  // belt and suspenders initialization
  m_ImageMask(0),
  m_ClippedImageMask(0),
  m_TrimmedImage(0),
  m_DebugDistanceImage(0),
  m_DebugGridImage(0),
  m_DebugAfterGridComputationsForegroundImage(0),
  m_DebugClippedImageMask(0),
  m_DebugTrimmedImage(0)
{
  m_CenterOfBrain[0] = 0.0;
  m_CenterOfBrain[1] = 0.0;
  m_CenterOfBrain[2] = 0.0;
}

template< class TInputImage, class TMaskImage >
FindCenterOfBrainFilter< TInputImage, TMaskImage >
::~FindCenterOfBrainFilter()
{}

template< class TInputImage, class TMaskImage >
void
FindCenterOfBrainFilter< TInputImage, TMaskImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Maximize:                " << this->m_Maximize << std::endl;
  os << indent << "Axis:                    " << this->m_Axis     << std::endl;
  os << indent << "OtsuPercintileThreshold: " << this->m_OtsuPercentileThreshold << std::endl;
  os << indent << "ClosingSize:             " << this->m_ClosingSize << std::endl;
  os << indent << "HeadSizeLimit:           " << this->m_HeadSizeLimit << std::endl;
  os << indent << "HeadSizeEstimate:        " << this->m_HeadSizeEstimate << std::endl;
  os << indent << "BackgroundValue:         " << this->m_BackgroundValue << std::endl;
  os << indent << "CenterOfBrain:           " << this->m_CenterOfBrain << std::endl;
  os << indent << "ImageMask:               " << this->m_ImageMask << std::endl;
  os << indent << "TrimmedImage:            " << this->m_TrimmedImage << std::endl;
}

template< class TInputImage, class TMaskImage >
void
FindCenterOfBrainFilter< TInputImage, TMaskImage >
::AllocateOutputs()
{
  // Pass the input through as the output
  InputImagePointer image =
    const_cast< TInputImage * >( this->GetInput() );

  this->GraftOutput(image);
}

template< class TInputImage, class TMaskImage >
void
FindCenterOfBrainFilter< TInputImage, TMaskImage >
::GenerateData()
{
  if ( this->m_ImageMask.IsNull() )   // Need to autogenerate this
    {
    // //////////////////////////////////////////////////////////////////////
    typename LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
    LFF->SetInput( this->GetInput() );
    LFF->SetOtsuPercentileThreshold(this->m_OtsuPercentileThreshold);
    LFF->SetClosingSize(this->m_ClosingSize);
    LFF->Update();
    this->m_ImageMask = LFF->GetOutput();
    }
  typename MaskImageType::Pointer LFFimage;
    {
    typename itk::ImageDuplicator< MaskImageType >::Pointer id = itk::ImageDuplicator< MaskImageType >::New();
    id->SetInputImage(this->m_ImageMask);
    id->Update();
    LFFimage = id->GetOutput();
    }

  //  LFFimage will initially hold just a tissue LFFimage mask region (a
  // so-called integer mask),
  //  then it will code for the integer distance to the top of physical space,
  // only within the LFFimage mask region,
  //  then we will convert the zeros and distance codes to input image signal
  // and the assigned background value
  //  in conformity with the assigned volume-distance thresholding from a
  // histogram.

  // //////////////////////////////////////////////////////////////////////
  //  This will find maximum Superior physical location of all image voxels.
  double maxSIDirection;
    {
    typedef itk::ImageRegionIteratorWithIndex< MaskImageType > MaskIteratorType;
    MaskIteratorType ItPixel( LFFimage, LFFimage->GetLargestPossibleRegion() );

    typename MaskImageType::PointType PixelPhysicalPoint;
    ItPixel.Begin();
    LFFimage->TransformIndexToPhysicalPoint(ItPixel.GetIndex(), PixelPhysicalPoint);
    maxSIDirection = PixelPhysicalPoint[m_Axis];
    for (; !ItPixel.IsAtEnd(); ++ItPixel )
      {
      LFFimage->TransformIndexToPhysicalPoint(ItPixel.GetIndex(), PixelPhysicalPoint);
      if ( PixelPhysicalPoint[m_Axis] > maxSIDirection )
        {
        maxSIDirection = PixelPhysicalPoint[m_Axis];
        }
      }
    //  Now maxSIDirection is the level of the highest subspace plane in the
    // voxel array.
    }
  std::cout << "maxSIDirection = " << maxSIDirection << std::endl;

  // //////////////////////////////////////////////////////////////////////
  //  This will produce ForegroundLevel representing where to threshold the head
  // from the neck.
  // double ForegroundLevel = 1;
  typename DistanceImageType::Pointer distanceMap = DistanceImageType::New();
  distanceMap->CopyInformation(LFFimage);
  distanceMap->SetRegions( LFFimage->GetLargestPossibleRegion() );
  distanceMap->Allocate();
  distanceMap->FillBuffer(0.0);
    {
    typedef itk::ImageRegionIteratorWithIndex< MaskImageType > TInputIteratorType;
    TInputIteratorType ItPixel( LFFimage, LFFimage->GetLargestPossibleRegion() );

    //    typedef itk::ImageRegionIterator< DistanceImageType >
    // DistanceImageIteratorType;
    //    DistanceImageIteratorType ItDistPixel( distanceMap,
    // distanceMap->GetLargestPossibleRegion() );

    typename MaskImageType::PointType PixelPhysicalPoint;
    PixelPhysicalPoint.Fill(0.0);

    for ( ItPixel.Begin(); !ItPixel.IsAtEnd(); ++ItPixel )
      {
      if ( ItPixel.Get() != 0 )
        {
        const typename MaskImageType::IndexType tempIndex = ItPixel.GetIndex();
        LFFimage->TransformIndexToPhysicalPoint(tempIndex, PixelPhysicalPoint);
        double val =  vnl_math_rnd( vnl_math_abs(maxSIDirection
                                                 - PixelPhysicalPoint[m_Axis]) );
        distanceMap->SetPixel( tempIndex,
                               static_cast< typename DistanceImageType::PixelType >( val ) );
        }
      // else, leave the LFFimage coded zero, not some positive distance from
      // the top.
      }
    if ( this->m_GenerateDebugImages )
      {
      this->m_DebugDistanceImage = distanceMap;
      }
    }

  double inferiorCutOff = -1000000;
  std::cout << "Computing Sampled Distance Computations" << std::endl;
  const double samplingDistanceMM = 3.0;
  // Only look in 3mm regions
  const double samplingDistanceCM = samplingDistanceMM * 0.1;
  // Only look in 3mm regions
  const double rectangularGridRadius = 150.0;
  // How big of region around COM are we going to look for?
  const int numberOfSamplelingLines =
    static_cast< int >( ( rectangularGridRadius * 2.0 ) / samplingDistanceMM );

  if ( this->m_GenerateDebugImages )
    {
    this->m_DebugGridImage = TInputImage::New();
    this->m_DebugGridImage->CopyInformation(LFFimage);
    this->m_DebugGridImage->SetRegions( LFFimage->GetLargestPossibleRegion() );
    this->m_DebugGridImage->Allocate();
    this->m_DebugGridImage->FillBuffer(numberOfSamplelingLines);
    }

  // Use 400mm as the (-200mm,200mm) as the distance to look.
  std::vector< unsigned int > maskCountsInPlane(numberOfSamplelingLines);

  std::fill(maskCountsInPlane.begin(), maskCountsInPlane.end(), 0);
    {
    typename TInputImage::PointType CenterOfMass;
      {
        {
        /*
          * This will get an initial center of mass for the entire
          * foreground region.
          */
          {
          typedef typename itk::ImageMomentsCalculator< MaskImageType > momentsCalculatorType;
          typename momentsCalculatorType::Pointer moments = momentsCalculatorType::New();
          moments->SetImage(LFFimage);
          moments->Compute();
          typename TInputImage::PointType::VectorType tempCenterOfMass = moments->GetCenterOfGravity();
          for ( unsigned int q = 0; q < TInputImage::ImageDimension; q++ )
            {
            CenterOfMass[q] = tempCenterOfMass[q];
            }
          }
        }

      bool   exitCriteriaMet = false;
      double MaxVolumeBasedOnArea = 0.0;
      typename TInputImage::PointType rectPhysPoint;
      typename TInputImage::PointType currRotatedSampleGridLocation;
      for ( int dIndex = numberOfSamplelingLines - 1; dIndex >= 0; dIndex-- )
        {
        // Equally space the SI sampling around the COM for each index
        rectPhysPoint[2] =
          ( rectangularGridRadius
            * 2.0 )
          * ( static_cast< double >( dIndex
                                     + 1 ) / static_cast< double >( numberOfSamplelingLines ) ) - rectangularGridRadius;
        currRotatedSampleGridLocation[2] = CenterOfMass[2] + rectPhysPoint[2];
        for ( rectPhysPoint[1] = -rectangularGridRadius;
              rectPhysPoint[1] < rectangularGridRadius;
              rectPhysPoint[1] += samplingDistanceMM )
        // Raster through (-120mm,120mm)
          {
          currRotatedSampleGridLocation[1] = CenterOfMass[1] + rectPhysPoint[1];
          for ( rectPhysPoint[0] = -rectangularGridRadius;
                rectPhysPoint[0] < rectangularGridRadius;
                rectPhysPoint[0] += samplingDistanceMM )
          // Raster through (-120mm,120mm)
            {
            currRotatedSampleGridLocation[0] = CenterOfMass[0] + rectPhysPoint[0];

            typename TInputImage::IndexType currIndex;
            const bool isValidRegion = LFFimage->TransformPhysicalPointToIndex(
              currRotatedSampleGridLocation,
              currIndex);
            if ( isValidRegion && ( LFFimage->GetPixel(currIndex) > 0 ) )
              {
              // #ifdef USE_DEBUGGIN_IMAGES
              if ( this->m_GenerateDebugImages )
                {
                this->m_DebugGridImage->SetPixel( currIndex, ( numberOfSamplelingLines + dIndex ) );
                }
              maskCountsInPlane[dIndex]++;
              }
            // #ifdef USE_DEBUGGIN_IMAGES
            else if ( this->m_GenerateDebugImages && isValidRegion )
              {
              this->m_DebugGridImage->SetPixel(currIndex, 0);
              }
            }
          }

        const double crossSectionalArea = maskCountsInPlane[dIndex] * samplingDistanceCM * samplingDistanceCM;
        // Put this into cm^2
        const double crossSectionalVolume = crossSectionalArea * ( samplingDistanceCM );
        // Put this into cm^3
        const double estimated_radius = vcl_sqrt(crossSectionalArea / vnl_math::pi);
        // Estimate the radis of a circle filling this much space
        const double ScaleFactor = 1.1;
        // Add 10% for safety //5+(crossSectionalArea-200)/100; //Larger brains
        // need more scaling
        const double CurentVolumeBasedOnArea = ScaleFactor
                                               * ( 1.33333333333333333 * vnl_math::pi * estimated_radius
                                                   * estimated_radius * estimated_radius );
        // (4/3)*pi*r^3
        MaxVolumeBasedOnArea =
          ( CurentVolumeBasedOnArea > MaxVolumeBasedOnArea ) ? CurentVolumeBasedOnArea : MaxVolumeBasedOnArea;
        // Now compute 1.5 times the size of a sphere with this estimated
        // radius.
        // DesiredVolumeToIncludeBeforeClipping=CurentVolumeBasedOnArea;
        this->m_HeadSizeEstimate += crossSectionalVolume;
        if ( ( exitCriteriaMet == false ) && ( this->m_HeadSizeEstimate > this->m_HeadSizeLimit )
             && ( this->m_HeadSizeEstimate > MaxVolumeBasedOnArea ) )
          {
          exitCriteriaMet = true;
          inferiorCutOff = currRotatedSampleGridLocation[2];
          }

        if ( this->m_GenerateDebugImages )
          {
          std::cout << ( dIndex ) << ": " << maskCountsInPlane[dIndex]
                    << " Estimated Radius: " << estimated_radius
                    << " CurrentCalculatedVolumeBasedOnArea: " << CurentVolumeBasedOnArea
                    << " CummulativeVolume: " << this->m_HeadSizeEstimate
                    << " ExitCriteriaMet: " << exitCriteriaMet
                    << " Inferior Cut Off: " << currRotatedSampleGridLocation[2] << " " << inferiorCutOff
                    << std::endl;
          }
        }
      }
    }
  if ( this->m_GenerateDebugImages )
    {
      {
      this->m_DebugAfterGridComputationsForegroundImage = LFFimage;
      }
    }

  ////////////////////////////////////////////////////////////////////////
  // This will convert the LFFimage code image with the rule, foreach voxel,
  // if not in tissue LFFimage mask region or distance map is greater than T,
  // set the result image voxel to Background;
  // otherwise set the result image voxel to the source image pixel value.
    {
      {
      typename itk::ImageDuplicator< MaskImageType >::Pointer id = itk::ImageDuplicator< MaskImageType >::New();
      id->SetInputImage(this->m_ImageMask);
      id->Update();
      this->m_ClippedImageMask = id->GetOutput();
      }
    typedef typename itk::ImageRegionIteratorWithIndex< MaskImageType > MaskImageIteratorType;
    MaskImageIteratorType ClippedMaskPixel( this->m_ClippedImageMask,
                                            this->m_ClippedImageMask->GetLargestPossibleRegion() );

      {
      typename itk::ImageDuplicator< TInputImage >::Pointer id = itk::ImageDuplicator< TInputImage >::New();
      id->SetInputImage( this->GetInput() );
      id->Update();
      this->m_TrimmedImage = id->GetOutput();
      }

    typedef typename itk::ImageRegionIteratorWithIndex< TInputImage > TInputIteratorType;
    TInputIteratorType ClippedImagePixel( this->m_TrimmedImage, this->m_TrimmedImage->GetLargestPossibleRegion() );

    ClippedImagePixel.Begin();
    while ( ( !ClippedImagePixel.IsAtEnd() ) )
      {
      typename TInputImage::PointType currLoc;
      this->m_TrimmedImage->TransformIndexToPhysicalPoint(ClippedImagePixel.GetIndex(), currLoc);
      if ( currLoc[2] > inferiorCutOff && ( ClippedMaskPixel.Get() != 0 ) )
      // If this mask voxel is in the foreground AND above the inferiorCutOff
        {
        ClippedMaskPixel.Set(1);
        // putchar('1');
        }
      else
        {
        ClippedImagePixel.Set(this->m_BackgroundValue);
        ClippedMaskPixel.Set(0);
        // putchar('0');
        }
      ++ClippedImagePixel;
      ++ClippedMaskPixel;
      }
    }
  // #ifdef USE_DEBUGGIN_IMAGES
  if ( this->m_GenerateDebugImages )
    {
    this->m_DebugClippedImageMask =  this->m_ClippedImageMask;
    this->m_DebugTrimmedImage = this->m_TrimmedImage;
    }

  ////////////////////////////////////////////////////////////////////////
  //  This will use the clipped LFFimage image to get the head center of mass.
    {
    typedef typename itk::ImageMomentsCalculator< TInputImage > momentsCalculatorType;
    typename momentsCalculatorType::Pointer moments = momentsCalculatorType::New();
    moments->SetImage(this->m_TrimmedImage);
      {
      // convert mask image to mask
      typedef typename itk::ImageMaskSpatialObject< TInputImage::ImageDimension > ImageMaskSpatialObjectType;
      typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
      mask->SetImage(this->m_ClippedImageMask);
      mask->ComputeObjectToWorldTransform();
      moments->SetSpatialObjectMask( dynamic_cast< itk::SpatialObject< TInputImage::ImageDimension > * >( mask.
                                                                                                          GetPointer() ) );
      }

    moments->Compute();
    typename TInputImage::PointType::VectorType tempCenterOfMass = moments->GetCenterOfGravity();
    for ( unsigned int q = 0; q < TInputImage::ImageDimension; q++ )
      {
      this->m_CenterOfBrain[q] = tempCenterOfMass[q];
      }
    }

  // #ifdef USE_DEBUGGIN_IMAGES
  if ( this->m_GenerateDebugImages )
    {
    std::cout << "---CenterOfMass:" << this->m_CenterOfBrain << std::endl;
    }
}
}
#endif // itkFindCenterOfBrainFilter_txx
