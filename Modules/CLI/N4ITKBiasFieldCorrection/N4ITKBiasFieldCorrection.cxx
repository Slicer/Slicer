#include "itkConstantPadImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkShrinkImageFilter.h"

#if ITK_VERSION_MAJOR >= 4
// This is  now officially part of ITKv4
#include "itkN4BiasFieldCorrectionImageFilter.h"
#else
// Need private version for ITKv3 that does not conflict with ITKv4 fixes
#include "SlicerITKv3N4MRIBiasFieldCorrectionImageFilter.h"
#endif

#include "N4ITKBiasFieldCorrectionCLP.h"
#include "itkPluginUtilities.h"

namespace
{

typedef float RealType;
const int ImageDimension = 3;
typedef itk::Image<RealType, ImageDimension> ImageType;

template <class TFilter>
class CommandIterationUpdate : public itk::Command
{
public:
  typedef CommandIterationUpdate  Self;
  typedef itk::Command            Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate()
  {
  };
public:

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *) caller, event);
  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    const TFilter * filter =
      dynamic_cast<const TFilter *>( object );

    if( typeid( event ) != typeid( itk::IterationEvent ) )
      {
      return;
      }
    std::cout << "Progress: " << filter->GetProgress() << std::endl;
  }

};

template <class T>
int SaveIt(ImageType::Pointer img, const char* fname, T)
{
  typedef itk::Image<T, 3>                                 OutputImageType;
  typedef itk::CastImageFilter<ImageType, OutputImageType> CastType;

  typename CastType::Pointer caster = CastType::New();
  caster->SetInput(img);

  typedef  itk::ImageFileWriter<OutputImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetInput( caster->GetOutput() );
  writer->SetFileName( fname );
  writer->SetUseCompression(1);
  writer->Update();

  return EXIT_SUCCESS;
}

};

int main(int argc, char* * argv)
{

  PARSE_ARGS;

  ImageType::Pointer inputImage = NULL;

  typedef itk::Image<unsigned char, ImageDimension> MaskImageType;
  MaskImageType::Pointer maskImage = NULL;

#if ITK_VERSION_MAJOR >= 4
  typedef    itk::N4BiasFieldCorrectionImageFilter<ImageType, MaskImageType, ImageType> CorrecterType;
#else
  typedef itk::N4MRIBiasFieldCorrectionImageFilter<ImageType, MaskImageType, ImageType> CorrecterType;
#endif
  CorrecterType::Pointer correcter = CorrecterType::New();

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( inputImageName.c_str() );
  reader->Update();
  inputImage = reader->GetOutput();

  /**
   * handle he mask image
   */

  if( maskImageName != "" )
    {
    typedef itk::ImageFileReader<MaskImageType> ReaderType;
    ReaderType::Pointer maskreader = ReaderType::New();
    maskreader->SetFileName( maskImageName.c_str() );
    maskreader->Update();
    maskImage = maskreader->GetOutput();
    itk::ImageRegionConstIterator<MaskImageType> IM(
      maskImage, maskImage->GetBufferedRegion() );
    MaskImageType::PixelType maskLabel = 0;
    for( IM.GoToBegin(); !IM.IsAtEnd(); ++IM )
      {
      if( IM.Get() )
        {
        maskLabel = IM.Get();
        break;
        }
      }
    if( !maskLabel )
      {
      return EXIT_FAILURE;
      }
    correcter->SetMaskLabel(maskLabel);
    }

  if( !maskImage )
    {
    std::cout << "Mask no read.  Creaing Otsu mask." << std::endl;
    typedef itk::OtsuThresholdImageFilter<ImageType, MaskImageType>
    ThresholderType;
    ThresholderType::Pointer otsu = ThresholderType::New();
    otsu->SetInput( inputImage );
    otsu->SetNumberOfHistogramBins( 200 );
    otsu->SetInsideValue( 0 );
    otsu->SetOutsideValue( 1 );
    otsu->Update();

    maskImage = otsu->GetOutput();
    }

  ImageType::Pointer weightImage = NULL;

  if( weightImageName != "" )
    {
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer weightreader = ReaderType::New();
    weightreader->SetFileName( weightImageName.c_str() );
    weightreader->Update();
    weightImage = weightreader->GetOutput();
    }

  /**
   * convergence opions
   */
  if( numberOfIterations.size() > 1 && numberOfIterations[0] )
    {
    CorrecterType::VariableSizeArrayType
    maximumNumberOfIterations( numberOfIterations.size() );
    for( unsigned d = 0; d < numberOfIterations.size(); d++ )
      {
      maximumNumberOfIterations[d] = numberOfIterations[d];
      }
    correcter->SetMaximumNumberOfIterations( maximumNumberOfIterations );

    CorrecterType::ArrayType numberOfFittingLevels;
    numberOfFittingLevels.Fill( numberOfIterations.size() );
    correcter->SetNumberOfFittingLevels( numberOfFittingLevels );
    }

  if( convergenceThreshold )
    {
    correcter->SetConvergenceThreshold( convergenceThreshold );
    }

  /**
   * B-spline opions -- we place his here o ake care of he case where
   * he user wans o specify hings in erms of he spline disance.
   */

  bool                 useSplineDistance = false;
  ImageType::IndexType inputImageIndex =
    inputImage->GetLargestPossibleRegion().GetIndex();
  ImageType::SizeType inputImageSize =
    inputImage->GetLargestPossibleRegion().GetSize();
  ImageType::IndexType maskImageIndex =
    maskImage->GetLargestPossibleRegion().GetIndex();
  ImageType::SizeType maskImageSize =
    maskImage->GetLargestPossibleRegion().GetSize();

  ImageType::PointType newOrigin = inputImage->GetOrigin();

  if( bsplineOrder )
    {
    correcter->SetSplineOrder(bsplineOrder);
    }

  CorrecterType::ArrayType numberOfControlPoints;
  if( splineDistance )
    {
    useSplineDistance = true;

    unsigned long lowerBound[ImageDimension];
    unsigned long upperBound[ImageDimension];
    for( unsigned  d = 0; d < 3; d++ )
      {
      float domain = static_cast<RealType>( inputImage->
                                            GetLargestPossibleRegion().GetSize()[d] - 1 ) * inputImage->GetSpacing()[d];
      unsigned int  numberOfSpans = static_cast<unsigned int>( vcl_ceil( domain / splineDistance ) );
      unsigned long extraPadding = static_cast<unsigned long>( ( numberOfSpans
                                                                 * splineDistance
                                                                 - domain ) / inputImage->GetSpacing()[d] + 0.5 );
      lowerBound[d] = static_cast<unsigned long>( 0.5 * extraPadding );
      upperBound[d] = extraPadding - lowerBound[d];
      newOrigin[d] -= ( static_cast<RealType>( lowerBound[d] )
                        * inputImage->GetSpacing()[d] );
      numberOfControlPoints[d] = numberOfSpans + correcter->GetSplineOrder();
      }

    typedef itk::ConstantPadImageFilter<ImageType, ImageType> PadderType;
    PadderType::Pointer padder = PadderType::New();
    padder->SetInput( inputImage );
    padder->SetPadLowerBound( lowerBound );
    padder->SetPadUpperBound( upperBound );
    padder->SetConstant( 0 );
    padder->Update();
    inputImage = padder->GetOutput();

    typedef itk::ConstantPadImageFilter<MaskImageType, MaskImageType> MaskPadderType;
    MaskPadderType::Pointer maskPadder = MaskPadderType::New();
    maskPadder->SetInput( maskImage );
    maskPadder->SetPadLowerBound( lowerBound );
    maskPadder->SetPadUpperBound( upperBound );
    maskPadder->SetConstant( 0 );
    maskPadder->Update();
    maskImage = maskPadder->GetOutput();

    if( weightImage )
      {
      PadderType::Pointer weightPadder = PadderType::New();
      weightPadder->SetInput( weightImage );
      weightPadder->SetPadLowerBound( lowerBound );
      weightPadder->SetPadUpperBound( upperBound );
      weightPadder->SetConstant( 0 );
      weightPadder->Update();
      weightImage = weightPadder->GetOutput();
      }
    correcter->SetNumberOfControlPoints( numberOfControlPoints );
    }
  else if( initialMeshResolution.size() == 3 )
    {
    for( unsigned d = 0; d < 3; d++ )
      {
      numberOfControlPoints[d] = static_cast<unsigned int>( initialMeshResolution[d] )
        + correcter->GetSplineOrder();
      }
    correcter->SetNumberOfControlPoints( numberOfControlPoints );
    }

  typedef itk::ShrinkImageFilter<ImageType, ImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( inputImage );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors( shrinkFactor );
  maskshrinker->SetShrinkFactors( shrinkFactor );
  shrinker->Update();
  maskshrinker->Update();

  itk::TimeProbe timer;
  timer.Start();

  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );
  if( weightImage )
    {
    typedef itk::ShrinkImageFilter<ImageType, ImageType> WeightShrinkerType;
    WeightShrinkerType::Pointer weightshrinker = WeightShrinkerType::New();
    weightshrinker->SetInput( weightImage );
    weightshrinker->SetShrinkFactors( 1 );
    weightshrinker->SetShrinkFactors( shrinkFactor );
    weightshrinker->Update();
    correcter->SetConfidenceImage( weightshrinker->GetOutput() );
    }

  typedef CommandIterationUpdate<CorrecterType> CommandType;
  CommandType::Pointer observer = CommandType::New();
  correcter->AddObserver( itk::IterationEvent(), observer );

  /**
   * histogram sharpening options
   */
  if( bfFWHM )
    {
    correcter->SetBiasFieldFullWidthAtHalfMaximum( bfFWHM );
    }
  if( wienerFilterNoise )
    {
    correcter->SetWienerFilterNoise( wienerFilterNoise );
    }
  if( nHistogramBins )
    {
    correcter->SetNumberOfHistogramBins( nHistogramBins );
    }

  try
    {
    itk::PluginFilterWatcher watchN4(correcter, "N4 Bias field correction", CLPProcessInformation, 1.0 / 1.0, 0.0);
    correcter->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    std::cerr << "Unknown Exception caught." << std::endl;
    return EXIT_FAILURE;
    }

  correcter->Print( std::cout, 3 );

  timer.Stop();
  std::cout << "Elapsed ime: " << timer.GetMean() << std::endl;

  /**
   * ouput
   */
  if( outputImageName != "" )
    {
    /**
     * Reconsruct the bias field at full image resoluion.  Divide
     * the original input image by the bias field to get the final
     * corrected image.
     */
    typedef itk::BSplineControlPointImageFilter<
      CorrecterType::BiasFieldControlPointLatticeType,
      CorrecterType::ScalarImageType> BSplinerType;
    BSplinerType::Pointer bspliner = BSplinerType::New();
    bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
    bspliner->SetSplineOrder( correcter->GetSplineOrder() );
    bspliner->SetSize( inputImage->GetLargestPossibleRegion().GetSize() );
    bspliner->SetOrigin( newOrigin );
    bspliner->SetDirection( inputImage->GetDirection() );
    bspliner->SetSpacing( inputImage->GetSpacing() );
    bspliner->Update();

    ImageType::Pointer logField = ImageType::New();
    logField->SetOrigin( inputImage->GetOrigin() );
    logField->SetSpacing( inputImage->GetSpacing() );
    logField->SetRegions( inputImage->GetLargestPossibleRegion() );
    logField->SetDirection( inputImage->GetDirection() );
    logField->Allocate();

    itk::ImageRegionIterator<CorrecterType::ScalarImageType> IB(
      bspliner->GetOutput(),
      bspliner->GetOutput()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<ImageType> IF( logField,
                                            logField->GetLargestPossibleRegion() );
    for( IB.GoToBegin(), IF.GoToBegin(); !IB.IsAtEnd(); ++IB, ++IF )
      {
      IF.Set( IB.Get()[0] );
      }

    typedef itk::ExpImageFilter<ImageType, ImageType> ExpFilterType;
    ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput( logField );
    expFilter->Update();

    typedef itk::DivideImageFilter<ImageType, ImageType, ImageType> DividerType;
    DividerType::Pointer divider = DividerType::New();
    divider->SetInput1( inputImage );
    divider->SetInput2( expFilter->GetOutput() );
    divider->Update();

    ImageType::RegionType inputRegion;
    inputRegion.SetIndex( inputImageIndex );
    inputRegion.SetSize( inputImageSize );

    typedef itk::ExtractImageFilter<ImageType, ImageType> CropperType;
    CropperType::Pointer cropper = CropperType::New();
    cropper->SetInput( divider->GetOutput() );
    cropper->SetExtractionRegion( inputRegion );
#if ITK_VERSION_MAJOR >= 4
    cropper->SetDirectionCollapseToSubmatrix();
#endif
    cropper->Update();

    CropperType::Pointer biasFieldCropper = CropperType::New();
    biasFieldCropper->SetInput( expFilter->GetOutput() );
    biasFieldCropper->SetExtractionRegion( inputRegion );
#if ITK_VERSION_MAJOR >= 4
    biasFieldCropper->SetDirectionCollapseToSubmatrix();
#endif

    biasFieldCropper->Update();

    if( outputBiasFieldName != "" )
      {
      typedef itk::ImageFileWriter<ImageType> WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( outputBiasFieldName.c_str() );
      writer->SetInput( biasFieldCropper->GetOutput() );
      writer->SetUseCompression(1);
      writer->Update();
      }

    try
      {

      itk::ImageIOBase::IOPixelType     pixelType;
      itk::ImageIOBase::IOComponentType componentType;

      itk::GetImageType(inputImageName, pixelType, componentType);

      // This filter handles all types on input, but only produces
      // signed types
      const char *fname = outputImageName.c_str();

      switch( componentType )
        {
        case itk::ImageIOBase::UCHAR:
          return SaveIt( cropper->GetOutput(), fname, static_cast<unsigned char>(0) );
          break;
        case itk::ImageIOBase::CHAR:
          return SaveIt( cropper->GetOutput(), fname, static_cast<char>(0) );
          break;
        case itk::ImageIOBase::USHORT:
          return SaveIt( cropper->GetOutput(), fname, static_cast<unsigned short>(0) );
          break;
        case itk::ImageIOBase::SHORT:
          return SaveIt( cropper->GetOutput(), fname, static_cast<short>(0) );
          break;
        case itk::ImageIOBase::UINT:
          return SaveIt( cropper->GetOutput(), fname, static_cast<unsigned int>(0) );
          break;
        case itk::ImageIOBase::INT:
          return SaveIt( cropper->GetOutput(), fname, static_cast<int>(0) );
          break;
        case itk::ImageIOBase::ULONG:
          return SaveIt( cropper->GetOutput(), fname, static_cast<unsigned long>(0) );
          break;
        case itk::ImageIOBase::LONG:
          return SaveIt( cropper->GetOutput(), fname, static_cast<long>(0) );
          break;
        case itk::ImageIOBase::FLOAT:
          return SaveIt( cropper->GetOutput(), fname, static_cast<float>(0) );
          break;
        case itk::ImageIOBase::DOUBLE:
          return SaveIt( cropper->GetOutput(), fname, static_cast<double>(0) );
          break;
        case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
          std::cerr << "Cannot saved the result using the requested pixel type" << std::endl;
          return EXIT_FAILURE;
        default:
          std::cout << "unknown component type" << std::endl;
          break;
        }
      }
    catch( itk::ExceptionObject & e )
      {
      std::cerr << "Failed to save the data: " << e << std::endl;
      return EXIT_FAILURE;
      }

    }

  return EXIT_SUCCESS;
}
