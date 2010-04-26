#include "itkBSplineControlPointImageFilter.h"
#include "itkCommandLineOption.h"
#include "itkCommandLineParser.h"
#include "itkConstantPadImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkN4MRIBiasFieldCorrectionImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkShrinkImageFilter.h"

#include "itkTimeProbe.h"

#include <string>
#include <algorithm>
#include <vector>

template<class TFilter>
class CommandIterationUpdate : public itk::Command
{
public:
  typedef CommandIterationUpdate   Self;
  typedef itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
    Execute( (const itk::Object *) caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
    const TFilter * filter =
      dynamic_cast< const TFilter * >( object );
    if( typeid( event ) != typeid( itk::IterationEvent ) )
      { return; }
    if( filter->GetElapsedIterations() == 1 )
      {
      std::cout << "Current level = " << filter->GetCurrentLevel() + 1
        << std::endl;
      }
    std::cout << "  Iteration " << filter->GetElapsedIterations()
      << " (of "
      << filter->GetMaximumNumberOfIterations()[filter->GetCurrentLevel()]
      << ").  ";
    std::cout << " Current convergence value = "
      << filter->GetCurrentConvergenceMeasurement()
      << " (threshold = " << filter->GetConvergenceThreshold()
      << ")" << std::endl;
    }

};

template <unsigned int ImageDimension>
int InhomogeneityCorrectImage( itk::CommandLineParser *parser )
{
  typedef float RealType;

  typedef itk::Image<RealType, ImageDimension> ImageType;
  typename ImageType::Pointer inputImage = NULL;

  typedef itk::Image<unsigned char, ImageDimension> MaskImageType;
  typename MaskImageType::Pointer maskImage = NULL;

  typedef itk::N4MRIBiasFieldCorrectionImageFilter<ImageType, MaskImageType,
    ImageType> CorrecterType;
  typename CorrecterType::Pointer correcter = CorrecterType::New();

  typedef itk::ImageFileReader<ImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();

typename itk::CommandLineParser::OptionType::Pointer inputImageOption =
parser->GetOption( "input-image" );
if( inputImageOption )
{
    std::string inputFile = inputImageOption->GetValue();
reader->SetFileName( inputFile.c_str() );
reader->Update();

    inputImage = reader->GetOutput();
    }
  else
    {
    std::cerr << "Input image not specified." << std::endl;
    return EXIT_FAILURE;
    }

  /**
   * handle the mask image
   */

typename itk::CommandLineParser::OptionType::Pointer maskImageOption =
parser->GetOption( "mask-image" );
if( maskImageOption && maskImageOption->GetNumberOfValues() )
{
    std::string inputFile = maskImageOption->GetValue();
typedef itk::ImageFileReader<MaskImageType> ReaderType;
typename ReaderType::Pointer maskreader = ReaderType::New();
maskreader->SetFileName( inputFile.c_str() );
    try
      {
  maskreader->Update();
  maskImage = maskreader->GetOutput();
      }
    catch(...) {}
    }
  if( !maskImage )
    {
    std::cout << "Mask not read.  Creating Otsu mask." << std::endl;
    typedef itk::OtsuThresholdImageFilter<ImageType, MaskImageType>
      ThresholderType;
    typename ThresholderType::Pointer otsu = ThresholderType::New();
    otsu->SetInput( inputImage );
    otsu->SetNumberOfHistogramBins( 200 );
    otsu->SetInsideValue( 0 );
    otsu->SetOutsideValue( 1 );
    otsu->Update();

    maskImage = otsu->GetOutput();
    }

  typename ImageType::Pointer weightImage = NULL;

typename itk::CommandLineParser::OptionType::Pointer weightImageOption =
parser->GetOption( "weight-image" );
if( weightImageOption  && weightImageOption->GetNumberOfValues() )
{
    std::string inputFile = weightImageOption->GetValue();
typedef itk::ImageFileReader<ImageType> ReaderType;
typename ReaderType::Pointer weightreader = ReaderType::New();
weightreader->SetFileName( inputFile.c_str() );
weightreader->Update();
weightImage = weightreader->GetOutput();
    }

  /**
   * convergence options
   */
  typename itk::CommandLineParser::OptionType::Pointer convergenceOption =
    parser->GetOption( "convergence" );
  if( convergenceOption )
    {
    if( convergenceOption->GetNumberOfParameters() > 0 )
      {
      std::vector<unsigned int> numIters = parser->ConvertVector<unsigned int>(
        convergenceOption->GetParameter( 0 ) );
      typename CorrecterType::VariableSizeArrayType
        maximumNumberOfIterations( numIters.size() );
      for( unsigned int d = 0; d < numIters.size(); d++ )
        {
        maximumNumberOfIterations[d] = numIters[d];
        }
      correcter->SetMaximumNumberOfIterations( maximumNumberOfIterations );

typename CorrecterType::ArrayType numberOfFittingLevels;
numberOfFittingLevels.Fill( numIters.size() );
correcter->SetNumberOfFittingLevels( numberOfFittingLevels );
      }
    if( convergenceOption->GetNumberOfParameters() > 1 )
      {
      correcter->SetConvergenceThreshold( parser->Convert<float>(
        convergenceOption->GetParameter( 1 ) ) );
      }
    }

  /**
   * B-spline options -- we place this here to take care of the case where
   * the user wants to specify things in terms of the spline distance.
   */

  bool useSplineDistance = false;
  typename ImageType::IndexType inputImageIndex =
    inputImage->GetLargestPossibleRegion().GetIndex();
  typename ImageType::SizeType inputImageSize =
    inputImage->GetLargestPossibleRegion().GetSize();
  typename ImageType::IndexType maskImageIndex =
    maskImage->GetLargestPossibleRegion().GetIndex();
  typename ImageType::SizeType maskImageSize =
    maskImage->GetLargestPossibleRegion().GetSize();

  typename ImageType::PointType newOrigin = inputImage->GetOrigin();

  typename itk::CommandLineParser::OptionType::Pointer bsplineOption =
    parser->GetOption( "bspline-fitting" );
  if( bsplineOption )
    {
    if( bsplineOption->GetNumberOfParameters() > 1 )
      {
      correcter->SetSplineOrder( parser->Convert<unsigned int>(
        bsplineOption->GetParameter( 1 ) ) );
      }
    if( bsplineOption->GetNumberOfParameters() > 2 )
      {
      correcter->SetSigmoidNormalizedAlpha( parser->Convert<float>(
        bsplineOption->GetParameter( 2 ) ) );
      }
    if( bsplineOption->GetNumberOfParameters() > 3 )
      {
      correcter->SetSigmoidNormalizedBeta( parser->Convert<float>(
        bsplineOption->GetParameter( 3 ) ) );
      }
    if( bsplineOption->GetNumberOfParameters() > 0 )
      {
      std::vector<float> array = parser->ConvertVector<float>(
        bsplineOption->GetParameter( 0 ) );
      typename CorrecterType::ArrayType numberOfControlPoints;
      if( array.size() == 1 )
        {
        // the user wants to specify things in terms of spline distance.
        //  1. need to pad the images to get as close to possible to the
        //     requested domain size.
        useSplineDistance = true;
        float splineDistance = array[0];

unsigned long lowerBound[ImageDimension];
unsigned long upperBound[ImageDimension];

for( unsigned int d = 0; d < ImageDimension; d++ )
{
float domain = static_cast<RealType>( inputImage->
GetLargestPossibleRegion().GetSize()[d] - 1 ) * inputImage->GetSpacing()[d];
unsigned int numberOfSpans = static_cast<unsigned int>(
            vcl_ceil( domain / splineDistance ) );
unsigned long extraPadding = static_cast<unsigned long>( ( numberOfSpans *
splineDistance - domain ) / inputImage->GetSpacing()[d] + 0.5 );
lowerBound[d] = static_cast<unsigned long>( 0.5 * extraPadding );
upperBound[d] = extraPadding - lowerBound[d];
          newOrigin[d] -= ( static_cast<RealType>( lowerBound[d] ) *
            inputImage->GetSpacing()[d] );

          numberOfControlPoints[d] = numberOfSpans + correcter->GetSplineOrder();
}

typedef itk::ConstantPadImageFilter<ImageType, ImageType> PadderType;
typename PadderType::Pointer padder = PadderType::New();
padder->SetInput( inputImage );
padder->SetPadLowerBound( lowerBound );
padder->SetPadUpperBound( upperBound );
padder->SetConstant( 0 );
padder->Update();
        inputImage = padder->GetOutput();

typedef itk::ConstantPadImageFilter<MaskImageType, MaskImageType> MaskPadderType;
typename MaskPadderType::Pointer maskPadder = MaskPadderType::New();
maskPadder->SetInput( maskImage );
maskPadder->SetPadLowerBound( lowerBound );
maskPadder->SetPadUpperBound( upperBound );
maskPadder->SetConstant( 0 );
maskPadder->Update();
        maskImage = maskPadder->GetOutput();

        if( weightImage )
          {
typename PadderType::Pointer weightPadder = PadderType::New();
weightPadder->SetInput( weightImage );
weightPadder->SetPadLowerBound( lowerBound );
weightPadder->SetPadUpperBound( upperBound );
weightPadder->SetConstant( 0 );
weightPadder->Update();
weightImage = weightPadder->GetOutput();
          }
        }
      else if( array.size() == ImageDimension )
        {
        for( unsigned int d = 0; d < ImageDimension; d++ )
          {
          numberOfControlPoints[d] = static_cast<unsigned int>( array[d] ) +
            correcter->GetSplineOrder();
          }
        }
      else
        {
        std::cerr << "Incorrect mesh resolution" << std::endl;
        return EXIT_FAILURE;
        }
      correcter->SetNumberOfControlPoints( numberOfControlPoints );
      }
    }

  typedef itk::ShrinkImageFilter<ImageType, ImageType> ShrinkerType;
  typename ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( inputImage );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  typename MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

typename itk::CommandLineParser::OptionType::Pointer shrinkFactorOption =
parser->GetOption( "shrink-factor" );
  int shrinkFactor = 4;
if( shrinkFactorOption )
{
    shrinkFactor = parser->Convert<int>( shrinkFactorOption->GetValue() );
    }
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
typename WeightShrinkerType::Pointer weightshrinker = WeightShrinkerType::New();
weightshrinker->SetInput( weightImage );
weightshrinker->SetShrinkFactors( 1 );
if( shrinkFactorOption )
{
int shrinkFactor = parser->Convert<int>( shrinkFactorOption->GetValue() );
weightshrinker->SetShrinkFactors( shrinkFactor );
}
    weightshrinker->Update();
    correcter->SetConfidenceImage( weightshrinker->GetOutput() );
    }

  typedef CommandIterationUpdate<CorrecterType> CommandType;
  typename CommandType::Pointer observer = CommandType::New();
  correcter->AddObserver( itk::IterationEvent(), observer );

  /**
   * histogram sharpening options
   */
  typename itk::CommandLineParser::OptionType::Pointer histOption =
    parser->GetOption( "histogram-sharpening" );
  if( histOption )
    {
    if( histOption->GetNumberOfParameters() > 0 )
      {
      correcter->SetBiasFieldFullWidthAtHalfMaximum( parser->Convert<float>(
        histOption->GetParameter( 0 ) ) );
      }
    if( histOption->GetNumberOfParameters() > 1 )
      {
      correcter->SetWeinerFilterNoise( parser->Convert<float>(
        histOption->GetParameter( 1 ) ) );
      }
    if( histOption->GetNumberOfParameters() > 2 )
      {
      correcter->SetNumberOfHistogramBins( parser->Convert<unsigned int>(
        histOption->GetParameter( 2 ) ) );
      }
    }


  try
    {
    correcter->Update();
    }
  catch(...)
    {
    std::cerr << "Exception caught." << std::endl;
    return EXIT_FAILURE;
    }

  correcter->Print( std::cout, 3 );

  timer.Stop();
  std::cout << "Elapsed time: " << timer.GetMeanTime() << std::endl;

  /**
   * output
   */
  typename itk::CommandLineParser::OptionType::Pointer outputOption =
    parser->GetOption( "output" );
  if( outputOption )
    {
/**
* Reconstruct the bias field at full image resolution.  Divide
* the original input image by the bias field to get the final
* corrected image.
*/
typedef itk::BSplineControlPointImageFilter<typename
CorrecterType::BiasFieldControlPointLatticeType, typename
CorrecterType::ScalarImageType> BSplinerType;
typename BSplinerType::Pointer bspliner = BSplinerType::New();
bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
bspliner->SetSplineOrder( correcter->GetSplineOrder() );
bspliner->SetSize( inputImage->GetLargestPossibleRegion().GetSize() );
bspliner->SetOrigin( newOrigin );
bspliner->SetDirection( inputImage->GetDirection() );
bspliner->SetSpacing( inputImage->GetSpacing() );
bspliner->Update();

typename ImageType::Pointer logField = ImageType::New();
logField->SetOrigin( inputImage->GetOrigin() );
logField->SetSpacing( inputImage->GetSpacing() );
logField->SetRegions( inputImage->GetLargestPossibleRegion() );
logField->SetDirection( inputImage->GetDirection() );
logField->Allocate();

itk::ImageRegionIterator<typename CorrecterType::ScalarImageType> ItB(
bspliner->GetOutput(),
bspliner->GetOutput()->GetLargestPossibleRegion() );
itk::ImageRegionIterator<ImageType> ItF( logField,
logField->GetLargestPossibleRegion() );
for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
{
ItF.Set( ItB.Get()[0] );
}

typedef itk::ExpImageFilter<ImageType, ImageType> ExpFilterType;
typename ExpFilterType::Pointer expFilter = ExpFilterType::New();
expFilter->SetInput( logField );
expFilter->Update();

typedef itk::DivideImageFilter<ImageType, ImageType, ImageType> DividerType;
typename DividerType::Pointer divider = DividerType::New();
divider->SetInput1( inputImage );
divider->SetInput2( expFilter->GetOutput() );
divider->Update();

    typename ImageType::RegionType inputRegion;
    inputRegion.SetIndex( inputImageIndex );
    inputRegion.SetSize( inputImageSize );

typedef itk::ExtractImageFilter<ImageType, ImageType> CropperType;
typename CropperType::Pointer cropper = CropperType::New();
cropper->SetInput( divider->GetOutput() );
cropper->SetExtractionRegion( inputRegion );
cropper->Update();

typename CropperType::Pointer biasFieldCropper = CropperType::New();
biasFieldCropper->SetInput( expFilter->GetOutput() );
biasFieldCropper->SetExtractionRegion( inputRegion );
biasFieldCropper->Update();

    if( outputOption->GetNumberOfParameters() == 0 )
      {
      typedef  itk::ImageFileWriter<ImageType> WriterType;
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetInput( cropper->GetOutput() );
      writer->SetFileName( ( outputOption->GetValue() ).c_str() );
      writer->Update();
      }
    if( outputOption->GetNumberOfParameters() > 0 )
      {
      typedef  itk::ImageFileWriter<ImageType> WriterType;
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetInput( cropper->GetOutput() );
      writer->SetFileName( ( outputOption->GetParameter( 0 ) ).c_str() );
      writer->Update();
      }

    outputOption = parser->GetOption( "output-bias-field" );
    if(outputOption){
      std::cout << "Option is set" << std::endl;
      typedef itk::ImageFileWriter<ImageType> WriterType;
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( ( outputOption->GetValue() ).c_str() );
      writer->SetInput( biasFieldCropper->GetOutput() );
      writer->Update();
    } else {
      std::cout << "Option f not set" << std::endl;
    }
  }
  

  return EXIT_SUCCESS;
}

void InitializeCommandLineOptions( itk::CommandLineParser *parser )
{
  typedef itk::CommandLineParser::OptionType OptionType;

  {
  std::string description = std::string( "inputImage" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "input-image" );
  option->SetShortName( 'i' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "maskImage" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "mask-image" );
  option->SetShortName( 'x' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "weightImage" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "weight-image" );
  option->SetShortName( 'w' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "shrinkFactor" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "shrink-factor" );
  option->SetShortName( 's' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description =
    std::string( "[<numberOfIterations>,<convergenceThreshold>]" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "convergence" );
  option->SetShortName( 'c' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description =
    std::string( "[<initialMeshResolution> or " ) +
    std::string( "<splineDistance>,<splineOrder>," ) +
    std::string( "<sigmoidAlpha>,<sigmoidBeta>]" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "bspline-fitting" );
  option->SetShortName( 'b' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description =
    std::string( "[<FWHM>,<wienerNoise>,<numberOfHistogramBins>]" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "histogram-sharpening" );
  option->SetShortName( 't' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "correctedImage" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "output" );
  option->SetShortName( 'o' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "biasField" );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "output-bias-field" );
  option->SetShortName( 'f' );
  option->SetDescription( description );
  parser->AddOption( option );
  }

  {
  std::string description = std::string( "Print menu." );

  OptionType::Pointer option = OptionType::New();
  option->SetLongName( "help" );
  option->SetShortName( 'h' );
  option->SetDescription( description );
  option->AddValue( std::string( "0" ) );
  parser->AddOption( option );
  }
}

int main( int argc, char *argv[] )
{
  if ( argc < 2 )
    {
    std::cout << "Usage: " << argv[0]
      << " imageDimension args" << std::endl;
    exit( 1 );
    }

  itk::CommandLineParser::Pointer parser = itk::CommandLineParser::New();
  parser->SetCommand( argv[0] );

  parser->SetCommandDescription( "N4 bias correction." );
  InitializeCommandLineOptions( parser );

  parser->Parse( argc, argv );

  if( argc < 3 || parser->Convert<bool>(
    parser->GetOption( "help" )->GetValue() ) )
    {
    parser->PrintMenu( std::cout, 5 );
    exit( EXIT_FAILURE );
    }

  switch( atoi( argv[1] ) )
   {
   case 2:
     InhomogeneityCorrectImage<2>( parser );
     break;
   case 3:
     InhomogeneityCorrectImage<3>( parser );
     break;
   default:
      std::cerr << "Unsupported dimension" << std::endl;
      exit( EXIT_FAILURE );
   }
}

